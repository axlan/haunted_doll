#!/usr/bin/env python

'''
Here's a gift that should be familiar from long ago. I'm not sure if "haunted doll" is the best gift to wish a happy start to your marriage, but when all you have is a hammer...

Due to this being a cursed artifact, it has some limitations in how it can communicate. While a little more advanced then spirit rapping, this doll can only communicate by presenting itself as a USB keyboard. This means you should be able to connect it to any computer, open a notepad-esque application, and invoke it by double pressing Num Lock twice. Keep in mind it can only "hear" the Num, Scroll, and Caps lock keys. If you interact with the document it's typing in the message will be messed up. The doll is also only the master of a very ancient USB protocol, and may not be able to make itself heard on newer hardware. Using the included cable should make this more likely to work, but an older Windows machine is probably more likely to work then a new Mac.
'''

LINE_LEN = 100

out_file = 'src/entries.h'

TEXT_BLOCKS = {
  'start': (
    'Hello.',
    ['one', 'two', 'three']
  ),
  'one': (
    "one.\ntest? I Don't know?",
    ['start', 'two', 'three']
  ),
  'two': (
    'two.',
    ['start', 'one', 'three']
  ),
  'three': (
    'three.',
    ['start', 'one', 'two']
  ),

}



def validate_entries():
  for key, entry in TEXT_BLOCKS.items():
    for line in entry[0].split('\n'):
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
  for line in entry[0].split('\n'):
    spacing = int((LINE_LEN - len(line)) / 2)
    print(' ' * spacing, end='')
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
    key_strs = '", "'.join(TEXT_BLOCKS.keys())
    key_idx = {k: str(i) for i, k in enumerate(TEXT_BLOCKS.keys())}
    fd.write(f'const PROGMEM char MENU_KEYS[] = {{"{key_strs}"}};\n')
    entries = []
    choices = []
    for key, entry in TEXT_BLOCKS.items():
      fixed_entry = entry[0].replace('\n','\\n')
      fd.write(f'const PROGMEM char ENTRY_{key.upper()}[] = "{fixed_entry}";\n')
      fixed_chioces = ', '.join([str(len(entry[1]))] + [key_idx[i] for i in entry[1]])
      fd.write(f'const PROGMEM uint8_t CHOICES_{key.upper()}[] = {{{fixed_chioces}}};\n')
      entries.append('ENTRY_' + key.upper())
      choices.append('CHOICES_' + key.upper())
    entries_str = ', '.join(entries)
    choices_str = ', '.join(choices)
    fd.write(f'const PROGMEM char* ENTRIES[] = {{{entries_str}}};\n')
    fd.write(f'const PROGMEM uint8_t* CHOICES[] = {{{choices_str}}};\n')

def main():
  validate_entries()
  # test('one')
  compile()


if __name__ == "__main__":
  main()
