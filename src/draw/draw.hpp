#pragma once

#include <array>
#include <functional>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "shader.hpp"
#include "util/util.hpp"

#include <boost/multi_array.hpp>

namespace draw
{
    /**
     * \brief   class providing visual ouput
     */
    class Renderer
    {
        //---------------------------------------------------------------------
        // class-wide constants and default values
        //---------------------------------------------------------------------
        static constexpr int REQUIRED_OGL_VERSION_MAJOR = 3;
        static constexpr int REQUIRED_OGL_VERSION_MINOR = 3;

        static const glm::vec3 DEFAULT_CAMERA_POSITION;
        static const glm::vec3 DEFAULT_CAMERA_LOOKAT;

        static const std::array<size_t, 2>FRACTURE_TEXTURE_RESOLUTION;

        public:
        Renderer();
        Renderer(unsigned int winWidth, unsigned int winHeight);
        ~Renderer();

        /**
         * \brief Initializes everything needed for drawing
         *
         * \return EXIT_SUCCESS after succesful initialization,
         *         error code otherwise
         *
         * Initializes the window and OpenGL context, persistent config
         * variables and other stuff.
         */
        int initialize();

        /**
         * \brief Cyclic event processing function
         *
         * \return true as long as the window is still open, false when
         *         it was closed
         *
         * Event processing function that has to be called periodically in
         * order to process events like resizing and closing of the drawing
         * window.
         */
        bool processEvents();

        /**
         * \brief Draw call for visualization of the single fracture case
         *
         * \param data  scalar data values
         *
         * \return EXIT_SUCCESS true as long as the window is still open,
         *         false when it was closed
         *
         * Draws the scalar data values into a window spanning quad using a
         * color map and isolines.
         */
        using fractureData_t = boost::multi_array<double, 2>;
        int drawSingleFracture(const fractureData_t &data);

        /**
         * \brief Draw call for visualization of the fracture network case
         *
         * \param dataArray  array of scalar data values for the 9 fractures
         *
         * \return EXIT_SUCCESS true as long as the window is still open,
         *         false when it was closed
         *
         * Draws the scalar data values as texture onto 3D planes representing
         * the fracture network.
         */
        using fractureDataArray_t =
                std::array<std::reference_wrapper<const fractureData_t>, 9>;
        int drawFractureNetwork(const fractureDataArray_t &dataArray);

        private:
        GLFWwindow* m_window;
        std::array<unsigned int, 2> m_windowDimensions;

        bool m_isInitialized;

        // clipping values for color mapping
        float m_cmClipMin;
        float m_cmClipMax;
        int m_cmSelect;


        // interval between isolines
        float m_isovalueInterval;
        std::array<float, 3> m_isolineColor;

        // fracture network geometry
        std::array<util::geometry::Quad, 9> m_fractureNetwork;
        std::array<glm::mat4, 9> m_fractureModelMxs;

        // 3D visualization objects
        float m_fovY;
        float m_zNear;
        float m_zFar;
        glm::vec3 m_cameraPosition;
        glm::vec3 m_cameraLookAt;
        float m_cameraZoomSpeed;
        float m_cameraRotationSpeed;
        float m_cameraTranslationSpeed;

        glm::mat4 m_3dViewMx;
        glm::mat4 m_3dProjMx;

        util::FramebufferObject m_fractureFbo;

        Shader m_planeShader;
        Shader m_frameShader;

        util::geometry::CubeFrame m_volumeFrame;

        // common rendering objects
        util::FramebufferObject m_framebuffer;
        util::texture::Texture2D m_viridisMap;
        util::texture::Texture2D m_smoothcoolwarmMap;
        Shader m_windowShader;
        Shader m_fractureShader;
        Shader m_isolineShader;
        util::geometry::Quad m_windowQuad;
        glm::mat4 m_quadProjMx;

        bool m_showDemoWindow;

        //---------------------------------------------------------------------
        // helper functions
        //---------------------------------------------------------------------
        GLFWwindow* createWindow(
                unsigned int win_w,
                unsigned int win_h,
                const char* title);
        int initializeGl3w();
        int initializeImGui();

        void processInput();

        void reloadShaders();

        void createHelpMarker(const std::string description);

        void renderImgui(void);

        void updateFramebufferObjects();

        //---------------------------------------------------------------------
        // glfw callback functions
        //---------------------------------------------------------------------
        static void cursorPosition_cb(
                GLFWwindow *window,
                double xpos,
                double ypos);

        static void mouseButton_cb(
                GLFWwindow* window,
                int button,
                int action,
                int mods);

        static void scroll_cb(
                GLFWwindow* window,
                double xoffset,
                double yoffset);

        static void key_cb(
                GLFWwindow* window,
                int key,
                int scancode,
                int action,
                int mods);

        static void char_cb(
                GLFWwindow* window,
                unsigned int c);

        static void framebufferSize_cb(
                GLFWwindow* window,
                int width,
                int height);

        static void error_cb(
                int error,
                const char* description);

    };
}

