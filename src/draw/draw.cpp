#include <iostream>
#include <cstdlib>
#include <ctime>
#include <functional>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "draw.hpp"
#include "viridis.hpp"
#include "smoothcoolwarm.hpp"
#include "shader.hpp"

#include "util/util.hpp"

#define REQUIRED_OGL_VERSION_MAJOR 3
#define REQUIRED_OGL_VERSION_MINOR 3

//-----------------------------------------------------------------------------
// draw class construction and destruction
//-----------------------------------------------------------------------------
const glm::vec3 draw::Renderer::DEFAULT_CAMERA_POSITION(1.5f, 1.25f, 1.5f);
const glm::vec3 draw::Renderer::DEFAULT_CAMERA_LOOKAT(0.5f);
const std::array<size_t, 2> draw::Renderer::FRACTURE_TEXTURE_RESOLUTION =
    { 720, 720 };

//-----------------------------------------------------------------------------
// draw class construction and destruction
//-----------------------------------------------------------------------------
draw::Renderer::Renderer() :
    // window state
    m_window(nullptr),
    m_windowDimensions{ {1280, 720} },
    m_isInitialized(false),
    // common visualization parameters
    m_cmClipMin(0.f),
    m_cmClipMax(1.f),
    m_cmSelect(0),
    m_isovalueInterval(0.1f),
    m_isolineColor({0.f, 0.f, 0.f}),
    // fracture network geometry
    m_fractureNetwork{
        false, false, false, false, false, false, false, false, false},
    m_fractureModelMxs{ glm::mat4(1.f), glm::mat4(1.f), glm::mat4(1.f),
        glm::mat4(1.f), glm::mat4(1.f), glm::mat4(1.f),
        glm::mat4(1.f), glm::mat4(1.f), glm::mat4(1.f) },
    // 3D visualization objects and parameters
    m_fovY(45.f),
    m_zNear(0.05f),
    m_zFar(5.f),
    m_cameraPosition(DEFAULT_CAMERA_POSITION),
    m_cameraLookAt(DEFAULT_CAMERA_LOOKAT),
    m_cameraZoomSpeed(0.1f),
    m_cameraRotationSpeed(0.2f),
    m_cameraTranslationSpeed(0.002f),
    m_3dViewMx(1.f),
    m_3dProjMx(1.f),
    m_fractureFbo(),
    m_planeShader(),
    m_frameShader(),
    m_volumeFrame(false),
    // common rendering objects
    m_framebuffer(),
    m_viridisMap(),
    m_smoothcoolwarmMap(),
    m_windowShader(),
    m_fractureShader(),
    m_isolineShader(),
    m_windowQuad(false),
    m_quadProjMx(glm::ortho(-0.5f, 0.5f, -0.5f, 0.5f)),
    m_showDemoWindow(false)
{
    // nothing to see here
}

draw::Renderer::Renderer(unsigned int winWidth, unsigned int winHeight) :
    draw::Renderer::Renderer()
{
    m_windowDimensions[0] = winWidth;
    m_windowDimensions[1] = winHeight;
}

draw::Renderer::~Renderer()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (nullptr != m_window)
        glfwDestroyWindow(m_window);

    glfwTerminate();
}

