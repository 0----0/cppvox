#ifndef __MORTON_HPP
#define __MORTON_HPP

#include "types.hpp"
#include <tuple>

namespace Morton {

uint deinterleave(uint x) {
        x |= x >> 2;
        x &= 0xc30c30c3;
        x |= (x >> 4);
        x &= 0x0f00f00f;
        x |= x >> 8;
        x &= 0xFF0000FF;
        x |= x >> 16;
        x &= 0x0000FFFF;
        return x;
}

auto decode(uint m) {
        uint mask = 0x49249249;
        uint x = deinterleave(m & mask);
        uint y = deinterleave((m >> 1) & mask);
        uint z = deinterleave((m >> 2) & mask);
        return std::make_tuple(x, y, z);
}

uint interleave2(uint x) {
        x |= x << 8;
        x &= 0x00FF00FF;
        x |= x << 4;
        x &= 0x0F0F0F0F;
        x |= x << 2;
        x &= 0x33333333;
        x |= x << 1;
        x &= 0x55555555;
        return x;
}

uint encode2(uint x, uint y) {
        return interleave2(x) | (interleave2(y) << 1);
}

}

#endif //__MORTON_HPP
