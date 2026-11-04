#include "placeholder_art.hpp"
#include "tiles.hpp"

#include <phantasm/application.hpp>
#include <phantasm/fixed_step.hpp>
#include <phantasm/iso/iso_grid.hpp>
#include <phantasm/iso/tile_map.hpp>
#include <phantasm/log.hpp>
#include <phantasm/paths.hpp>
#include <phantasm/physics/world.hpp>
#include <phantasm/sprite.hpp>
#include <phantasm/texture.hpp>

#include <algorithm>
#include <cmath>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using phantasm::Vec2;
using phantasm::iso::GridPos;
using phantasm::iso::TileCoord;
using phantasm::iso::TileMap;

namespace placeholder = scavenger::placeholder;
using placeholder::Rgba;

constexpr Rgba kWood{150, 110, 74};
constexpr Rgba kWoodEdge{110, 78, 52};
constexpr Rgba kGrass{86, 132, 70};
constexpr Rgba kGrassEdge{70, 110, 56};
constexpr Rgba kWallTop{200, 196, 186};
constexpr Rgba kWallLeft{150, 146, 138};
constexpr Rgba kWallRight{112, 108, 102};
constexpr Rgba kPlayer{70, 120, 200};
constexpr Rgba kSupply{220, 150, 40};
constexpr Rgba kEdge{40, 38, 44};

constexpr float kArtScale = 2.0F;
// Physics runs in grid space: one tile is one Box2D meter.
constexpr float kTilesPerMeter = 1.0F;
constexpr float kPlayerSpeed = 3.0F;   // tiles per second
constexpr float kPlayerRadius = 0.3F;  // tiles
constexpr double kPhysicsStep = 1.0 / 60.0;
constexpr float kZoomStep = 1.5F;
constexpr float kMinZoom = 0.5F;
constexpr float kMaxZoom = 4.0F;

// Pivots: where each image touches its tile. Floors are centered on the tile;
// the wall block's base diamond center sits 16 art px above its bottom edge.
constexpr Vec2 kFloorPivot{0.5F, 0.5F};
constexpr Vec2 kWallPivot{0.5F, 56.0F / 64.0F};
constexpr Vec2 kStandingPivot{0.5F, 0.95F};

std::string asset(const std::string& name) {
    return phantasm::asset_path("assets/scavenger/" + name);
}

Vec2 scaled_size(const phantasm::Texture& texture) {
    return Vec2{static_cast<float>(texture.width()) * kArtScale,
                static_cast<float>(texture.height()) * kArtScale};
}

GridPos tile_center(TileCoord tile) {
    return GridPos{static_cast<float>(tile.x) + 0.5F, static_cast<float>(tile.y) + 0.5F,
                   tile.level};
}

// Floors of a level draw first; walls, items, and characters then sort by depth.
int floor_layer(int level) {
    return level * 2;
}
int object_layer(int level) {
    return level * 2 + 1;
}

class Scavenger final : public phantasm::Application {
public:

    Scavenger()
        : Application{{.title = "Scavenger", .width = 1280, .height = 720}},
          map_{TileMap::parse(phantasm::read_text_file(asset("house.map")))},
          floor_wood_{make_texture(placeholder::diamond(32, kWood, kWoodEdge))},
          floor_grass_{make_texture(placeholder::diamond(32, kGrass, kGrassEdge))},
          wall_{make_texture(placeholder::block(32, 64, kWallTop, kWallLeft, kWallRight, kEdge))},
          player_{make_texture(placeholder::capsule(12, 22, kPlayer, kEdge))},
          supply_{make_texture(placeholder::box(12, 10, kSupply, kEdge))} {
        const auto starts = map_.find(scavenger::tiles::kPlayerStart);
        if (starts.size() != 1) {
            throw std::runtime_error("Map needs exactly one player start 'P'");
        }
        add_wall_bodies();
        player_position_ = tile_center(starts.front());
        player_body_ = physics_.create_body(
            phantasm::physics::BodyDef{.position = {player_position_.x, player_position_.y}});
        physics_.add_circle(player_body_, phantasm::physics::Circle{.radius = kPlayerRadius});
        renderer().set_camera({.position = grid_.grid_to_world(player_position_), .zoom = 1.0F});
        set_clear_color({0.08F, 0.09F, 0.12F, 1.0F});
        phantasm::log_info("Map {}x{}, {} level(s)", map_.width(), map_.height(),
                           map_.level_count());
    }

protected:
    void on_update(double delta_seconds) override {
        using phantasm::Key;
        if (input().was_pressed(Key::Escape)) {
            request_quit();
        }

        physics_.set_velocity(player_body_, player_velocity());
        for (int steps = stepper_.advance(delta_seconds); steps > 0; --steps) {
            physics_.step(static_cast<float>(stepper_.step()));
        }
        const Vec2 position = physics_.position(player_body_);
        player_position_ = GridPos{position.x, position.y, 0};

        phantasm::Camera camera = renderer().camera();
        camera.position = grid_.grid_to_world(player_position_);
        if (input().was_pressed(Key::E)) {
            camera.zoom = std::min(camera.zoom * kZoomStep, kMaxZoom);
        }
        if (input().was_pressed(Key::Q)) {
            camera.zoom = std::max(camera.zoom / kZoomStep, kMinZoom);
        }
        renderer().set_camera(camera);
    }

