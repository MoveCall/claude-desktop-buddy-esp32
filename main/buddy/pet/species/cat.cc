#include "species_data.h"

namespace {
static const char* const CAT_SLEEP_FRAMES[] = {
    "\n\n   .-..-.\n  ( -.- )\n  `------`~",
    "\n\n   .-..-.\n  ( -.- )_\n `~------'~",
    "\n\n   .-..-.\n  ( u.u )\n `~------'~",
    "\n\n   .-/\\.\n  (  ..  ))\n  `~~~~~~`",
    "\n\n   .-/\\.\n  (  ..  ))\n  `~~~~~~`~",
};
static const uint8_t CAT_SLEEP_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 2, 0, 1, 3, 4, 3, 4, 3, 4, 0, 0, 0, 1, 0, 1, 4, 4, 3, 3 };

static const char* const CAT_IDLE_FRAMES[] = {
    "\n   /\\_/\\\n  ( o   o )\n  (  w   )\n  (\")_(\")",
    "\n   /\\_/\\\n  (o    o )\n  (  w   )\n  (\")_(\")",
    "\n   /\\_/\\\n  ( o    o)\n  (  w   )\n  (\")_(\")",
    "\n   /\\_/\\\n  ( -   - )\n  (  w   )\n  (\")_(\")",
    "\n   /\\-/\\\n  ( _   _ )\n  (  w   )\n  (\")_(\")",
    "\n   <\\_/\\\n  ( o   o )\n  (  w   )\n  (\")_(\")",
    "\n   /\\_/>\n  ( o   o )\n  (  w   )\n  (\")_(\")",
    "\n   /\\_/\\\n  ( o   o )\n  (  w   )\n  (\")_(\")~",
    "\n   /\\_/\\\n  ( o   o )\n  (  w   )\n ~(\")_(\")",
    "\n   /\\_/\\\n  ( ^   ^ )\n  (  P   )\n  (\")_(\")",
};
static const uint8_t CAT_IDLE_SEQ[] = { 0, 0, 0, 3, 0, 1, 0, 2, 0, 7, 8, 7, 8, 7, 0, 5, 0, 6, 0, 4, 4, 0, 9, 9, 9, 0, 0, 3, 0, 8, 7, 8, 7, 0, 0, 4, 0 };

static const char* const CAT_BUSY_FRAMES[] = {
    "      .\n   /\\_/\\\n  ( o   o )\n  (  w   )/\n  (\")_(\")",
    "    .\n   /\\_/\\\n  ( o   o )\n  (  w   )_\n  (\")_(\")",
    "\n   /\\_/\\\n  ( O   O )\n  (  w   )\n  (\")_(\")",
    "    o\n   /\\_/\\\n  ( o   o )\n  ( -w   )\n  (\")_(\")",
    "  o\n   /\\_/\\\n  ( o   o )\n  (-w    )\n  (\")_(\")",
    "\n   /\\_/\\\n  ( -   - )\n  (  w   )\n  (\")_(\")",
};
static const uint8_t CAT_BUSY_SEQ[] = { 2, 2, 2, 0, 1, 0, 1, 3, 4, 3, 4, 5, 5, 2, 2, 0, 1, 0, 1, 5, 2 };

static const char* const CAT_ATTENTION_FRAMES[] = {
    "\n   /^_^\\\n  ( O   O )\n  (  v   )\n  (\")_(\")",
    "\n   /^_^\\\n  (O    O )\n  (  v   )\n  (\")_(\")",
    "\n   /^_^\\\n  ( O    O)\n  (  v   )\n  (\")_(\")",
    "\n   /^_^\\\n  ( ^   ^ )\n  (  v   )\n  (\")_(\")",
    "\n   /^_^\\\n /( O   O )\\\n (   v    )\n /(\")_(\")\\",
    "\n   /^_^\\\n  ( O   O )\n  (  >   )\n  (\")_(\")",
};
static const uint8_t CAT_ATTENTION_SEQ[] = { 0, 4, 0, 1, 0, 2, 0, 3, 4, 4, 0, 1, 2, 0, 5, 0 };

static const char* const CAT_CELEBRATE_FRAMES[] = {
    "\n   /\\_/\\\n  ( ^   ^ )\n  (  W   )\n /(\")_(\")\\",
    "  \\^   ^/\n    /\\_/\\\n  ( ^   ^ )\n  (  W   )\n  (\")_(\")",
    "  \\^   ^/\n    /\\_/\\\n  ( * * * )\n  (  W   )\n  (\")_(\")~",
    "\n   /\\_/\\\n  ( <   < )\n  (  W   ) /\n ~(\")_(\")",
    "\n   /\\_/\\\n  ( >   > )\n \\(  W   )\n  (\")_(\")~",
    "    \\o/\n   /\\_/\\\n  ( ^   ^ )\n /(  W   )\\\n  (\")_(\")",
};
static const uint8_t CAT_CELEBRATE_SEQ[] = { 0, 1, 2, 1, 0, 3, 4, 3, 4, 0, 1, 2, 1, 0, 5, 5 };

static const char* const CAT_DIZZY_FRAMES[] = {
    "\n  /\\_/\\\n ( @   @ )\n (   ~~  )\n (\")_(\")",
    "\n    /\\_/\\\n  ( @   @ )\n  (  ~~  )\n    (\")_(\")",
    "\n   /\\_/\\\n  ( x   @ )\n  (  v   )\n  (\")_(\")~",
    "\n   /\\_/\\\n  ( @   x )\n  (  v   )\n ~(\")_(\")",
    "\n   /\\_/\\\n  ( @   @ )\n  (  -   )\n /(\")_(\")\\~",
};
static const uint8_t CAT_DIZZY_SEQ[] = { 0, 1, 0, 1, 2, 3, 0, 1, 0, 1, 4, 4, 2, 3 };

static const char* const CAT_HEART_FRAMES[] = {
    "\n   /\\_/\\\n  ( ^   ^ )\n  (  u   )\n  (\")_(\")~",
    "\n   /\\_/\\\n  (#^   ^#)\n  (  u   )\n  (\")_(\")",
    "\n   /\\_/\\\n  ( <3 <3 )\n  (  u   )\n  (\")_(\")~",
    "\n   /\\-/\\\n  ( ~   ~ )\n  (  u   )\n ~(\")_(\")~",
    "\n   /\\_/\\\n  ( ^   - )\n  (  u   )\n  (\")_(\")",
};
static const uint8_t CAT_HEART_SEQ[] = { 0, 0, 1, 0, 2, 2, 0, 1, 0, 4, 0, 0, 3, 3, 0, 1, 0, 2, 1, 0 };

} // namespace

extern const SpeciesData kCatSpecies = {
    "Cat",
    {
        BUDDY_ANIM_SEQ(CAT_SLEEP_FRAMES, CAT_SLEEP_SEQ, 5),
        BUDDY_ANIM_SEQ(CAT_IDLE_FRAMES, CAT_IDLE_SEQ, 5),
        BUDDY_ANIM_SEQ(CAT_BUSY_FRAMES, CAT_BUSY_SEQ, 5),
        BUDDY_ANIM_SEQ(CAT_ATTENTION_FRAMES, CAT_ATTENTION_SEQ, 5),
        BUDDY_ANIM_SEQ(CAT_CELEBRATE_FRAMES, CAT_CELEBRATE_SEQ, 3),
        BUDDY_ANIM_SEQ(CAT_DIZZY_FRAMES, CAT_DIZZY_SEQ, 4),
        BUDDY_ANIM_SEQ(CAT_HEART_FRAMES, CAT_HEART_SEQ, 5),
    },
};
