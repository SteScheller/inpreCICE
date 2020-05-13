#include <iostream>
#include <cstdlib>
#include <ctime>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "draw.hpp"
#include "viridis.hpp"
#include "shader.hpp"

#include "../util/util.hpp"

#define REQUIRED_OGL_VERSION_MAJOR 3
#define REQUIRED_OGL_VERSION_MINOR 3

//-----------------------------------------------------------------------------
// draw class construction and destruction
//-----------------------------------------------------------------------------
draw::Renderer::Renderer() :
    m_window(nullptr),
    m_windowDimensions{ {1280, 720} },
    m_isInitialized(false),
    m_cmClipMin(0.f),
    m_cmClipMax(0.01f),
    m_viridisMap(),
    m_sampleShader(),
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

    glEnable(GL_CULL_FACE);
    glEnable(GL_LINE_SMOOTH);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.f, 0.f, 0.f, 1.f);


    //-------------------------------------------------------------------------
    // shader setup
    //-------------------------------------------------------------------------
    m_sampleShader = Shader(
            "src/draw/shader/sample.vert", "src/draw/shader/sample.frag");
    m_isolineShader = Shader(
            "src/draw/shader/isolines.vert", "src/draw/shader/isolines.frag");

    // ------------------------------------------------------------------------
    // geometry
    // ------------------------------------------------------------------------
    m_windowQuad = util::geometry::Quad(true);

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

    //-------------------------------------------------------------------------
    // framebuffer object for deferred shading
    //-------------------------------------------------------------------------
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

    const std::vector<GLenum> attachments { GL_COLOR_ATTACHMENT0 };

    m_framebuffer = util::FramebufferObject(
            std::move(fboTextures), attachments);

    if (EXIT_SUCCESS == ret)
        m_isInitialized = true;

    return ret;
}

bool draw::Renderer::processEvents()
{
    if (false == m_isInitialized)
    {
        std::cerr << "Error: Renderer::initialize() must be called "
            "successfully before Renderer::processEvents(...) can be used!" <<
            std::endl;
        return false;
    }

    if (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
        processInput();
        return true;
    }
    else
        return false;
}

int draw::Renderer::draw(const boost::multi_array<double, 2> &data)
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

    glViewport(0, 0, m_windowDimensions[0], m_windowDimensions[1]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // draw the data
    m_sampleShader.use();
    m_sampleShader.setMat4("projMX", m_quadProjMx);
    m_sampleShader.setFloat("tfMin", m_cmClipMin);
    m_sampleShader.setFloat("tfMax", m_cmClipMax);

    glActiveTexture(GL_TEXTURE0);
    sampleTex.bind();
    m_sampleShader.setInt("sampleTex", 0);

    glActiveTexture(GL_TEXTURE1);
    m_viridisMap.bind();
    m_sampleShader.setInt("tfTex", 1);

    m_windowQuad.draw();

    glm::mat3 pvmMx = glm::transpose(glm::mat3(
            2.f / (dataTexture.shape()[0] - 1.f), 0.f, -1.0f,
            0.f, 2.f / (dataTexture.shape()[1] -1.f), -1.0f,
            0.f, 0.f, 1.f));

    m_isolineShader.use();
    m_isolineShader.setMat3("pvmMx", pvmMx);
    for (float isovalue = 1e-3f; isovalue < 1e-2f; isovalue += 1e-4f)
    {
        std::vector<util::geometry::Line2D> isolines =
            util::extractIsolines(dataTexture, isovalue);
        glm::vec4 color = glm::vec4(
                glm::vec3(
                    1.f,
                    1.f - isovalue * 100.f,
                    isovalue * 100.f),
                1.f);
        m_isolineShader.setVec4("linecolor", color);
        printOpenGLError();

        for (auto &line : isolines)
            line.draw();
        printOpenGLError();
    }

    // draw ImGui windows
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("inpreCICE menu");
    {
        ImGui::DragFloatRange2(
            "Transfer function interval", &m_cmClipMin, &m_cmClipMax, 0.001f);
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

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init();

    ImGui::StyleColorsDark();

    return EXIT_SUCCESS;
}

GLFWwindow* draw::Renderer::createWindow(
    unsigned int width, unsigned int height, const char* title)
{
    if (!glfwInit()) exit(EXIT_FAILURE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        width, height, title, nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetWindowUserPointer(window, this);

    // install callbacks
    glfwSetErrorCallback(error_cb);
    glfwSetFramebufferSizeCallback(window, framebufferSize_cb);

    return window;
}

void draw::Renderer::reloadShaders()
{
    std::cout << "Reloading shaders..." << std::endl;
    m_sampleShader = Shader(
            "src/draw/shader/sample.vert",
            "src/draw/shader/sample.frag");
    m_isolineShader = Shader(
            "src/draw/shader/isolines.vert",
            "src/draw/shader/isolines.frag");
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

//-----------------------------------------------------------------------------
// GLFW callbacks and input processing
//-----------------------------------------------------------------------------
void draw::Renderer::framebufferSize_cb(
    __attribute__((unused)) GLFWwindow* window,
    int width,
    int height)
{
    draw::Renderer *pThis =
        reinterpret_cast<draw::Renderer*>(glfwGetWindowUserPointer(window));

    pThis->m_windowDimensions[0] = width;
    pThis->m_windowDimensions[1] = height;
}

void draw::Renderer::error_cb(int error, const char* description)
{
    std::cerr << "Glfw error " << error << ": " << description << std::endl;
}

void draw::Renderer::processInput()
{
    if(glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_window, true);

    if(glfwGetKey(m_window, GLFW_KEY_F5) == GLFW_PRESS)
        reloadShaders();

    if(glfwGetKey(m_window, GLFW_KEY_F9) == GLFW_PRESS)
    {
        std::time_t t = std::time(nullptr);
        std::tm* tm = std::localtime(&t);
        char filename[200];

        strftime(
                filename,
                sizeof(filename),
                "./screenshots/%F_%H%M%S.tiff",
                tm);

        util::makeScreenshot(
            m_framebuffer,
            m_windowDimensions[0],
            m_windowDimensions[1],
            filename,
            FIF_TIFF);
        std::cout << "Saved screenshot " << filename << std::endl;
    }
}

