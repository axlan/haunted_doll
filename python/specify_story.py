#!/usr/bin/env python
import re

'''
Here's a gift that should be familiar from long ago. I'm not sure if "haunted doll" is the best gift to wish a happy start to your marriage, but when all you have is a hammer...

Due to this being a cursed artifact, it has some limitations in how it can communicate. While a little more advanced then spirit rapping, this doll can only communicate by presenting itself as a USB keyboard. This means you should be able to connect it to any computer, open a notepad-esque application, and invoke it by double pressing Caps Lock twice. Keep in mind it can only "hear" the Num, Scroll, and Caps lock keys. If you interact with the document it's typing in the message will be messed up. You also need to use an editor that doesn't do automatic indentation. The doll is also only the master of a very ancient USB protocol, and may not be able to make itself heard on newer hardware. Using the included cable should make this more likely to work, but an older Windows machine is probably more likely to work then a new Mac.
'''

LINE_LEN = 70
CENTER = False
LINE_OFFSET = 4

out_file = 'src/entries.h'

HEADER_TOP = '''
void Exit();

void NoOp(uint8_t from, uint8_t to) {}
'''

TEXT_BLOCKS = {
  'Start': (
    ["Hello Amelia and/or Paul! I'm the haunted doll Zozo!",
     "Tap the \\\"Caps Lock\\\" key to move the cursor.",
     "Double click it to select the response.",
     "Since this is for your wedding how about a relationship quiz?"],
    ['Zozo?', 'Are you stealing data?', 'Quiz', 'Exit'],
    None
  ),
  'Zozo?': (
    ["That's me!",
     "I usually talk over Ouija boards,",
     "but I'm trying to change with the times.",
     "At least I don't have to compete with the other",
     "haunted dolls on Ebay."],
    ['Are you stealing data?', 'Quiz', 'Exit'],
    None
  ),
  'Are you stealing data?': (
    ["Well, not sure why you'd trust me",
     "but I can't actually listen to your key presses.",
     "I can only listen for the keys that trigger lights.",
     "You know, like scroll, num, and caps lock.",
     "Who even has those these days?",
     "That's why I'm just listening for \\\"Caps Lock\\\"",
     "I use black magic for my identity theft."],
    ['Zozo?', 'Quiz', 'Exit'],
    None
  ),
  'Quiz': (
    ["While of course you two are perfect for each other,",
     "what if instead of being born as flesh and blood humans,",
     "you were instead a pair of possessed dolls?"],
    ['Start', 'Exit'],
    None
  ),
  'Exit': (
    ['Good Bye!', 'May your love be as eternal as I am.'],
    [],
    'Exit();'
  ),
}



def validate_entries():
  for key, entry in TEXT_BLOCKS.items():
    for line in entry[0]:
      if len(line) > LINE_LEN:
        print(f'Line {line} in {key} is {len(line) - LINE_LEN} too long')
        return False
    key_len = sum([len(choice) for choice in entry[1]]) + len(entry[1]) + 1
    if key_len > LINE_LEN:
      print(f'Choices in {key} are {key_len - LINE_LEN} too long')
      return False
    for choice in entry[1]:
      if choice not in TEXT_BLOCKS:
        print(f'Choice {choice} in {key} not valid')
        return False
  print('Entries valid')
  return True


def test(key):
  print('=' * LINE_LEN + '\n')
  entry = TEXT_BLOCKS[key]
  for line in entry[0]:
    if CENTER:
      line_offset = int((LINE_LEN - len(line)) / 2)
    else:
      line_offset = LINE_OFFSET
    print(' ' * line_offset, end='')
    print(line)
  print()
  key_total = sum([len(choice) for choice in entry[1]])
  padding = int((LINE_LEN - key_total) / (len(entry[1]) + 1))
  for choice in entry[1]:
    print(' ' * padding, end='')
    print(choice, end='')
  print('\n')
  print('='*(LINE_LEN + 2))

def compile():
  with open(out_file, 'w') as fd:
    fd.write(HEADER_TOP + '\n')
    key_strs = '", "'.join(TEXT_BLOCKS.keys())
    key_idx = {k: str(i) for i, k in enumerate(TEXT_BLOCKS.keys())}
    fd.write(f'const char* const MENU_KEYS[] = {{"{key_strs}"}};\n')
    entries = []
    choices = []
    callbacks = []
    for key, entry in TEXT_BLOCKS.items():
      key = re.sub(r"[^a-zA-Z\d]", "_", key)
      fixed_entry = []
      for line in entry[0]:
        if CENTER:
          line_offset = int((LINE_LEN - len(line)) / 2)
        else:
          line_offset = LINE_OFFSET
        fixed_entry.append(' ' * line_offset + line)
      fixed_entry = '\\n'.join(fixed_entry)
      fd.write(f'const char ENTRY_{key.upper()}[] PROGMEM = "{fixed_entry}";\n')
      fixed_chioces = ', '.join([key_idx[i] for i in entry[1]])
      fd.write(f'const uint8_t CHOICES_{key.upper()}[] = {{{fixed_chioces}}};\n')
      if entry[2]:
        func_name = 'Callback' + key.capitalize()
        fd.write(f'void {func_name}(uint8_t from, uint8_t to) {{\n{entry[2]}\n}}\n')
        callbacks.append(func_name)
      else:
        callbacks.append('NoOp')
      entries.append('ENTRY_' + key.upper())
      choices.append('CHOICES_' + key.upper())
    entries_str = ', '.join(entries)
    choices_str = ', '.join(choices)
    choice_counts_str = ', '.join([str(len(x[1])) for x in TEXT_BLOCKS.values()])
    callback_str = ', '.join(callbacks)
    fd.write(f'const char* const ENTRIES[] = {{{entries_str}}};\n')
    fd.write(f'const uint8_t* const CHOICES[] = {{{choices_str}}};\n')
    fd.write(f'const uint8_t CHOICE_COUNTS[] = {{{choice_counts_str}}};\n')
    fd.write(f'void (*ENTRY_CALLBACKS[]) (uint8_t from, uint8_t to) = {{{callback_str}}};\n')

def main():
  if not validate_entries():
    return
  # test('one')
  compile()


if __name__ == "__main__":
  main()
