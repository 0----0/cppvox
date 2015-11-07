#include <GL/glew.h>
#include <string>
#include <iostream>

namespace GLLib {

class Shader {
private:
        GLuint id = 0;
public:
        auto getID() const { return id; }
        static Shader create(GLenum type) {
                Shader self;
                self.id = glCreateShader(type);
                return self;
        }
        ~Shader() {
                glDeleteShader(id);
        }

        GLint getInt(GLenum name) {
                GLint result;
                glGetShaderiv(id, name, &result);
                return result;
        }

        std::string getInfoLog() {
                auto len = getInt(GL_INFO_LOG_LENGTH);
                if (len == 0) { return ""; }
                std::string infolog;
                infolog.resize(len);
                glGetShaderInfoLog(id, len, 0, &infolog[0]);
                return infolog;
        }

        void compileFromSource(const char* shaderSource) {
                glShaderSource(id, 1, &shaderSource, NULL);
                glCompileShader(id);

                if (!getInt(GL_COMPILE_STATUS)) {
                        std::cout << "HI!" << std::endl;
                        auto infolog = getInfoLog();
                        std::cout << infolog;
                }
        }

        static Shader fromString(GLenum type, const char* shaderSource) {
                auto self = Shader::create(type);
                self.compileFromSource(shaderSource);
                return self;
        }
        const bool isAlive() const noexcept { return id != 0; }
};

};
