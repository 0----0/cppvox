#include <GL/glew.h>
#include <iostream>
#include <vector>

namespace GLLib {

class Buffer {
private:
        GLuint id = 0;
public:
        GLuint getID() { return id; }

        static Buffer create() {
                Buffer self;
                glGenBuffers(1, &self.id);
                return self;
        }
        ~Buffer() {
                if (isAlive()) glDeleteBuffers(1, &id);
        }

        void fill(uint size, void* data, GLenum usage) {
                glBindBuffer(GL_ARRAY_BUFFER, id);
                glBufferData(GL_ARRAY_BUFFER, size, data, usage);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        template<typename T>
        void fill(std::vector<T>& vec, GLenum usage) {
                fill(vec.size() * sizeof(T), vec.data(), usage);
        }

        const bool isAlive() const noexcept { return id != 0; }
};

};
