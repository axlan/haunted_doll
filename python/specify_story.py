#!/usr/bin/env python

'''
Here's a gift that should be familiar from long ago. I'm not sure if "haunted doll" is the best gift to wish a happy start to your marriage, but when all you have is a hammer...

Due to this being a cursed artifact, it has some limitations in how it can communicate. While a little more advanced then spirit rapping, this doll can only communicate by presenting itself as a USB keyboard. This means you should be able to connect it to any computer, open a notepad-esque application, and invoke it by double pressing Num Lock twice. Keep in mind it can only "hear" the Num, Scroll, and Caps lock keys. If you interact with the document it's typing in the message will be messed up. The doll is also only the master of a very ancient USB protocol, and may not be able to make itself heard on newer hardware. Using the included cable should make this more likely to work, but an older Windows machine is probably more likely to work then a new Mac.
'''

LINE_LEN = 100
CENTER = False
LINE_OFFSET = 4

out_file = 'src/entries.h'

HEADER_TOP = '''
void Exit();

void NoOp(uint8_t from, uint8_t to) {}
'''

TEXT_BLOCKS = {
  'start': (
    ['Hello.'],
    ['one', 'two', 'three', 'exit'],
    None
  ),
  'one': (
    ["one.","test? I Don't know?"],
    ['start', 'two', 'three', 'exit'],
    None
  ),
  'two': (
    ['two.'],
    ['start', 'one', 'three', 'exit'],
    None
  ),
  'three': (
    ['three.'],
    ['start', 'one', 'two', 'exit'],
    None
  ),
  'exit': (
    ['three.'],
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
    fd.write(f'const char* const MENU_KEYS[] PROGMEM = {{"{key_strs}"}};\n')
    entries = []
    choices = []
    callbacks = []
    for key, entry in TEXT_BLOCKS.items():
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
      fd.write(f'const uint8_t CHOICES_{key.upper()}[] PROGMEM = {{{fixed_chioces}}};\n')
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
    fd.write(f'const char* const ENTRIES[] PROGMEM = {{{entries_str}}};\n')
    fd.write(f'const uint8_t* const CHOICES[] PROGMEM= {{{choices_str}}};\n')
    fd.write(f'const uint8_t CHOICE_COUNTS[] PROGMEM = {{{choice_counts_str}}};\n')
    fd.write(f'void (*ENTRY_CALLBACKS[]) (uint8_t from, uint8_t to) = {{{callback_str}}};\n')

def main():
  if not validate_entries():
    return
  # test('one')
  compile()


if __name__ == "__main__":
  main()
