#include "GLLib/GLLib.hpp"
#include <GLFW/glfw3.h>
#include "Timer/Timer.hpp"
#include "Camera.hpp"
#include "types.hpp"

#include <bitset>
template <typename T> auto bits(T x) {
        return std::bitset<sizeof(T)*8>(*(unsigned long long*)&x);
}

#include "Func/Func.hpp"
struct PreVoxelOctreeNode {
        uint childIdxs[8] = {0};
        uint8 validMask = 0;
        uint8 leafMask = 0;
        uint subtreeSize = 0;

        void print() const {
                std::cout << bits(validMask) << '\t' << bits(leafMask) << '\t' << subtreeSize << std::endl;
                for (auto i : range(0,8)) {
                        std::cout << i << ": " << childIdxs[i] << std::endl;
                }
        }
};

#include <vector>
struct PreVoxelOctree {
        std::vector<PreVoxelOctreeNode> nodePool;

        template<typename MVoxIterT>
        bool addSubtree(uint size, MVoxIterT& vox) {
                if (size == 0) { bool v = *vox; ++vox; return v; }

                uint nodeIdx = nodePool.size();
                nodePool.emplace_back();

                for(int i = 0; i < 8; i++) {
                        uint curIdx = nodePool.size();
                        nodePool[nodeIdx].childIdxs[i] = curIdx;

                        bool isLeaf = addSubtree(size - 1, vox);
                        if (nodePool.size() != curIdx) {
                                nodePool[nodeIdx].validMask |= 1 << i;
                        } else if (isLeaf) {
                                nodePool[nodeIdx].validMask |= 1 << i;
                                nodePool[nodeIdx].leafMask |= 1 << i;
                        }
                }

                nodePool[nodeIdx].subtreeSize = nodePool.size() - nodeIdx - 1;
                if (nodePool[nodeIdx].leafMask == u'\xFF') {
                        if (nodeIdx != 0) nodePool.pop_back();
                        return true;
                }
                else if (nodePool[nodeIdx].validMask == u'\x00') {
                        if (nodeIdx != 0) nodePool.pop_back();
                }
                return false;
        }

        void print() const {
                for (auto n : nodePool) { n.print(); }
        }
};

#include "Morton.hpp"
#include "Perlin.hpp"
struct SimpleMvoxIter {
        uint64 idx = 0;
        uint64 offset = 0;
        mutable Perlin::CachedHeightmapGenerator gen{8};

        bool operator*() const {
                uint x, y, z;
                std::tie(x, y, z) = Morton::decode(idx);
                uint height = gen.getHeight(x, y, z);
                if (z == 0) { return true; }
                else if (z < height) { return true; }
                else { return false; }
        }
        void operator++() {
                ++idx;
        }
};

struct VoxelNode {
        uint16 _childPtr;
        uint8 validMask;
        uint8 leafMask;

        void setChildPtr(uint16 offset, bool far) {
                _childPtr = offset << 1;
                if (far) _childPtr |= 1;
        }
        uint16 getChildPtr() const {
                return _childPtr >> 1;
        }

        void print() const {
                std::cout << bits(validMask) << '\t';
                std::cout << bits(leafMask) << '\t';
                std::cout << getChildPtr() << std::endl;
        }
};

union NodeOrFarPtr {
        VoxelNode node;
        uint32 farptr;
};

uint8 popCount(uint8 x) {
        x -= (x >> 1) & '\x55';
        x = (x & '\x33') + ((x >> 2) & '\x33');
        return (x + (x >> 4)) & 0x0f;
}

struct VoxelOctree {
        std::vector<NodeOrFarPtr> nodes;

        static VoxelOctree create() {
                VoxelOctree self;
                PreVoxelOctree preOct;
                SimpleMvoxIter iter;

                preOct.addSubtree(10, iter);

                VoxelNode root;
                root.validMask = preOct.nodePool[0].validMask;
                root.leafMask = preOct.nodePool[0].leafMask;
                root.setChildPtr(1, false);
                self.nodes.push_back(NodeOrFarPtr{root});
                self.addSubtree(preOct, 0, 0);

                return self;
        }

