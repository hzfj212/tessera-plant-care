#include "plant_care.h"
#include <algorithm>
#include <string>

namespace esphome::plant_care {

using tessera::Font;
using tessera::Tone;
namespace ui = tessera::ui;

// Days since 1970-01-01 for a date (Howard Hinnant's days_from_civil): the difference of two of these is whole days.
static long days_from_civil(int y, unsigned m, unsigned d) {
  y -= m <= 2;
  const long era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = static_cast<unsigned>(y - era * 400);
  const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + static_cast<long>(doe) - 719468;
}

// The day an entity holds, as days since 1970; false while it holds none (never set, nothing restored).
static bool entity_day(const datetime::DateEntity *d, long &out) {
  if (!d || !d->has_state()) return false;
  out = days_from_civil(d->year, d->month, d->day);
  return true;
}

// The tile: the plant's name, and two rows under it, one for watering and one for fertilizing. Each row has a dot in the
// colour of how overdue it is (green, amber, red) and a few words ("Watered: 3 d"). A tap on a row says "done today".
// create() makes the parts once; on_tick() sets their texts, and ui::set_text changes a label only when its text differs.
class PlantTile : public tessera::Tile {
 public:
  explicit PlantTile(PlantCare *plugin) : plugin_(plugin) {}

  void create(const tessera::TileContext &c) override {
    width_ = c.width;
    height_ = c.height;
    every_[0] = std::max(1, static_cast<int>(c.options["water_every"] | 7));
    every_[1] = std::max(1, static_cast<int>(c.options["fert_every"] | 30));
    pad_ = ui::px(ui::large() ? 8 : 5);
    gap_ = ui::px(ui::large() ? 6 : 4);
    dot_ = ui::px(ui::large() ? 9 : 7);
    space_ = ui::px(ui::large() ? 7 : 5);

    // The name on top when there is room for it, else the two rows fill the whole tile.
    const int name_h = ui::line_height(Font::BODY);
    top_ = name_h + gap_;
    row_h_ = (height_ - top_ - gap_) / 2;
    if (row_h_ < ui::line_height(Font::BODY) + ui::px(6)) {
      top_ = 0;
      row_h_ = (height_ - gap_) / 2;
    }

    std::string name = c.options["what"] | "";
    if (name.empty()) name = c.name;
    name_ = ui::label(c.parent, Font::BODY, theme::MUTED);
    lv_obj_set_width(name_, width_);
    lv_obj_set_style_text_align(name_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(name_, 0, 0);
    ui::set_text(name_, name);
    if (top_ == 0) lv_obj_add_flag(name_, LV_OBJ_FLAG_HIDDEN);

    for (int i = 0; i < 2; i++) {
      Row &r = rows_[i];
      r.bg = ui::block(c.parent, theme::RAISED);
      lv_obj_remove_flag(r.bg, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_remove_flag(r.bg, LV_OBJ_FLAG_SCROLLABLE);
      lv_obj_set_style_pad_all(r.bg, 0, 0);
      lv_obj_set_size(r.bg, width_, row_h_);
      lv_obj_set_pos(r.bg, 0, top_ + i * (row_h_ + gap_));

      r.dot = ui::block(r.bg, theme::MUTED);
      lv_obj_remove_flag(r.dot, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_set_size(r.dot, dot_, dot_);
      lv_obj_set_style_radius(r.dot, dot_ / 2, 0);
      lv_obj_set_pos(r.dot, pad_, (row_h_ - dot_) / 2);

      r.text = ui::label(r.bg, Font::BODY_LARGE, theme::INK);
      lv_obj_set_width(r.text, std::max(1, width_ - 2 * pad_ - dot_ - space_));
    }
  }

  void on_tick(uint32_t epoch) override {
    long today = 0;
    if (epoch) {
      const tessera::LocalTime now = tessera::local_time(epoch);
      today = days_from_civil(now.year, static_cast<unsigned>(now.month), static_cast<unsigned>(now.day));
    }
    update(0, plugin_->watered(), epoch != 0, today);
    update(1, plugin_->fertilized(), epoch != 0, today);
  }

  // Light and dark: the theme roles have other values now.
  void on_theme() override {
    ui::set_color(name_, theme::MUTED);
    for (int i = 0; i < 2; i++) {
      ui::set_fill(rows_[i].bg, Tone::NORMAL, theme::RAISED);
      ui::set_fill(rows_[i].dot, tone_[i], theme::MUTED);
      ui::set_color(rows_[i].text, theme::INK);
    }
  }

  // A tap on the upper row says "watered today", on the lower row "fertilized today". A tap on the name does nothing.
  void on_tap() override {
    const tessera::Point p = tessera::tap_point();
    if (p.y < top_) return;  // on the name, or the finger's place is not known (-1)
    const uint32_t epoch = tessera::epoch();
    if (!epoch) return;      // the screen's clock is not set yet: today is not known
    const int i = p.y < top_ + row_h_ + gap_ / 2 ? 0 : 1;
    datetime::DateEntity *d = i == 0 ? plugin_->watered() : plugin_->fertilized();
    if (!d) return;
    const tessera::LocalTime now = tessera::local_time(epoch);
    auto call = d->make_call();
    call.set_date(static_cast<uint16_t>(now.year), static_cast<uint8_t>(now.month), static_cast<uint8_t>(now.day));
    call.perform();
    on_tick(epoch);  // show it at once
  }

 private:
  struct Row {
    lv_obj_t *bg{}, *dot{}, *text{};
  };

  // One row: the words ("Watered: 3 d"), and a dot in the colour of how long ago it is against how often it is due.
  void update(int i, const datetime::DateEntity *date, bool clock_set, long today) {
    static const char *const NONE[2] = {"water_none", "fert_none"};
    static const char *const TODAY[2] = {"water_today", "fert_today"};
    static const char *const YESTERDAY[2] = {"water_yesterday", "fert_yesterday"};
    static const char *const DAYS[2] = {"water_days", "fert_days"};

    std::string words;
    Tone tone = Tone::NORMAL;
    long day = 0;
    if (!clock_set) {
      // the screen's clock is not set yet: say nothing
    } else if (!entity_day(date, day)) {
      words = plugin_->text(NONE[i]);
    } else {
      const long ago = std::max(0L, today - day);  // a date in the future counts as today
      if (ago == 0) {
        words = plugin_->text(TODAY[i]);
      } else if (ago == 1) {
        words = plugin_->text(YESTERDAY[i]);
      } else {
        words = tessera::format(plugin_->text(DAYS[i]), ago);
      }
      // Fine until it is due, amber for about half as long again, then red.
      tone = ago < every_[i] ? Tone::GOOD : ago * 2 < every_[i] * 3 ? Tone::BUSY : Tone::ALERT;
    }
    tone_[i] = tone;

    Row &r = rows_[i];
    const int room = std::max(1, width_ - 2 * pad_ - dot_ - space_);
    Font face = Font::BODY_LARGE;
    if (ui::text_width(words, face) > room || ui::line_height(face) > row_h_) face = Font::BODY;
    ui::set_font(r.text, face);
    ui::set_text(r.text, words);
    lv_obj_set_pos(r.text, pad_ + dot_ + space_, (row_h_ - ui::line_height(face)) / 2);
    ui::set_fill(r.dot, tone, theme::MUTED);
  }

  PlantCare *plugin_;
  int width_ = 0, height_ = 0;
  int pad_ = 0, gap_ = 0, dot_ = 0, space_ = 0;
  int top_ = 0, row_h_ = 0;
  int every_[2] = {7, 30};
  Tone tone_[2] = {Tone::NORMAL, Tone::NORMAL};
  lv_obj_t *name_{};
  Row rows_[2];
};

void PlantCare::setup() {
  // The id is the tile's id in the manifest; the core makes a PlantTile for every card that shows one.
  add_tile("plant_care", [this]() { return new PlantTile(this); });
}

}  // namespace esphome::plant_care
