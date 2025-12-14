#include "GameWidget.h"
#include <LibGfx/Painter.h>
#include <LibGUI/Painter.h>   // ВАЖНО: Добавлен этот include для GUI::Painter
#include <LibGfx/Font/Font.h>
#include <LibGUI/MessageBox.h>
#include <LibGUI/Application.h>
#include <AK/String.h>
#include <AK/StringView.h>
#include <LibGUI/Window.h>
#include <LibMain/Main.h>
#include <AK/Error.h>
#include <LibGUI/Icon.h>

namespace ProgressOSGame {

ErrorOr<NonnullRefPtr<GameWidget>> GameWidget::try_create()
{
    return adopt_nonnull_ref_or_enomem(new (std::nothrow) GameWidget());
}

GameWidget::GameWidget()
    : m_game_timer(Core::Timer::construct())
{
    set_override_cursor(Gfx::StandardCursor::Hidden);
    set_focus_policy(GUI::FocusPolicy::StrongFocus);

    m_game_timer->set_interval(20); // ~50 FPS
    m_game_timer->on_timeout = [this] { on_timer(); };
    m_game_timer->start();

    m_bar_x = 0.0f;
    m_target_bar_x = 0.0f;

    reset_game();
}

// ================= INPUT =================

void GameWidget::keydown_event(GUI::KeyEvent& event)
{
    if (event.key() == KeyCode::Key_R) {
        if (m_state == State::GameOver || m_state == State::Victory) {
            reset_game();
            event.accept();
            return;
        }
    }

    if (event.key() == KeyCode::Key_P) {
        m_state = (m_state == State::Playing) ? State::Paused : State::Playing;
        event.accept();
        update();
        return;
    }

    GUI::Widget::keydown_event(event);
}

void GameWidget::mousedown_event(GUI::MouseEvent& event)
{
    if (m_state != State::Playing)
        return;
    if (event.button() != GUI::MouseButton::Primary)
        return;

    Gfx::IntRect bar { (int)floor(m_bar_x), height() - 60, BAR_WIDTH, BAR_HEIGHT };
    if (bar.contains(event.position())) {
        m_dragging_bar = true;
        m_drag_offset_x = event.x() - m_bar_x;
        event.accept();
    }
}

void GameWidget::mouseup_event(GUI::MouseEvent& event)
{
    if (event.button() == GUI::MouseButton::Primary) {
        m_dragging_bar = false;
        event.accept();
    }
}

void GameWidget::mousemove_event(GUI::MouseEvent& event)
{
    if (m_state != State::Playing || !m_dragging_bar)
        return;

    m_target_bar_x = clamp(
        event.x() - m_drag_offset_x,
        0.0f,
        (float)max(0, width() - BAR_WIDTH)
    );
    event.accept();
}

// ================= GAME =================

void GameWidget::reset_game()
{
    m_state = State::Playing;
    m_captured_segments.clear();
    m_falling_segments.clear();
    m_ticks_until_spawn = 0;
    m_score = 0;
    m_level = 1;
    m_bar_speed = 0.0f;

    int w = width();
    m_bar_x = (w > 0) ? (w - BAR_WIDTH) * 0.5f : 0.0f;
    m_target_bar_x = m_bar_x;

    update();
}

int GameWidget::spawn_interval() const
{
    int base = BASE_SPAWN_TICKS - (m_level - 1) * 2 - (int)m_captured_segments.size();
    if (base < MIN_SPAWN_TICKS)
        base = MIN_SPAWN_TICKS;
    return base;
}

void GameWidget::increase_difficulty_if_needed()
{
    int new_level = 1 + m_score / 100;
    if (new_level > m_level)
        m_level = new_level;
}

void GameWidget::spawn_wave(int count)
{
    for (int i = 0; i < count; ++i)
        spawn_segment();
}

void GameWidget::spawn_segment()
{
    int w = width();
    int spawn_x_max = max(0, w - SEGMENT_SIZE);

    int roll = get_random_uniform(100);
    SegmentType type;
    if (roll < 50) type = SegmentType::Blue;
    else if (roll < 70) type = SegmentType::Grey;
    else if (roll < 82) type = SegmentType::Yellow;
    else if (roll < 94) type = SegmentType::Pink;
    else type = SegmentType::Red;

    Segment segment;
    segment.type = type;
    float x = (float)get_random_uniform(spawn_x_max + 1);
    segment.rect = { x, -(float)SEGMENT_SIZE, (float)SEGMENT_SIZE, (float)SEGMENT_SIZE };
    segment.speed = 1.5f + (m_level - 1) * 0.3f + (m_captured_segments.size() * 0.05f);

    m_falling_segments.append(segment);
}

void GameWidget::on_timer()
{
    if (m_state != State::Playing)
        return;

    float diff = m_target_bar_x - m_bar_x;
    m_bar_speed = m_bar_speed * 0.8f + diff * 0.15f;
    m_bar_x += m_bar_speed;

    clamp_bar_position();

    if (m_ticks_until_spawn-- <= 0) {
        if (get_random_uniform(100) < 15)
            spawn_wave(2 + get_random_uniform(3));
        else
            spawn_segment();
        m_ticks_until_spawn = spawn_interval();
    }

    for (auto& seg : m_falling_segments) {
        seg.rect.set_y(seg.rect.y() + seg.speed);
        if (seg.rect.y() > height())
            seg.active = false;
    }

    m_falling_segments.remove_all_matching([](auto& s) { return !s.active; });

    check_collisions();
    increase_difficulty_if_needed();
    update();
}

void GameWidget::clamp_bar_position()
{
    float minx = 0.0f;
    float maxx = (float)max(0, width() - BAR_WIDTH);

    if (m_bar_x < minx) { m_bar_x = minx; m_bar_speed = 0.0f; }
    if (m_bar_x > maxx) { m_bar_x = maxx; m_bar_speed = 0.0f; }
}

void GameWidget::check_collisions()
{
    Gfx::IntRect bar_rect { (int)floor(m_bar_x), height() - 60, BAR_WIDTH, BAR_HEIGHT };

    for (auto& seg : m_falling_segments) {
        if (!seg.active) continue;
        if (!seg.rect.to_type<int>().intersects(bar_rect)) continue;

        seg.active = false;

        switch (seg.type) {
        case SegmentType::Red:
            m_state = State::GameOver;
            GUI::MessageBox::show(window(), "CRITICAL ERROR: SYSTEM HALTED"sv, "BSOD"sv, GUI::MessageBox::Type::Error);
            return;
        case SegmentType::Pink:
            if (!m_captured_segments.is_empty())
                m_captured_segments.take_last();
            m_score = max(0, m_score - 5);
            break;
        case SegmentType::Blue:
            if (m_captured_segments.size() < MAX_SEGMENTS)
                m_captured_segments.append(SegmentType::Blue);
            m_score += 5;
            break;
        case SegmentType::Yellow:
            if (m_captured_segments.size() < MAX_SEGMENTS)
                m_captured_segments.append(SegmentType::Yellow);
            m_score += 10;
            break;
        case SegmentType::Grey:
            break;
        }

        if (m_captured_segments.size() >= MAX_SEGMENTS) {
            m_state = State::Victory;
            GUI::MessageBox::show(window(), "System Upgraded Successfully!"sv, "Congratulations"sv, GUI::MessageBox::Type::Information);
            return;
        }
    }
}

// ================= RENDER =================

void GameWidget::draw_classic_bevel(Gfx::Painter& painter, Gfx::IntRect rect, bool pressed)
{
    auto tl = pressed ? Gfx::Color::from_rgb(0x404040) : Gfx::Color::White;
    auto br = pressed ? Gfx::Color::White : Gfx::Color::from_rgb(0x404040);

    painter.fill_rect(rect, Gfx::Color::MidGray);
    painter.draw_line(rect.top_left(), rect.top_right(), tl);
    painter.draw_line(rect.top_left(), rect.bottom_left(), tl);
    painter.draw_line(rect.bottom_right(), rect.top_right(), br);
    painter.draw_line(rect.bottom_right(), rect.bottom_left(), br);
}

void GameWidget::paint_event(GUI::PaintEvent& event)
{
    GUI::Widget::paint_event(event);

    GUI::Painter painter(*this);
    painter.fill_rect(event.rect(), Gfx::Color::from_rgb(0x008080));

    for (auto& seg : m_falling_segments) {
        auto r = seg.rect.to_type<int>();
        draw_classic_bevel(painter, r);
        painter.fill_rect(r.shrunken(4, 4), Segment::color_for_type(seg.type));
    }

    Gfx::IntRect bar_bg { (int)floor(m_bar_x), height() - 60, BAR_WIDTH, BAR_HEIGHT };
    draw_classic_bevel(painter, bar_bg);

    auto inner = bar_bg.shrunken(6, 6);
    painter.fill_rect(inner, Gfx::Color::Black);

    int slot_w = inner.width() / MAX_SEGMENTS;
    int x = inner.x();
    for (auto type : m_captured_segments) {
        painter.fill_rect(Gfx::IntRect { x, inner.y(), max(1, slot_w - 1), inner.height() },
                          Segment::color_for_type(type));
        x += slot_w;
    }

    int percent = (m_captured_segments.size() * 100) / MAX_SEGMENTS;
    auto percent_text = MUST(String::formatted("{}%", percent));
    painter.draw_text(bar_bg.translated(1, 1), percent_text, Gfx::TextAlignment::Center, Gfx::Color::Black);
    painter.draw_text(bar_bg, percent_text, Gfx::TextAlignment::Center, Gfx::Color::White);

    auto score_text = MUST(String::formatted("Score: {}", m_score));
    painter.draw_text(Gfx::IntRect { 10, 8, 200, 18 }, score_text, Gfx::TextAlignment::TopLeft, Gfx::Color::White);

    auto level_text = MUST(String::formatted("Level: {}", m_level));
    painter.draw_text(Gfx::IntRect { width() - 120, 8, 110, 18 }, level_text, Gfx::TextAlignment::TopRight, Gfx::Color::White);

    if (m_state == State::Paused)
        painter.draw_text(Gfx::IntRect { 0, height()/2 - 10, width(), 20 }, "PAUSED"sv, Gfx::TextAlignment::Center, Gfx::Color::Yellow);
    else if (m_state == State::GameOver)
        painter.draw_text(Gfx::IntRect { 0, height()/2 - 10, width(), 20 }, "GAME OVER — R to restart"sv, Gfx::TextAlignment::Center, Gfx::Color::Red);
    else if (m_state == State::Victory)
        painter.draw_text(Gfx::IntRect { 0, height()/2 - 10, width(), 20 }, "VICTORY — R to restart"sv, Gfx::TextAlignment::Center, Gfx::Color::Green);
}

}