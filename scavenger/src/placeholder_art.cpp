#include "placeholder_art.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <optional>

namespace scavenger::placeholder {

namespace {

class Canvas {
public:
    Canvas(int width, int height)
        : width_{width}, height_{height},
          cells_(static_cast<std::size_t>(width * height)) {}

    void set(int x, int y, Rgba color) { cells_[index(x, y)] = color; }
    [[nodiscard]] bool filled(int x, int y) const {
        return x >= 0 && y >= 0 && x < width_ && y < height_ && cells_[index(x, y)].has_value();
    }

    // Recolors filled pixels that touch transparency or the image edge.
    void outline(Rgba border) {
        std::vector<std::size_t> edge;
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                if (filled(x, y) && (!filled(x - 1, y) || !filled(x + 1, y) ||
                                     !filled(x, y - 1) || !filled(x, y + 1))) {
                    edge.push_back(index(x, y));
                }
            }
        }
        for (const std::size_t i : edge) {
            cells_[i] = border;
        }
    }

    [[nodiscard]] Pixels to_pixels() const {
        Pixels pixels{width_, height_, std::vector<std::byte>(cells_.size() * 4)};
        for (std::size_t i = 0; i < cells_.size(); ++i) {
            const Rgba color = cells_[i].value_or(Rgba{0, 0, 0, 0});
            pixels.rgba[i * 4 + 0] = std::byte{color.r};
            pixels.rgba[i * 4 + 1] = std::byte{color.g};
            pixels.rgba[i * 4 + 2] = std::byte{color.b};
            pixels.rgba[i * 4 + 3] = std::byte{color.a};
        }
        return pixels;
    }

private:
    [[nodiscard]] std::size_t index(int x, int y) const {
        return static_cast<std::size_t>(y * width_ + x);
    }

    int width_;
    int height_;
    std::vector<std::optional<Rgba>> cells_;
};

// Normalized distance from the diamond center: <= 1 means inside.
float diamond_distance(int x, int y, float center_x, float center_y, float half_w, float half_h) {
    return std::abs(static_cast<float>(x) + 0.5F - center_x) / half_w +
           std::abs(static_cast<float>(y) + 0.5F - center_y) / half_h;
}

} // namespace

Pixels diamond(int width, Rgba fill, Rgba border) {
    const int height = width / 2;
    Canvas canvas{width, height};
    const float half_w = static_cast<float>(width) / 2.0F;
    const float half_h = static_cast<float>(height) / 2.0F;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (diamond_distance(x, y, half_w, half_h, half_w, half_h) <= 1.0F) {
                canvas.set(x, y, fill);
            }
        }
    }
    canvas.outline(border);
    return canvas.to_pixels();
}

Pixels block(int width, int height, Rgba top, Rgba left, Rgba right, Rgba border) {
    Canvas canvas{width, height};
    const float half_w = static_cast<float>(width) / 2.0F;
    const float half_h = static_cast<float>(width) / 4.0F; // 2:1 diamond
    const float top_center = half_h;
    const float base_center = static_cast<float>(height) - half_h;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const float dx = std::abs(static_cast<float>(x) + 0.5F - half_w) / half_w;
            const float reach = half_h * (1.0F - dx);
            const float py = static_cast<float>(y) + 0.5F;
            if (diamond_distance(x, y, half_w, top_center, half_w, half_h) <= 1.0F) {
                canvas.set(x, y, top);
            } else if (py >= top_center - reach && py <= base_center + reach) {
                canvas.set(x, y, static_cast<float>(x) + 0.5F < half_w ? left : right);
            }
        }
    }
    canvas.outline(border);
    return canvas.to_pixels();
}

Pixels capsule(int width, int height, Rgba fill, Rgba border) {
    Canvas canvas{width, height};
    const float radius = static_cast<float>(width) / 2.0F;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const float px = static_cast<float>(x) + 0.5F - radius;
            const float py = static_cast<float>(y) + 0.5F;
            const float cap_y = std::clamp(py, radius, static_cast<float>(height) - radius);
            const float dy = py - cap_y;
            if (px * px + dy * dy <= radius * radius) {
                canvas.set(x, y, fill);
            }
        }
    }
    canvas.outline(border);
    return canvas.to_pixels();
}

Pixels box(int width, int height, Rgba fill, Rgba border) {
    Canvas canvas{width, height};
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            canvas.set(x, y, fill);
        }
    }
    canvas.outline(border);
    return canvas.to_pixels();
}

} // namespace scavenger::placeholder
