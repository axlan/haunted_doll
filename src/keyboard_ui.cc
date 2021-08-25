#include "keyboard_ui.h"

#include <string.h>

#include <DigiKeyboard.h>

#include "entries.h"

void EntryUI::ChooseEntry(uint8_t idx)
{
  entry_active = idx;
  selected_choice = 0;
  num_choices = CHOICE_COUNTS[idx];
  DigiKeyboardDevice::GetInstance().RepeatKeyStroke(KEY_EQUALS, NUM_COLS);
  DigiKeyboardDevice::GetInstance().RepeatKeyStroke(KEY_ENTER, 2);
  DigiKeyboardDevice::GetInstance().LightPrint(ENTRIES[idx], true);
  if (extra_text)
  {
    DigiKeyboardDevice::GetInstance().RepeatKeyStroke(KEY_ENTER, 2);
    DigiKeyboardDevice::GetInstance().LightPrint(extra_text);
    extra_text = nullptr;
  }
  DigiKeyboardDevice::GetInstance().RepeatKeyStroke(KEY_ENTER, 2);
  uint8_t choice_len = 0;
  for (int i = 0; i < num_choices; i++)
  {
    choice_len += strlen(MENU_LABELS[idx][i]);
  }
  padding = (NUM_COLS - choice_len) / (num_choices + 1);
  for (int i = 0; i < num_choices; i++)
  {
    DigiKeyboardDevice::GetInstance().RepeatKeyStroke(KEY_SPACE, padding);
    DigiKeyboardDevice::GetInstance().LightPrint(MENU_LABELS[idx][i]);
  }
  DigiKeyboardDevice::GetInstance().sendKeyStroke(KEY_ENTER);
  DigiKeyboardDevice::GetInstance().sendKeyStroke(KEY_ARROW_UP);
  HighLight(selected_choice);
}

void EntryUI::IncrementChoice()
{
  selected_choice = (selected_choice + 1) % num_choices;
  HighLight(selected_choice);
}
void EntryUI::DecrementChoice()
{
  selected_choice =
      (selected_choice == 0) ? num_choices - 1 : selected_choice - 1;
  HighLight(selected_choice);
}
void EntryUI::Select()
{
  DigiKeyboardDevice::GetInstance().RepeatKeyStroke(KEY_ARROW_DOWN, 3);
  DigiKeyboardDevice::GetInstance().RepeatKeyStroke(KEY_ENTER, LINES_BETWEEN_ENTRIES);
  uint8_t choice = CHOICES[entry_active][selected_choice];
  ENTRY_CALLBACKS[choice](entry_active, choice, selected_choice);
  ChooseEntry(choice);
}

void EntryUI::SetExtraText(const char *buffer)
{
  extra_text = buffer;
}

void EntryUI::HighLight(uint8_t idx)
{
  if (num_choices == 0)
  {
    return;
  }
  DigiKeyboardDevice::GetInstance().sendKeyStroke(KEY_ARROW_UP);
  DigiKeyboardDevice::GetInstance().sendKeyStroke(KEY_ARROW_RIGHT);
  int i;
  for (i = 0; i <= idx; i++)
  {
    DigiKeyboardDevice::GetInstance().RepeatKeyStroke(KEY_ARROW_RIGHT, padding);
    size_t label_len = strlen(MENU_LABELS[entry_active][i]);
    if (i == idx)
    {
      DigiKeyboardDevice::GetInstance().RepeatKeyStroke(KEY_ARROW_RIGHT, MOD_SHIFT_LEFT, label_len);
    }
    else
    {
      DigiKeyboardDevice::GetInstance().RepeatKeyStroke(KEY_ARROW_RIGHT, label_len);
    }
  }
}
