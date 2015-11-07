#include "types.hpp"
#include <cmath>
#include <vector>
#include "Morton.hpp"

namespace Perlin {

float noise1d(uint x) {
        x = (x << 13) ^ x;
        return (1.0f - ( (x * (x * 15731 + 789221 + 1376312589) & 0x7fffffff) / 1073741824.0f));
}

float noise2d(uint x, uint y) {
        return noise1d(x + y * 71);
}

float noise3d(uint x, uint y, uint z) {
        return noise1d(x + y * 67 + z * 101);
}

float cosinInterp(float a, float b, float x) {
        float f = (1.0f - cos(x * M_PI))/2.0f;
        return (b - a) * f + a;
}

float interpNoise2d(float x, float y) {
        float ix = (int)(floor(x));
        float iy = (int)(floor(y));
        float fx = x - floor(x);
        float fy = y - floor(y);
        float v00 = noise2d(ix, iy);
        float v10 = noise2d(ix + 1, iy);
        float v01 = noise2d(ix, iy + 1);
        float v11 = noise2d(ix + 1, iy + 1);
        float u0 = cosinInterp(v00, v10, fx);
        float u1 = cosinInterp(v01, v11, fx);
        return cosinInterp(u0, u1, fy);
}

float perlinNoise2d(float x, float y) {
        float result = 0.0f;
        for (int i = 0; i < 5; i++) {
                result += interpNoise2d(x * exp2f(i), y * exp2f(i)) * exp2f(-i);
        }
        return result;
}

class CachedHeightmapGenerator {
private:
        std::vector<uint> cachedHeights;
        int scale;
public:
        CachedHeightmapGenerator(int scale): scale(scale) {}
        uint getHeight(uint x, uint y, uint z) {
                uint cacheIdx = Morton::encode2(x, y);
                float scalef = exp2f(scale);
                if (z == 0) {
                        uint height = (Perlin::perlinNoise2d(x/scalef, y/scalef) + 2.0f) * scalef/2.0f;
                        if (cachedHeights.size() <= cacheIdx) {
                                cachedHeights.resize(cacheIdx + 1);
                        }
                        cachedHeights[cacheIdx] = height;
                        return height;
                }
                else {
                        return cachedHeights[cacheIdx];
                }

        }
};

}
