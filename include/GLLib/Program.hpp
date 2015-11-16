#include <GL/glew.h>
#include "GLLib/Shader.hpp"
#include <iostream>

namespace GLLib {

class Program {
private:
        GLuint id = 0;
public:
        GLuint getID() { return id; }

        Program(): id(glCreateProgram()) {}
        // static Program create() {
        //         Program self;
        //         self.id = glCreateProgram();
        //         return self;
        // }
        ~Program() {
                if (isAlive()) glDeleteProgram(id);
        }

        const bool isAlive() const noexcept { return id != 0; }

        GLint getInt(GLenum name) {
                GLint result;
                glGetProgramiv(id, name, &result);
                return result;
        }

        std::string getInfoLog() {
                auto len = getInt(GL_INFO_LOG_LENGTH);
                std::cout << len << std::endl;
                if (len == 0) { return ""; }
                std::string infolog;
                infolog.resize(len);
                glGetProgramInfoLog(id, len, 0, &infolog[0]);
                return infolog;
        }

        void addShader(const Shader& shader) {
                glAttachShader(id, shader.getID());
        }

        void link() {
                glLinkProgram(id);
                if (!getInt(GL_LINK_STATUS)) {
                        std::cout << "Hi!" << getInfoLog() << std::endl;
                }
        }
        void use() { glUseProgram(id); }

        auto getUniformLoc(const char* name) { return glGetUniformLocation(id, name); }
};

};
