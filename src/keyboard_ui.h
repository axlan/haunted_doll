#pragma once

#include <stdint.h>
#include <stddef.h>

class EntryUI {
 public:
  void ChooseEntry(uint8_t idx);

  void IncrementChoice();

  void DecrementChoice();

  void Select();

  void SetExtraText(const char* buffer);

 private:
  static const size_t NUM_COLS = 70;
  static const size_t LINES_BETWEEN_ENTRIES = 40;
  uint8_t entry_active = 0;
  uint8_t selected_choice = 0;
  uint8_t num_choices = 0;
  uint8_t padding = 0;
  const char* extra_text = nullptr;

  void HighLight(uint8_t idx);
};
