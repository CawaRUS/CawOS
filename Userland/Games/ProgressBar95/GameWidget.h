#pragma once

#include <LibGUI/Widget.h>
#include <LibCore/Timer.h>
#include <LibGfx/Rect.h>
#include <AK/Vector.h>
#include <AK/Random.h>
#include "Segment.h"

namespace ProgressOSGame {

class GameWidget final : public GUI::Widget {
    C_OBJECT_ABSTRACT(GameWidget)

public:
    static ErrorOr<NonnullRefPtr<GameWidget>> try_create();

protected:
    virtual void paint_event(GUI::PaintEvent&) override;
    virtual void keydown_event(GUI::KeyEvent&) override;
    virtual void mousedown_event(GUI::MouseEvent&) override;
    virtual void mouseup_event(GUI::MouseEvent&) override;
    virtual void mousemove_event(GUI::MouseEvent&) override;

private:
    GameWidget();

    // ===== Drag progress bar =====
    bool m_dragging_bar { false };
    float m_drag_offset_x { 0.0f };

    // ===== Const =====
    static constexpr int MAX_SEGMENTS = 20;
    static constexpr int BAR_WIDTH = 300;
    static constexpr int BAR_HEIGHT = 40;
    static constexpr int BASE_SPAWN_TICKS = 40;
    static constexpr int MIN_SPAWN_TICKS = 10;
    static constexpr int SEGMENT_SIZE = 30;

    enum class State {
        Playing,
        Paused,
        GameOver,
        Victory
    };

    State m_state { State::Playing };

    Vector<SegmentType> m_captured_segments;
    Vector<Segment> m_falling_segments;

    float m_bar_x { 0.0f };
    float m_target_bar_x { 0.0f };
    float m_bar_speed { 0.0f };

    int m_ticks_until_spawn { 0 };
    NonnullRefPtr<Core::Timer> m_game_timer;

    int m_score { 0 };
    int m_level { 1 };

    // ===== Logic =====
    void on_timer();
    void spawn_segment();
    void spawn_wave(int count);
    void reset_game();
    void check_collisions();
    void clamp_bar_position();
    int spawn_interval() const;
    void increase_difficulty_if_needed();

    void draw_classic_bevel(Gfx::Painter&, Gfx::IntRect, bool pressed = false);
};

}
