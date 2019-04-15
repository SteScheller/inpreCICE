#pragma once

#include <array>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "shader.hpp"
#include "../util/util.hpp"

#include <boost/multi_array.hpp>

namespace draw
{
    /**
     * \brief   class providing visual ouput
     */
    class Renderer
    {
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
         * \brief Cyclic draw call
         *
         * \return EXIT_SUCCESS true as long as the window is still open, false when
         *         it was closed
         *
         * Event processing function that has to be called periodically in
         * order to process events like resizing and closing of the drawing
         * window.
         */
        int draw(const boost::multi_array<double, 2> &data);

        private:
        GLFWwindow* m_window;
        std::array<unsigned int, 2> m_windowDimensions;

        bool m_isInitialized;

        // clipping values for color mapping
        float m_cmClipMin;
        float m_cmClipMax;

        util::FramebufferObject m_framebuffer;
        util::texture::Texture2D m_viridisMap;
        Shader m_sampleShader;
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

        //---------------------------------------------------------------------
        // glfw callback functions
        //---------------------------------------------------------------------
        static void framebufferSize_cb(
                GLFWwindow* window,
                int width,
                int height);
        static void error_cb(
                int error,
                const char* description);

    };
}

