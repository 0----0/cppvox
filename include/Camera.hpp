#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

auto rotatex(float x) {
        return glm::rotate(x, glm::vec3(1.0f, 0.0f, 0.0f));
}

auto rotatey(float y) {
        return glm::rotate(y, glm::vec3(0.0f, 1.0f, 0.0f));
}

auto rotatez(float z) {
        return glm::rotate(z, glm::vec3(0.0f, 0.0f, 1.0f));
}

class Camera {
public:
        glm::vec3 position = glm::vec3(0, 0, 0);
        glm::vec2 rotation = glm::vec2(0, 0);

        static Camera create() {
                return Camera {
                        glm::vec3(0.0f, 0.0f, 0.0f),
                        glm::vec2(0.0f, 0.0f)
                };
        }

        glm::mat4 getRotationTransform() const {
                return rotatez(-rotation.x) * rotatex(rotation.y);
        }

        void move(glm::vec3 movement) {
                position += glm::vec3(rotatez(-rotation.x) * rotatex(rotation.y) * glm::vec4(movement, 1));
        }

        glm::mat4 getTransform() const {
                // float fixCoords[16] = {1.0f,0.0f,0.0f, 0.0f,0.0f,-1.0f,0.0f, 0.0f,1.0f,0.0f,0.0f, 0.0f,0.0f,0.0f,1.0f};
                // return glm::perspectiveFov(90.0f, 1024.0f, 768.0f, 0.125f, 1024.0f)
                return // glm::perspective(90.0f, 1.0f, 0.125f, 1024.0f)
                        //  glm::make_mat4(fixCoords)
                        glm::translate(position) *
                        getRotationTransform();
        }
};
