/* Morse code decoder automaton
 * This is free and unencumbered software released into the public domain.
 */

/* Advance to the next state for an input, '.', '-'. or 0 (terminal).
 * The initial state is zero. Returns the next state, or the result:
 *   < 0 when more input is needed (i.e. next state)
 *   = 0 for invalid input
 *   > 0 the ASCII result
 */
#define ERR 0x1

#if 1
int morse_decode(int state, int c) {
  static const unsigned char table[] = {
      0x01, 0x45, 0x54, 0x49, 0x41, 0x4e, 0x4d, 0x53, 0x55, 0x52, 0x57, 0x44,
      0x4b, 0x47, 0x4f, 0x48, 0x56, 0x46, 0x01, 0x4c, 0x01, 0x50, 0x4a, 0x42,
      0x58, 0x43, 0x59, 0x5a, 0x51, 0x01, 0x01, 0x35, 0x34, 0x01, 0x33, 0x01,
      0x01, 0x01, 0x32, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x31, 0x36,
      0x3d, 0x2f, 0x01, 0x01, 0x01, 0x28, 0x01, 0x37, 0x01, 0x01, 0x01, 0x38,
      0x01, 0x39, 0x30, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
      0x01, 0x01, 0x01, 0x3f, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
      0x2e, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x27, 0x01, 0x01,
      0x2d, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
      0x29, 0x01, 0x01, 0x01, 0x01, 0x01, 0x2c, 0x01, 0x01, 0x01, 0x01, 0x3a,
  };
  if (state > 0 || -state >= (sizeof(table) / sizeof(*table))) {
    return ERR;
  }
  int value = table[-state];
  switch (c) {
  case '\0':
  case ' ':
    return value;
  case '.':
    return state * 2 - 1;
  case '-':
    return state * 2 - 2;
  default:
    return ERR;
  }
}
#else

int morse_decode(int state, int c) {
  static const unsigned char table[] = {
      0x03, 0x5b, 0x97, 0x6b, 0x4b, 0x7f, 0x7b, 0x93, 0x9b, 0x8d, 0xa3,
      0x57, 0x73, 0x63, 0x83, 0x67, 0x9e, 0x5c, 0x02, 0x74, 0x00, 0x84,
      0x6e, 0x4d, 0xa4, 0x50, 0xa8, 0xad, 0x88, 0x01, 0x03, 0x18, 0x14,
      0x00, 0x10, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x08, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x20, 0x00, 0x00, 0x00, 0x24, 0x00, 0x28, 0x04,
  };
  if (state > 0 || -state >= (sizeof(table) / sizeof(*table))) {
    return ERR;
  }
  int value = table[-state];
  switch (c) {
  case '\0':
    return value >= 0x4 ? (value >> 2) + '/' : ERR;
  case '.':
    return state * 2 - 1;
    // return value & 0x1 ? state * 2 - 1 : ERR;
  case '-':
    return state * 2 - 2;
    // return value & 0x2 ? state * 2 - 2 : ERR;
  default:
    return ERR;
  }
}

int morse_decode(int state, int c) {
  static const unsigned char table[] = {
      0xac, 0xca, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x80, 0x86, 0x92, 0x80, 0x94, 0x82, 0x80, 0x80, 0x80, 0x80, 0x80,
      0x80, 0x80, 0x80, 0x46, 0x50, 0x0e, 0x00, 0x00, 0x00, 0x26, 0x52,
      0x34, 0x24, 0x00, 0x00, 0x56, 0x44, 0x0c, 0x0a, 0x48, 0x4c, 0x00,
      0x04, 0x28, 0x54, 0x00, 0x00, 0x30, 0x40, 0x2a, 0x38, 0x18, 0x1a,
      0x00, 0x00, 0x00, 0x00, 0x3a, 0x00, 0x32, 0x4e, 0x3e, 0x3c, 0x2e,
      0x16, 0x00, 0x08, 0x42, 0x36, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
  };
  int index = (-state) & 0x7f;
  switch (c) {
  case '\0':
    return (table[index] & 0x80) == 0 ? index / 2 + '/' : 0;
  case '.':
    return -table[index];
  case '-':
    return -table[index + 1];
  default:
    return 0;
  }
}
int morse_decode(int state, int c) {
  static const unsigned char t[] = {
      0x03, 0x3f, 0x7b, 0x4f, 0x2f, 0x63, 0x5f, 0x77, 0x7f, 0x72, 0x87, 0x3b,
      0x57, 0x47, 0x67, 0x4b, 0x81, 0x40, 0x01, 0x58, 0x00, 0x68, 0x51, 0x32,
      0x88, 0x34, 0x8c, 0x92, 0x6c, 0x02, 0x03, 0x18, 0x14, 0x00, 0x10, 0x00,
      0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x1c,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x24,
      0x00, 0x28, 0x04, 0x00, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
      0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,
      0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56,
      0x57, 0x58, 0x59, 0x5a};
  int v = t[-state];
  switch (c) {
  case 0x00:
    return v >> 2 ? t[(v >> 2) + 63] : 0;
  case 0x2e:
    return v & 2 ? state * 2 - 1 : 0;
  case 0x2d:
    return v & 1 ? state * 2 - 2 : 0;
  default:
    return 0;
  }
}
#endif