        void addSubtree(PreVoxelOctree& preOct, uint curPreIdx, uint curNodeIdx) {
                uint startPos = nodes.size();

                PreVoxelOctreeNode& curPreNode = preOct.nodePool[curPreIdx];
                auto validNonLeaves = curPreNode.validMask ^ curPreNode.leafMask;
                auto isValidNonLeaf = [&](auto i) -> bool { return validNonLeaves & 1 << i; };
                for (int i : filter(isValidNonLeaf, range(0,8))) {
                        auto& curPreChild = preOct.nodePool[curPreNode.childIdxs[i]];
                        VoxelNode node;
                        node.validMask = curPreChild.validMask;
                        node.leafMask = curPreChild.leafMask;
                        nodes.push_back(NodeOrFarPtr{node});
                }

                uint sum = 0;
                uint farPtrs[8] = {0};
                for (int i : filter(isValidNonLeaf, range(0, 8))) {
                        uint8 bitmask = uint8(~0) >> (8 - i);
                        auto numPrevChildren = popCount(validNonLeaves & bitmask);
                        auto childIdx = startPos + numPrevChildren;
                        auto offset = sum + nodes.size() - childIdx;
                        if (offset > 0x7fff) {
                                farPtrs[i] = nodes.size();
                                nodes.push_back(NodeOrFarPtr{{0}});
                                continue;
                        }
                        sum += preOct.nodePool[curPreNode.childIdxs[i]].subtreeSize;
                }

                for (int i : filter(isValidNonLeaf, range(0,8))) {
                        uint8 bitmask = uint8(~0) >> (8 - i);
                        auto numPrevChildren = popCount(validNonLeaves & bitmask);
                        auto childIdx = startPos + numPrevChildren;

                        if (farPtrs[i] == 0) {
                                nodes[childIdx].node.setChildPtr(nodes.size() - childIdx, false);
                        }
                        else {
                                nodes[childIdx].node.setChildPtr(farPtrs[i] - childIdx, true);
                                nodes[farPtrs[i]].farptr = nodes.size() - farPtrs[i];
                        }
                        addSubtree(preOct, curPreNode.childIdxs[i], childIdx);
                }
        }

        void print() const {
                for (auto n : nodes) {
                        n.node.print();
                }
        }
};

#include <cerrno>
#include <fstream>

std::string readFile(const char *filename)
{
        std::ifstream in(filename, std::ios::in | std::ios::binary);
        if (in) {
                std::string contents;
                in.seekg(0, std::ios::end);
                contents.resize(in.tellg());
                in.seekg(0, std::ios::beg);
                in.read(&contents[0], contents.size());
                in.close();
                return(contents);
        }
        throw(errno);
}


class VoxelRenderer {
private:
        GLFWwindow* window;
        GLLib::Buffer octreeBuffer = GLLib::Buffer::create();
        GLuint octreeTexture = 0;
        GLLib::Buffer quadBuffer = GLLib::Buffer::create();
        GLLib::Buffer quadIdxBuffer = GLLib::Buffer::create();
        GLLib::Program program;
        GLLib::VertexArray vao = GLLib::VertexArray::create();

        VoxelRenderer(GLFWwindow* window): window(window) {}
public:
        static VoxelRenderer create(GLFWwindow* window) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                auto self = VoxelRenderer{window};
                self.initProgram();

