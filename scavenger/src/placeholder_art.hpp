#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

// Flat-shaded placeholder shapes generated in code, until real art is chosen.
namespace scavenger::placeholder {

struct Rgba {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;
};

struct Pixels {
    int width = 0;
    int height = 0;
    std::vector<std::byte> rgba;
};

// 2:1 diamond, e.g. 32x16.
[[nodiscard]] Pixels diamond(int width, Rgba fill, Rgba border);
// Full-story isometric block: top diamond plus left and right faces.
[[nodiscard]] Pixels block(int width, int height, Rgba top, Rgba left, Rgba right, Rgba border);
// Rounded upright capsule, for characters.
[[nodiscard]] Pixels capsule(int width, int height, Rgba fill, Rgba border);
// Solid box with a border, for items.
[[nodiscard]] Pixels box(int width, int height, Rgba fill, Rgba border);

} // namespace scavenger::placeholder
