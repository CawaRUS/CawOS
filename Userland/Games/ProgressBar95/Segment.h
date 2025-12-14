#pragma once

#include <LibGfx/Rect.h>
#include <LibGfx/Color.h>

namespace ProgressOSGame {

enum class SegmentType {
    Blue,   // +5
    Yellow, // +10 (rare)
    Red,    // Game Over
    Grey,   // +0
    Pink    // -5
};

struct Segment {
    Gfx::FloatRect rect;
    SegmentType type;
    float speed { 0.0f };
    bool active { true };

    static Gfx::Color color_for_type(SegmentType type) {
        switch (type) {
        case SegmentType::Blue:   return Gfx::Color::from_rgb(0x0000AA);
        case SegmentType::Yellow: return Gfx::Color::from_rgb(0xFFAA00);
        case SegmentType::Red:    return Gfx::Color::from_rgb(0xAA0000);
        case SegmentType::Pink:   return Gfx::Color::from_rgb(0xFF55FF);
        case SegmentType::Grey:   return Gfx::Color::MidGray;
        default: return Gfx::Color::Black;
        }
    }
};

}
