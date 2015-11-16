#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window {
public:
        GLFWwindow* window;

        static Window create(int width, int height, const char* name = "Untitled window") {
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
                auto self = Window{glfwCreateWindow(width, height, name, 0, 0)};

                glfwMakeContextCurrent(self.window);
                glfwSwapInterval(0);
                glewExperimental = GL_TRUE;
                glewInit();

                return self;
        }

        bool shouldClose() { return glfwWindowShouldClose(window); }

        bool getKey(int key) { return glfwGetKey(window, key); }

        ~Window() {
                glfwDestroyWindow(window);
        }
};