//-----------------------------------------------------------------------------
// provided functions
//-----------------------------------------------------------------------------
int draw::Renderer::initialize()
{
    int ret = EXIT_SUCCESS;

    //-------------------------------------------------------------------------
    // window and context creation
    //-------------------------------------------------------------------------
    m_window = createWindow(
        m_windowDimensions[0], m_windowDimensions[1], "inpreCICE");

    ret = initializeGl3w();
    if (EXIT_SUCCESS != ret) return ret;

    ret = initializeImGui();
    if (EXIT_SUCCESS != ret) return ret;

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClearDepth(1.f);
    glEnable(GL_DEPTH_TEST);


    //-------------------------------------------------------------------------
    // shader setup
    //-------------------------------------------------------------------------
    m_windowShader = Shader(
            "src/draw/shader/windowQuad.vert",
            "src/draw/shader/windowQuad.frag");
    m_fractureShader = Shader(
            "src/draw/shader/fracture.vert", "src/draw/shader/fracture.frag");
    m_isolineShader = Shader(
            "src/draw/shader/isolines.vert", "src/draw/shader/isolines.frag");
    m_planeShader = Shader(
            "src/draw/shader/plane.vert", "src/draw/shader/plane.frag");
    m_frameShader = Shader(
            "src/draw/shader/frame.vert", "src/draw/shader/frame.frag");

    // ------------------------------------------------------------------------
    // geometry
    // ------------------------------------------------------------------------
    m_windowQuad = util::geometry::Quad(true);
    for (size_t i = 0; i < 9; ++i)
        m_fractureNetwork[i] = util::geometry::Quad(true);

    m_volumeFrame = util::geometry::CubeFrame(true);

    //-------------------------------------------------------------------------
    // transformation matrices
    //-------------------------------------------------------------------------
    // initialize model matrices for fracture network case
    //
    // see appendix 6.1 in https://arxiv.org/pdf/1809.06926.pdf for
    // fracture coordinates
    //
    const glm::mat4 s1 = glm::scale(glm::mat4(1.f), glm::vec3(0.5f));
    const glm::mat4 s2 = glm::scale(glm::mat4(1.f), glm::vec3(0.25f));

    const float pi = 3.14159f;
    const glm::mat4 r1 = glm::rotate(
            glm::mat4(1.f), -0.5f * pi, glm::vec3(0.f, 1.f, 0.f));
    const glm::mat4 r2 = glm::rotate(
            glm::mat4(1.f), 0.5f * pi, glm::vec3(1.f, 0.f, 0.f));

    const glm::mat4 t1 = glm::translate(glm::mat4(1.f), glm::vec3(0.5f));
    const glm::mat4 t2 = glm::translate(glm::mat4(1.f), glm::vec3(0.75f));
    const glm::mat4 t3 = glm::translate(glm::mat4(1.f), glm::vec3(0.625f));

    m_fractureModelMxs[0] = t1 * r1;
    m_fractureModelMxs[1] = t1 * r2;
    m_fractureModelMxs[2] = t1;
    m_fractureModelMxs[3] = t2 * s1 * r1;
    m_fractureModelMxs[4] = t2 * s1;
    m_fractureModelMxs[5] = t2 * s1 * r2;
    m_fractureModelMxs[6] = t3 * s2 * r2;
    m_fractureModelMxs[7] = t3 * s2 * r1;
    m_fractureModelMxs[8] = t3 * s2;

    // initialize view and projections matrices for 3D visualization
    glm::vec3 right = glm::normalize(
        glm::cross(-m_cameraPosition, glm::vec3(0.f, 1.f, 0.f)));
    glm::vec3 up = glm::normalize(glm::cross(right, -m_cameraPosition));
    m_3dViewMx = glm::lookAt(m_cameraPosition, m_cameraLookAt, up);

    m_3dProjMx = glm::perspective(
        glm::radians(m_fovY),
        static_cast<float>(m_windowDimensions[0]) /
            static_cast<float>(m_windowDimensions[1]),
        m_zNear,
        m_zFar);

    //-------------------------------------------------------------------------
    // utility textures
    //-------------------------------------------------------------------------
    m_viridisMap = util::texture::Texture2D(
            GL_RGB,
            GL_RGB,
            0,
            GL_FLOAT,
            GL_LINEAR,
            GL_CLAMP_TO_EDGE,
            128,
            1,
            static_cast<void const *>(&VIRIDIS_FLOAT_RGB_128[0]));
    m_smoothcoolwarmMap = util::texture::Texture2D(
            GL_RGB,
            GL_RGB,
            0,
            GL_FLOAT,
            GL_LINEAR,
            GL_CLAMP_TO_EDGE,
            128,
            1,
            static_cast<void const *>(&SMOOTHCOOLWARM_FLOAT_RGB_128[0]));

    //-------------------------------------------------------------------------
    // framebuffer objects for deferred shading
    //-------------------------------------------------------------------------
    updateFramebufferObjects();

    if (EXIT_SUCCESS == ret)
        m_isInitialized = true;

    return ret;
}