#ifdef TEST
#include <stdio.h>
#include <string.h>

#define CR(s) "\x1b[91;1m" s "\x1b[0m"
#define CG(s) "\x1b[92;1m" s "\x1b[0m"

int main(int argc, char *argv[]) {
#ifdef _WIN32
  /* Best effort enable ANSI escape processing. */
  void *GetStdHandle(unsigned);
  int GetConsoleMode(void *, unsigned *);
  int SetConsoleMode(void *, unsigned);
  void *handle;
  unsigned mode;
  handle = GetStdHandle(-11); /* STD_OUTPUT_HANDLE */
  if (GetConsoleMode(handle, &mode)) {
    mode |= 0x0004;               /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */
    SetConsoleMode(handle, mode); /* ignore errors */
  }
#endif

  static const struct {
    char input[6];
    short expect;
  } tests[] = {
      {"", ERR},      {".", 'E'},     {"-", 'T'},     {"..", 'I'},
      {".-", 'A'},    {"-.", 'N'},    {"--", 'M'},    {"...", 'S'},
      {"..-", 'U'},   {".-.", 'R'},   {".--", 'W'},   {"-..", 'D'},
      {"-.-", 'K'},   {"--.", 'G'},   {"---", 'O'},   {"....", 'H'},
      {"...-", 'V'},  {"..-.", 'F'},  {"..--", ERR},  {".-..", 'L'},
      {".-.-", ERR},  {".--.", 'P'},  {".---", 'J'},  {"-...", 'B'},
      {"-..-", 'X'},  {"-.-.", 'C'},  {"-.--", 'Y'},  {"--..", 'Z'},
      {"--.-", 'Q'},  {"---.", ERR},  {"----", ERR},  {".....", '5'},
      {"....-", '4'}, {"...-.", ERR}, {"...--", '3'}, {"..-..", ERR},
      {"..-.-", ERR}, {"..--.", ERR}, {"..---", '2'}, {".-...", ERR},
      {".-..-", ERR}, {".-.-.", ERR}, {".-.--", ERR}, {".--..", ERR},
      {".--.-", ERR}, {".---.", ERR}, {".----", '1'}, {"-....", '6'},
      {"-...-", '='}, {"-..-.", '/'}, {"-..--", ERR}, {"-.-..", ERR},
      {"-.-.-", ERR}, {"-.--.", '('}, {"-.---", ERR}, {"--...", '7'},
      {"--..-", ERR}, {"--.-.", ERR}, {"--.--", ERR}, {"---..", '8'},
      {"---.-", ERR}, {"----.", '9'}, {"-----", '0'},
  };

  int fails = 0;
  int n, ntests = sizeof(tests) / sizeof(*tests);

  for (n = 0; n < ntests; n++) {
    const char *s = tests[n].input;
    int expect = tests[n].expect;
    int pass = 1;
    int state = 0;

    while (*s) {
      state = morse_decode(state, *s++);

      if (state == ERR) {
        if (expect != ERR) {
          printf(CR("FAIL") ": %s, want %c, got early error\n", tests[n].input,
                 expect);
          pass = 0;
        }
        break;
      }

      if (state > 1) {
        printf(CR("FAIL") ": %s, want %c, got early 0x%02x\n", tests[n].input,
               expect, state);
        pass = 0;
        break;
      }
    }

    if (state < 0) {
      state = morse_decode(state, 0);

      if (state == ERR) {
        if (expect != ERR) {
          printf(CR("FAIL") ": %s, want %c, got error\n", tests[n].input,
                 expect);
          pass = 0;
        }
      } else if (state < 0) {
        printf(CR("FAIL") ": %s, want %c, got continuation\n", tests[n].input,
               expect);
        pass = 0;
      } else if (state != expect) {
        if (expect != ERR) {
          printf(CR("FAIL") ": %s, want %c, got 0x%02x\n", tests[n].input,
                 expect, state);
        } else {
          printf(CR("FAIL") ": %s, want error, got 0x%02x (%c)\n",
                 tests[n].input, state, state);
        }
        pass = 0;
      }
    }

    if (pass) {
      if (expect) {
        printf(CG("PASS") ": %c %s\n", expect, tests[n].input);
      } else {
        printf(CG("PASS") ": ? %s\n", tests[n].input);
      }
    }

    fails += !pass;
  }

  // Test rejecting bad input "......"
  ntests++;
  int state = 0;
  for (int count = 0; count < 6; ++count) {
    state = morse_decode(state, '.');
  }
  state = morse_decode(state, 0);
  if (state == ERR) {
    printf(CG("PASS") ": ......\n");
  } else {
    printf(CR("FAIL") ": ......, got %d, want 0\n", state);
    fails++;
  }

  if (!fails) {
    printf("All %d tests pass\n", ntests);
  }

  return !!fails;
}
#endif
