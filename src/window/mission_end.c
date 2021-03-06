#include "mission_end.h"

#include "city/emperor.h"
#include "city/finance.h"
#include "city/population.h"
#include "city/ratings.h"
#include "city/victory.h"
#include "game/mission.h"
#include "game/settings.h"
#include "game/state.h"
#include "game/undo.h"
#include "graphics/generic_button.h"
#include "graphics/graphics.h"
#include "graphics/lang_text.h"
#include "graphics/panel.h"
#include "graphics/text.h"
#include "graphics/window.h"
#include "scenario/property.h"
#include "scenario/scenario.h"
#include "sound/music.h"
#include "sound/speech.h"
#include "window/intermezzo.h"
#include "window/main_menu.h"
#include "window/mission_selection.h"
#include "window/victory_video.h"

static void button_fired(int param1, int param2);

static generic_button fired_buttons[] = {
    {64, 208, 384, 228, GB_IMMEDIATE, button_fired, button_none, 0, 0},
};

static int focus_button_id;

static void draw_lost(void)
{
    outer_panel_draw(48, 16, 34, 16);
    lang_text_draw_centered(62, 1, 48, 32, 544, FONT_LARGE_BLACK);
    lang_text_draw_multiline(62, 16, 64, 72, 496, FONT_NORMAL_BLACK);
}

static void draw_won(void)
{
    outer_panel_draw(48, 128, 34, 18);
    lang_text_draw_centered(62, 0, 48, 144, 544, FONT_LARGE_BLACK);
   
    inner_panel_draw(64, 184, 32, 7);

    if (scenario_is_custom()) {
        lang_text_draw_multiline(147, 20, 80, 192, 496, FONT_NORMAL_WHITE);
    } else {
        lang_text_draw_multiline(147, scenario_campaign_mission(), 80, 192, 496, FONT_NORMAL_WHITE);
    }
    int width = lang_text_draw(148, 0, 88, 308, FONT_NORMAL_BLACK);
    text_draw_number(city_rating_culture(), '@', " ", 88 + width, 308, FONT_NORMAL_BLACK);

    width = lang_text_draw(148, 1, 348, 308, FONT_NORMAL_BLACK);
    text_draw_number(city_rating_prosperity(), '@', " ", 348 + width, 308, FONT_NORMAL_BLACK);

    width = lang_text_draw(148, 2, 88, 328, FONT_NORMAL_BLACK);
    text_draw_number(city_rating_peace(), '@', " ", 88 + width, 328, FONT_NORMAL_BLACK);

    width = lang_text_draw(148, 3, 348, 328, FONT_NORMAL_BLACK);
    text_draw_number(city_rating_favor(), '@', " ", 348 + width, 328, FONT_NORMAL_BLACK);

    width = lang_text_draw(148, 4, 88, 348, FONT_NORMAL_BLACK);
    text_draw_number(city_population(), '@', " ", 88 + width, 348, FONT_NORMAL_BLACK);

    width = lang_text_draw(148, 5, 348, 348, FONT_NORMAL_BLACK);
    text_draw_number(city_finance_treasury(), '@', " ", 348 + width, 348, FONT_NORMAL_BLACK);

    lang_text_draw_centered(13, 1, 64, 388, 512, FONT_NORMAL_BLACK);
}

static void draw_background(void)
{
    graphics_in_dialog();
    if (city_victory_state() == VICTORY_STATE_WON) {
        draw_won();
    } else {
        draw_lost();
    }
    graphics_reset_dialog();
}

static void draw_foreground(void)
{
    if (city_victory_state() != VICTORY_STATE_WON) {
        graphics_in_dialog();
        large_label_draw(80, 224, 30, focus_button_id == 1);
        lang_text_draw_centered(62, 6, 80, 230, 480, FONT_NORMAL_GREEN);
        graphics_reset_dialog();
    }
}

static void advance_to_next_mission(void)
{
    setting_set_personal_savings_for_mission(scenario_campaign_rank() + 1, city_emperor_personal_savings());
    scenario_set_campaign_rank(scenario_campaign_rank() + 1);

    city_victory_stop_governing();

    game_undo_disable();
    game_state_reset_overlay();

    if (scenario_campaign_rank() >= 11 || scenario_is_custom()) {
        window_main_menu_show();
        if (!scenario_is_custom()) {
            setting_clear_personal_savings();
            scenario_settings_init();
            scenario_set_campaign_rank(2);
        }
    } else {
        scenario_set_campaign_mission(game_mission_peaceful());
        window_mission_selection_show();
    }
}

static void handle_mouse(const mouse *m)
{
    if (city_victory_state() == VICTORY_STATE_WON) {
        if (m->right.went_up) {
            sound_music_stop();
            sound_speech_stop();
            advance_to_next_mission();
        }
    } else {
        generic_buttons_handle_mouse(mouse_in_dialog(m), 48, 16,
                                     fired_buttons, 1, &focus_button_id);
    }
}

static void button_fired(int param1, int param2)
{
    city_victory_stop_governing();
    game_undo_disable();
    if (scenario_is_custom()) {
        window_main_menu_show();
    } else {
        window_mission_selection_show();
    }
}

static void show_end_dialog(void)
{
    window_type window = {
        WINDOW_MISSION_END,
        draw_background,
        draw_foreground,
        handle_mouse,
        0
    };
    window_show(&window);
}

static void show_intermezzo(void)
{
    sound_music_reset();
    window_intermezzo_show(INTERMEZZO_WON, show_end_dialog);
}

void window_mission_end_show_won(void)
{
    mouse_reset_up_state();
    if (scenario_is_tutorial_1() || scenario_is_tutorial_2()) {
        // tutorials: immediately go to next mission
        show_intermezzo();
    } else if (!scenario_is_custom() && scenario_campaign_rank() >= 10) {
        // Won campaign
        window_victory_video_show("smk/win_game.smk", 400, 292, show_intermezzo);
    } else {
        if (setting_victory_video()) {
            window_victory_video_show("smk/victory_balcony.smk", 400, 292, show_intermezzo);
        } else {
            window_victory_video_show("smk/victory_senate.smk", 400, 292, show_intermezzo);
        }
    }
}

void window_mission_end_show_fired(void)
{
    window_intermezzo_show(INTERMEZZO_FIRED, show_end_dialog);
}