//-----------------------------------------------------------------------------
bool draw::Renderer::processEvents()
{
    if (false == m_isInitialized)
    {
        std::cerr << "Error: Renderer::initialize() must be called "
            "successfully before Renderer::processEvents(...) can be used!" <<
            std::endl;
        return false;
    }

    glfwPollEvents();
    processInput();

    if (!glfwWindowShouldClose(m_window))
    {
        return true;
    }
    else
        return false;
}

//-----------------------------------------------------------------------------
int draw::Renderer::drawSingleFracture(
        const boost::multi_array<double, 2> &data)
{
    if (false == m_isInitialized)
    {
        std::cerr << "Error: Renderer::initialize() must be called "
            "successfully before Renderer::draw can be used!" <<
            std::endl;
        return EXIT_FAILURE;
    }

    // create texture from sample data
    boost::multi_array<float, 2> dataTexture(
            boost::extents[data.shape()[0]][data.shape()[1]]);
    for (size_t y = 0; y < data.shape()[0]; ++y)
    for (size_t x = 0; x < data.shape()[1]; ++x)
        dataTexture[y][x] = static_cast<float>(data[y][x]);

    util::texture::Texture2D fractureTex(
            GL_R32F,
            GL_RED,
            0,
            GL_FLOAT,
            GL_LINEAR,
            GL_CLAMP_TO_EDGE,
            dataTexture.shape()[1],
            dataTexture.shape()[0],
            static_cast<void const*>(dataTexture.data()));

    glViewport(0, 0, m_windowDimensions[0], m_windowDimensions[1]);
    m_framebuffer.bind();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // draw the data into the framebuffer object
    m_fractureShader.use();
    m_fractureShader.setMat4("projMX", m_quadProjMx);
    m_fractureShader.setFloat("tfMin", m_cmClipMin);
    m_fractureShader.setFloat("tfMax", m_cmClipMax);

    glActiveTexture(GL_TEXTURE0);
    fractureTex.bind();
    m_fractureShader.setInt("sampleTex", 0);

    glActiveTexture(GL_TEXTURE1);
    if (m_cmSelect == 0)
        m_smoothcoolwarmMap.bind();
    else
        m_viridisMap.bind();
    m_fractureShader.setInt("tfTex", 1);

    m_windowQuad.draw();

    glm::mat3 pvmMx = glm::transpose(glm::mat3(
            2.f / (dataTexture.shape()[0] - 1.f), 0.f, -1.0f,
            0.f, 2.f / (dataTexture.shape()[1] -1.f), -1.0f,
            0.f, 0.f, 1.f));

    glClear(GL_DEPTH_BUFFER_BIT );
    glLineWidth(2.f);
    m_isolineShader.use();
    m_isolineShader.setMat3("pvmMx", pvmMx);
    for (
            float isovalue = m_cmClipMin;
            isovalue < m_cmClipMax;
            isovalue += m_isovalueInterval)
    {
        std::vector<util::geometry::Line2D> isolines =
            util::extractIsolines(dataTexture, isovalue);
        glm::vec4 color = glm::vec4(
                glm::vec3(
                    m_isolineColor[0], m_isolineColor[1], m_isolineColor[2]),
                1.f);
        m_isolineShader.setVec4("linecolor", color);

        for (auto &line : isolines)
            line.draw();
    }

    // show the rendering result as window filling quad
    glViewport(0, 0, m_windowDimensions[0], m_windowDimensions[1]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    m_windowShader.use();
    m_windowShader.setMat4("projMX", m_quadProjMx);

    glActiveTexture(GL_TEXTURE0);
    m_framebuffer.accessTextures()[0].bind();
    m_windowShader.setInt("renderTex", 0);

    m_windowQuad.draw();

    renderImgui();

    glfwSwapBuffers(m_window);


    if (printOpenGLError())
        return EXIT_FAILURE;
    else
        return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
int draw::Renderer::drawFractureNetwork(
        const fractureDataArray_t &dataArray)
{
    if (false == m_isInitialized)
    {
        std::cerr << "Error: Renderer::initialize() must be called "
            "successfully before Renderer::draw can be used!" <<
            std::endl;
        return EXIT_FAILURE;
    }

    // render the nine fractures into a frame buffer object
    for (size_t i = 0; i < 9; ++i)
    {
        // create a texture from the sampled data
        const boost::multi_array<double, 2>& data = dataArray[i];
        boost::multi_array<float, 2> dataTexture(
                boost::extents[data.shape()[0]][data.shape()[1]]);
        for (size_t y = 0; y < data.shape()[0]; ++y)
        for (size_t x = 0; x < data.shape()[1]; ++x)
            dataTexture[y][x] = static_cast<float>(data[y][x]);
        util::texture::Texture2D sampleTex(
                GL_R32F,
                GL_RED,
                0,
                GL_FLOAT,
                GL_LINEAR,
                GL_CLAMP_TO_EDGE,
                dataTexture.shape()[1],
                dataTexture.shape()[0],
                static_cast<void const*>(dataTexture.data()));

        // render the fracture into a 2D texture
        glViewport(
            0,
            0,
            FRACTURE_TEXTURE_RESOLUTION[0],
            FRACTURE_TEXTURE_RESOLUTION[1]);
        m_fractureFbo.bind();
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        m_fractureShader.use();
        m_fractureShader.setMat4("projMX", m_quadProjMx);
        m_fractureShader.setFloat("tfMin", m_cmClipMin);
        m_fractureShader.setFloat("tfMax", m_cmClipMax);

        glActiveTexture(GL_TEXTURE0);
        sampleTex.bind();
        m_fractureShader.setInt("sampleTex", 0);

        glActiveTexture(GL_TEXTURE1);
        if (m_cmSelect == 0)
            m_smoothcoolwarmMap.bind();
        else
            m_viridisMap.bind();
        m_fractureShader.setInt("tfTex", 1);

        m_windowQuad.draw();

        glm::mat3 pvmMx = glm::transpose(glm::mat3(
                2.f / (dataTexture.shape()[0] - 1.f), 0.f, -1.0f,
                0.f, 2.f / (dataTexture.shape()[1] -1.f), -1.0f,
                0.f, 0.f, 1.f));

        glLineWidth(2.f);
        m_isolineShader.use();
        m_isolineShader.setMat3("pvmMx", pvmMx);
        for (
                float isovalue = m_cmClipMin;
                isovalue < m_cmClipMax;
                isovalue += m_isovalueInterval)
        {
            std::vector<util::geometry::Line2D> isolines =
                util::extractIsolines(dataTexture, isovalue);
            glm::vec4 color = glm::vec4(
                    glm::vec3(
                        m_isolineColor[0],
                        m_isolineColor[1],
                        m_isolineColor[2]),
                    1.f);
            m_isolineShader.setVec4("linecolor", color);

            for (auto &line : isolines)
                line.draw();
        }
        m_fractureFbo.unbind();

        // map the fracture texture onto the according 3D plane and
        // draw it into the combined framebuffer object
        glViewport(0, 0, m_windowDimensions[0], m_windowDimensions[1]);
        m_framebuffer.bind();
        if (i == 0) glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        // draw the data into the framebuffer object
        m_planeShader.use();

        glActiveTexture(GL_TEXTURE0);
        m_fractureFbo.accessTextures()[0].bind();
        m_planeShader.setInt("fractureTex", 0);

        m_planeShader.setMat4("pvmMx",
            m_3dProjMx * m_3dViewMx * m_fractureModelMxs[i]);

        const glm::vec3 lightDir = glm::normalize(glm::vec3(1.f, 5.f, 1.f));
        m_planeShader.setVec3(
                "lightDir", lightDir[0], lightDir[1], lightDir[2]);


        m_fractureNetwork[i].draw();

    }
    // draw a frame around the domain
    glLineWidth(2.f);
    const glm::mat4 frameModelMx =
        glm::translate(glm::mat4(1.f), glm::vec3(0.5f));
    m_frameShader.use();
    m_frameShader.setMat4("pvmMX", m_3dProjMx * m_3dViewMx * frameModelMx);
    m_frameShader.setVec4("linecolor", 0.2f, 0.2f, 0.2f, 1.f);

    m_volumeFrame.draw();

    m_framebuffer.unbind();

    // show the rendering result as window filling quad
    glViewport(0, 0, m_windowDimensions[0], m_windowDimensions[1]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    m_windowShader.use();
    m_windowShader.setMat4("projMX", m_quadProjMx);

    glActiveTexture(GL_TEXTURE0);
    m_framebuffer.accessTextures()[0].bind();
    m_windowShader.setInt("renderTex", 0);

    m_windowQuad.draw();

    renderImgui();

    glfwSwapBuffers(m_window);


    if (printOpenGLError())
        return EXIT_FAILURE;
    else
        return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
// subroutines
//-----------------------------------------------------------------------------
int draw::Renderer::initializeGl3w()
{
    if (gl3wInit())
    {
        std::cerr << "Failed to initialize OpenGL" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }
    if (!gl3wIsSupported(
            REQUIRED_OGL_VERSION_MAJOR, REQUIRED_OGL_VERSION_MINOR))
    {
        std::cerr << "OpenGL " << REQUIRED_OGL_VERSION_MAJOR << "." <<
            REQUIRED_OGL_VERSION_MINOR << " not supported" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "OpenGL " << glGetString(GL_VERSION) << ", GLSL " <<
        glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    return EXIT_SUCCESS;
}

int draw::Renderer::initializeImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(m_window, false);
    ImGui_ImplOpenGL3_Init();

    ImGui::StyleColorsLight();

    return EXIT_SUCCESS;
}

GLFWwindow* draw::Renderer::createWindow(
    unsigned int width, unsigned int height, const char* title)
{
    glfwSetErrorCallback(error_cb);
    if (!glfwInit()) exit(EXIT_FAILURE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, REQUIRED_OGL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, REQUIRED_OGL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        width, height, title, nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetWindowUserPointer(window, this);

    // install callbacks
    glfwSetMouseButtonCallback(window, mouseButton_cb);
    glfwSetScrollCallback(window, scroll_cb);
    glfwSetKeyCallback(window, key_cb);
    glfwSetCharCallback(window, char_cb);
    glfwSetCursorPosCallback(window, cursorPosition_cb);
    glfwSetFramebufferSizeCallback(window, framebufferSize_cb);

    return window;
}

void draw::Renderer::reloadShaders()
{
    std::cout << "Reloading shaders..." << std::endl;
    m_windowShader = Shader(
            "src/draw/shader/windowQuad.vert",
            "src/draw/shader/windowQuad.frag");
    m_fractureShader = Shader(
            "src/draw/shader/fracture.vert",
            "src/draw/shader/fracture.frag");
    m_isolineShader = Shader(
            "src/draw/shader/isolines.vert",
            "src/draw/shader/isolines.frag");
    m_planeShader = Shader(
            "src/draw/shader/plane.vert",
            "src/draw/shader/plane.frag");
    m_frameShader = Shader(
            "src/draw/shader/frame.vert",
            "src/draw/shader/frame.frag");
}

// from imgui_demo.cpp
void draw::Renderer::createHelpMarker(const std::string description)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(description.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void draw::Renderer::renderImgui(void)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("inpreCICE menu");
    {
        ImGui::DragFloatRange2(
            "Transfer function interval", &m_cmClipMin, &m_cmClipMax, 0.001f);
        ImGui::Text("Select color map:");
        ImGui::RadioButton("coolwarm", &m_cmSelect, 0); ImGui::SameLine();
        ImGui::RadioButton("viridis", &m_cmSelect, 1);
        ImGui::DragFloat(
            "Isoline interval",
            &m_isovalueInterval,
            1e-5f,
            1e-5f,
            0.1f,
            "%.5f");
        ImGui::ColorEdit3("Isoline color", m_isolineColor.data());
        ImGui::Separator();
        ImGui::Checkbox("Demo Window", &m_showDemoWindow);
        ImGui::Separator();
        ImGui::Text(
            "Application average %.3f ms/frame (%.1f FPS)",
            1000.0f / ImGui::GetIO().Framerate,
            ImGui::GetIO().Framerate);
    }

    if(m_showDemoWindow) ImGui::ShowDemoWindow(&m_showDemoWindow);
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void draw::Renderer::updateFramebufferObjects(void)
{
    // for the final rendering result
    {
        std::vector<util::texture::Texture2D> fboTextures;
        fboTextures.emplace_back(
                GL_RGBA,
                GL_RGBA,
                0,
                GL_FLOAT,
                GL_LINEAR,
                GL_CLAMP_TO_BORDER,
                m_windowDimensions[0],
                m_windowDimensions[1]);

        fboTextures.emplace_back(
                GL_DEPTH_COMPONENT32,
                GL_DEPTH_COMPONENT,
                0,
                GL_FLOAT,
                GL_LINEAR,
                GL_CLAMP_TO_BORDER,
                m_windowDimensions[0],
                m_windowDimensions[1]);

        const std::vector<GLenum> attachments {
            GL_COLOR_ATTACHMENT0 , GL_DEPTH_ATTACHMENT };
        m_framebuffer = util::FramebufferObject(
                std::move(fboTextures), attachments);

    }

    // for textures of 3D fractures
    {
        std::vector<util::texture::Texture2D> fboTextures;
        fboTextures.emplace_back(
                GL_RGBA,
                GL_RGBA,
                0,
                GL_FLOAT,
                GL_LINEAR,
                GL_CLAMP_TO_BORDER,
                FRACTURE_TEXTURE_RESOLUTION[0],
                FRACTURE_TEXTURE_RESOLUTION[1]);
        const std::vector<GLenum> attachments { GL_COLOR_ATTACHMENT0 };
        m_fractureFbo = util::FramebufferObject(
                std::move(fboTextures), attachments);
    }
}

//-----------------------------------------------------------------------------
// GLFW callbacks and input processing
//-----------------------------------------------------------------------------
void draw::Renderer::cursorPosition_cb(
        GLFWwindow *window, double xpos, double ypos)
{
    static double xpos_old = 0.0;
    static double ypos_old = 0.0;
    double dx, dy;

    dx = xpos - xpos_old; xpos_old = xpos;
    dy = ypos - ypos_old; ypos_old = ypos;

    draw::Renderer *pThis =
        reinterpret_cast<draw::Renderer*>(glfwGetWindowUserPointer(window));

    if (GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT))
    {
        glm::vec3 tmp = pThis->m_cameraPosition - pThis->m_cameraLookAt;
        glm::vec3 polar = util::cartesianToPolar<glm::vec3>(tmp);
        float half_pi = glm::half_pi<float>();

        polar.y += glm::radians(dx) * pThis->m_cameraRotationSpeed;
        polar.z += glm::radians(dy) * pThis->m_cameraRotationSpeed;
        if (polar.z <= -0.999f * half_pi)
            polar.z = -0.999f * half_pi;
        else if (polar.z >= 0.999f * half_pi)
            polar.z = 0.999f * half_pi;

        pThis->m_cameraPosition =
            util::polarToCartesian<glm::vec3>(polar) + pThis->m_cameraLookAt;
    }
    else if (GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE))
    {
        glm::vec3 horizontal = glm::normalize(
            glm::cross(-(pThis->m_cameraPosition), glm::vec3(0.f, 1.f, 0.f)));
        glm::vec3 vertical = glm::vec3(0.f, 1.f, 0.f);
        pThis->m_cameraPosition +=
            static_cast<float>(
                -dx * pThis->m_cameraTranslationSpeed) * horizontal +
            static_cast<float>(
                dy * pThis->m_cameraTranslationSpeed) * vertical;
        pThis->m_cameraLookAt +=
            static_cast<float>(
                -dx * pThis->m_cameraTranslationSpeed) * horizontal +
            static_cast<float>(
                dy * pThis->m_cameraTranslationSpeed) * vertical;
    }

    glm::vec3 right = glm::normalize(
        glm::cross(-pThis->m_cameraPosition, glm::vec3(0.f, 1.f, 0.f)));
    glm::vec3 up = glm::normalize(glm::cross(right, -pThis->m_cameraPosition));

    pThis->m_3dViewMx = glm::lookAt(
            pThis->m_cameraPosition, pThis->m_cameraLookAt, up);

}

void draw::Renderer::mouseButton_cb(
        GLFWwindow* window, int button, int action, int mods)
{
    // chain ImGui callback
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}

void draw::Renderer::scroll_cb(GLFWwindow *window, double xoffset, double yoffset)

{
    draw::Renderer *pThis =
        reinterpret_cast<draw::Renderer*>(glfwGetWindowUserPointer(window));

    if ((GLFW_PRESS == glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) ||
        (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL)))
    {
        // y scrolling changes the distance of the camera from the origin
        pThis->m_cameraPosition +=
            static_cast<float>(-yoffset) *
            pThis->m_cameraZoomSpeed *
            pThis->m_cameraPosition;
    }

    glm::vec3 right = glm::normalize(
        glm::cross(-pThis->m_cameraPosition, glm::vec3(0.f, 1.f, 0.f)));
    glm::vec3 up = glm::normalize(glm::cross(right, -pThis->m_cameraPosition));

    pThis->m_3dViewMx = glm::lookAt(
            pThis->m_cameraPosition, pThis->m_cameraLookAt, up);

    // chain ImGui callback
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}

void draw::Renderer::key_cb(
        GLFWwindow* window, int key, int scancode , int action, int mods)
{
    draw::Renderer *pThis =
        reinterpret_cast<draw::Renderer*>(glfwGetWindowUserPointer(window));

    if((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS))
        glfwSetWindowShouldClose(window, true);

    if((key == GLFW_KEY_F5) && (action == GLFW_PRESS))
        pThis->reloadShaders();

    if((key == GLFW_KEY_F9) && (action == GLFW_PRESS))
    {
        std::time_t t = std::time(nullptr);
        std::tm* tm = std::localtime(&t);
        char filename[200];

        strftime(
                filename,
                sizeof(filename),
                "./screenshots/%F_%H%M%S.png",
                tm);

        util::makeScreenshot(
            pThis->m_framebuffer,
            pThis->m_windowDimensions[0],
            pThis->m_windowDimensions[1],
            filename,
            FIF_PNG);
        std::cout << "Saved screenshot " << filename << std::endl;
    }
    // chain ImGui callback
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
}

void draw::Renderer::char_cb(GLFWwindow* window, unsigned int c)
{
    // chain ImGui callback
    ImGui_ImplGlfw_CharCallback(window, c);
}

void draw::Renderer::framebufferSize_cb(
    __attribute__((unused)) GLFWwindow* window,
    int width,
    int height)
{
    draw::Renderer *pThis =
        reinterpret_cast<draw::Renderer*>(glfwGetWindowUserPointer(window));

    pThis->m_windowDimensions[0] = width;
    pThis->m_windowDimensions[1] = height;

    pThis->updateFramebufferObjects();
}

void draw::Renderer::error_cb(int error, const char* description)
{
    std::cerr << "Glfw error " << error << ": " << description << std::endl;
}

void draw::Renderer::processInput()
{
    // place holder
}

