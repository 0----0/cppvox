#include <GL/glew.h>
#include <iostream>
#include <vector>

namespace GLLib {

class VertexArray {
private:
        GLuint id = 0;
        VertexArray() {}
public:
        GLuint getID() { return id; }

        static VertexArray create() {
                VertexArray self;
                glGenVertexArrays(1, &self.id);
                return self;
        }
        ~VertexArray() {
                if (isAlive()) glDeleteVertexArrays(1, &id);
        }

        void use() const {
                glBindVertexArray(id);
        }

        const bool isAlive() const noexcept { return id != 0; }
};

};