                return self;
        }

        void initProgram() {
                auto vert = GLLib::Shader::fromString(GL_VERTEX_SHADER, readFile("../src/VoxelShaderVert.glsl").c_str());
                auto frag = GLLib::Shader::fromString(GL_FRAGMENT_SHADER, readFile("../src/VoxelShaderFrag.glsl").c_str());
                for (auto& shad : {vert, frag}) {
                        program.addShader(shad);
                }
                program.link();
                program.use();

                glVertexArrayAttribFormat(vao.getID(), 0, 2, GL_FLOAT, GL_FALSE, 0);
                glVertexArrayAttribBinding(vao.getID(), 0, 0);
                glEnableVertexArrayAttrib(vao.getID(), 0);

                auto quadGeomVerts = std::vector<float> {
                        -1, -1,
                        -1, 1,
                        1, -1,
                        1, 1
                };
                auto quadGeomIdxs = std::vector<uint> {
                        0, 1, 2,
                        1, 3, 2
                };

                quadBuffer.fill(quadGeomVerts, GL_STATIC_DRAW);
                quadIdxBuffer.fill(quadGeomIdxs, GL_STATIC_DRAW);

                glVertexArrayVertexBuffer(vao.getID(), 0, quadBuffer.getID(), 0, sizeof(float)*2);
                glVertexArrayElementBuffer(vao.getID(), quadIdxBuffer.getID());
        }

        void loadOctree(VoxelOctree& oct) {
                glDeleteTextures(1, &octreeTexture);
                glCreateTextures(GL_TEXTURE_BUFFER, 1, &octreeTexture);
                octreeBuffer.fill(oct.nodes, GL_STATIC_DRAW);
                glTextureBuffer(octreeTexture, GL_R32UI, octreeBuffer.getID());
                auto nodePool = program.getUniformLoc("nodePool");
                glUniform1i(nodePool, 0);
                glBindTextureUnit(0, octreeTexture);
        }

        void render(const Camera& camera) {
                glfwMakeContextCurrent(window);
                glClearColor(0.0f, 0.3f, 0.2f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                vao.use();
                auto camUni = program.getUniformLoc("camera");
                glUniformMatrix4fv(camUni, 1, GL_FALSE, glm::value_ptr(camera.getTransform()));

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

                glfwSwapBuffers(window);
        }
};


#include <cmath>
void simpleCameraMotion(Camera& camera, Window& window) {
        float speed = exp2(-8.0f);
        auto movement = glm::vec3(0, 0, 0);
        if(window.getKey(GLFW_KEY_W)) {
                movement.y += 1;
        }
        if(window.getKey(GLFW_KEY_A)) {
                movement.x += -1;
        }
        if(window.getKey(GLFW_KEY_S)) {
                movement.y += -1;
        }
        if(window.getKey(GLFW_KEY_D)) {
                movement.x += 1;
        }
        movement *= speed;
        camera.move(movement);

        float rotSpeed = M_PI / 64.0f;
        auto rotation = glm::vec2(0, 0);
        if (window.getKey(GLFW_KEY_UP)) {
                rotation.y += 1;
        }
        if (window.getKey(GLFW_KEY_DOWN)) {
                rotation.y += -1;
        }
        if (window.getKey(GLFW_KEY_LEFT)) {
                rotation.x += -1;
        }
        if (window.getKey(GLFW_KEY_RIGHT)) {
                rotation.x += 1;
        }
        rotation *= rotSpeed;
        camera.rotation += rotation;
}

#include "glm/ext.hpp"
int main() {
        if (!glfwInit()) return -1;

        auto window = Window::create(1024, 768, "Hello, world!");
        auto renderer = VoxelRenderer::create(window.window);

        auto camera = Camera::create();

        auto oct = VoxelOctree::create();

        renderer.loadOctree(oct);

        while(!window.shouldClose()) {
                Timer timer;
                simpleCameraMotion(camera, window);
                // std::cout << glm::to_string(camera.position) << std::endl;
                // std::cout << glm::to_string(rotatex(camera.rotation.y) * glm::vec4(0, 1, 0, 1)) << std::endl;
                // std::cout << glm::to_string(camera.getRotationTransform() * glm::vec4(0, 1, 0, 1)) << std::endl;
                // std::cout << glm::to_string(camera.getTransform() * glm::vec4(0, 1, 0, 1)) << std::endl;
                glfwPollEvents();
                renderer.render(camera);
                timer.roundTo(std::chrono::microseconds(16666));
        }

        glfwTerminate();
        return 0;
}