    void on_render() override {
        for (int level = 0; level < map_.level_count(); ++level) {
            for (int y = 0; y < map_.height(); ++y) {
                for (int x = 0; x < map_.width(); ++x) {
                    draw_tile(TileCoord{x, y, level});
                }
            }
        }
        draw_standing(player_, player_position_);
    }

private:
    phantasm::Texture make_texture(const placeholder::Pixels& pixels) {
        return renderer().create_texture(pixels.width, pixels.height, pixels.rgba);
    }

    // Screen-relative controls: W moves up the screen, which is diagonal on the grid.
    Vec2 player_velocity() const {
        using phantasm::Key;
        Vec2 screen{};
        if (input().is_down(Key::A) || input().is_down(Key::Left)) {
            screen.x -= 1.0F;
        }
        if (input().is_down(Key::D) || input().is_down(Key::Right)) {
            screen.x += 1.0F;
        }
        if (input().is_down(Key::W) || input().is_down(Key::Up)) {
            screen.y -= 1.0F;
        }
        if (input().is_down(Key::S) || input().is_down(Key::Down)) {
            screen.y += 1.0F;
        }
        const Vec2 grid = grid_.world_delta_to_grid(screen);
        const float length = std::sqrt(grid.x * grid.x + grid.y * grid.y);
        if (length == 0.0F) {
            return {};
        }
        return Vec2{grid.x / length * kPlayerSpeed, grid.y / length * kPlayerSpeed};
    }

    void add_wall_bodies() {
        for (const TileCoord wall : map_.find(scavenger::tiles::kWall)) {
            const GridPos center = tile_center(wall);
            const auto body = physics_.create_body(phantasm::physics::BodyDef{
                .type = phantasm::physics::BodyType::Static, .position = {center.x, center.y}});
            physics_.add_box(body, phantasm::physics::Box{.half_extents = {0.5F, 0.5F}});
        }
    }

    void draw_tile(TileCoord tile) {
        namespace tiles = scavenger::tiles;
        const char kind = map_.at(tile);
        const GridPos center = tile_center(tile);

        if (kind == tiles::kGrass || kind == tiles::kPlayerStart) {
            draw_floor(floor_grass_, center);
        } else if (tiles::is_indoor_floor(kind)) {
            draw_floor(floor_wood_, center);
        }

        if (kind == tiles::kWall) {
            draw_object(wall_, center, kWallPivot);
        } else if (kind == tiles::kSupply) {
            draw_standing(supply_, center);
        }
    }

    void draw_floor(const phantasm::Texture& texture, GridPos at) {
        renderer().draw_sprite(texture, {.position = grid_.grid_to_world(at),
                                         .size = scaled_size(texture),
                                         .pivot = kFloorPivot,
                                         .layer = floor_layer(at.level),
                                         .depth = phantasm::iso::draw_depth(at)});
    }

    void draw_object(const phantasm::Texture& texture, GridPos at, Vec2 pivot) {
        renderer().draw_sprite(texture, {.position = grid_.grid_to_world(at),
                                         .size = scaled_size(texture),
                                         .pivot = pivot,
                                         .layer = object_layer(at.level),
                                         .depth = phantasm::iso::draw_depth(at)});
    }

    void draw_standing(const phantasm::Texture& texture, GridPos at) {
        draw_object(texture, at, kStandingPivot);
    }

    phantasm::iso::IsoGrid grid_{};
    TileMap map_;
    phantasm::Texture floor_wood_;
    phantasm::Texture floor_grass_;
    phantasm::Texture wall_;
    phantasm::Texture player_;
    phantasm::Texture supply_;
    GridPos player_position_{};
    phantasm::physics::World physics_{phantasm::physics::WorldConfig{.units_per_meter = kTilesPerMeter}};
    phantasm::physics::BodyHandle player_body_{};
    phantasm::FixedStep stepper_{kPhysicsStep};
};

} // namespace

int main() {
    try {
        Scavenger game;
        game.run();
    } catch (const std::exception& error) {
        phantasm::log_error("Fatal: {}", error.what());
        return 1;
    }
    return 0;
}
