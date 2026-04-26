#include "species_data.h"

namespace {
static const char* const GOOSE_SLEEP_FRAMES[] = {
    "\n    __\n   /  \\__\n  (  zz  )\n   `~~~~`",
    "\n    __\n   /  \\__\n  (  --  )\n   `~~~~`",
    "    o O .\n    __\n   /  \\__\n  (  --  )\n   `~~~~`",
    "\n    _o\n   /  \\__\n  (  --  )\n   `~~~~`",
    "\n      __\n   __/  \\\n  (  zz  )\n   `~~~~`",
    "    HnK..\n    __\n   /  \\__\n  (  zZ  )\n   `~~~~`",
};
static const uint8_t GOOSE_SLEEP_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 1, 0, 1, 0, 3, 4, 4, 4, 1, 0, 1, 2, 2, 0, 1, 5, 1 };

static const char* const GOOSE_IDLE_FRAMES[] = {
    "\n    (>\n    ||\n  _(__)_\n   ^^^^",
    "\n  <)\n   ||\n  _(__)_\n   ^^^^",
    "\n      (>\n      ||\n  _(__)_\n   ^^^^",
    "    /\\\n    (>\n    ||\n  _(__)_\n   ^^^^",
    "\n    (->\n    ||\n  _(__)_\n   ^^^^",
    "\n    (v\n    |\\__\n  _(__)_\n   ^^^^",
    "\n\n    (>\n    || _\n  _(__)_",
    "\n    (>\n    ||\n  _(__)_\n   ^/ ^",
    "\n    (>\n    ||\n  _(__)_\n   ^ \\^",
    "\n    (>\n <==||==>\n  _(__)_\n   ^^^^",
};
static const uint8_t GOOSE_IDLE_SEQ[] = { 0, 0, 4, 0, 1, 0, 2, 0, 7, 8, 7, 8, 0, 0, 3, 0, 4, 5, 5, 0, 6, 6, 0, 0, 9, 9, 0, 4, 0 };

static const char* const GOOSE_BUSY_FRAMES[] = {
    "\n    (>\n <==||==>\n  _(__)_\n   /^ ^\\",
    "\n    (>\n  =-||-=\n  _(__)_\n   ^/ \\^",
    "\n      (>>\n    _||_\n   (___)\n   //\\\\",
    "\n\n    \\v_\n  _(__)_\n   ^^^^",
    "    sss\n    (>>>\n    ||\n  _(__)_\n   ^^^^",
    "\n    (>\n  /=||=\\\n  _(__)_\n  >^^  ^^<",
};
static const uint8_t GOOSE_BUSY_SEQ[] = { 0, 1, 0, 1, 0, 1, 3, 3, 0, 1, 0, 1, 2, 2, 4, 4, 0, 1, 5, 5, 0 };

static const char* const GOOSE_ATTENTION_FRAMES[] = {
    "    HONK!\n     (>>\n     ||\n   _(__)_\n    ^^^^",
    "    HONK!\n  <<)\n    ||\n   _(__)_\n    ^^^^",
    "    HONK!\n       (>>\n      ||\n   _(__)_\n    ^^^^",
    "   HONK!!\n <===(>>===>\n     ||\n   _(__)_\n    ^^^^",
    "  HONK!!!\n       (>>>\n      _||_\n    _(__)_\n    /^^^\\",
    "    HONK!\n    (-->\n    ||\n  _(__)_\n  /^ ^^ ^\\",
};
static const uint8_t GOOSE_ATTENTION_SEQ[] = { 0, 3, 0, 1, 0, 2, 0, 3, 4, 4, 0, 1, 2, 0, 5, 0 };

static const char* const GOOSE_CELEBRATE_FRAMES[] = {
    "\n    (>\n   _||_\n  (____)\n   ^^^^",
    "  HONK!\n    (>>\n <==||==>\n  _(__)_\n   /  \\",
    " *HONK!HONK*\n    (^^>\n<===||===>\n  _(__)_\n   /  \\",
    "\n  <)\n<==||\n  _(__)_\n   ^^^^",
    "\n       (>\n       ||==>\n  _(__)_\n   ^^^^",
    "    \\o/\n    (>>\n <==||==>\n _(____)_\n   /^^\\",
};
static const uint8_t GOOSE_CELEBRATE_SEQ[] = { 0, 1, 2, 1, 0, 3, 4, 3, 4, 0, 1, 2, 1, 0, 5, 5 };

static const char* const GOOSE_DIZZY_FRAMES[] = {
    "\n   (x\n   ||\\\n  _(__)_\n   v^^v",
    "\n       x)\n      /||\n    _(__)_\n     v^^v",
    "\n    (@\n    ||~\n  _(__)_\n    ~vv~",
    "\n    @)\n   ~||\n  _(__)_\n    ~vv~",
    "\n    (X\n    /\\\n _(____)_\n  v^v  v^v",
};
static const uint8_t GOOSE_DIZZY_SEQ[] = { 0, 1, 0, 1, 2, 3, 0, 1, 0, 1, 4, 4, 2, 3 };

static const char* const GOOSE_HEART_FRAMES[] = {
    "\n    (^>\n    ||\n  _(__)_\n   ^^^^",
    "\n   #(^>#\n    ||\n  _(__)_\n   ^^^^",
    "\n    (<3>\n    ||\n  _(__)_\n   ^^^^",
    "\n    (@>\n <==||==>\n  _(__)_\n   ^^^^",
    "    ~~~\n    (->\n    ||\n  _(__)_\n   ^^^^",
};
static const uint8_t GOOSE_HEART_SEQ[] = { 0, 0, 1, 0, 2, 2, 0, 1, 0, 4, 0, 0, 3, 3, 0, 1, 0, 2, 1, 0 };

} // namespace

extern const SpeciesData kGooseSpecies = {
    "Goose",
    {
        BUDDY_ANIM_SEQ(GOOSE_SLEEP_FRAMES, GOOSE_SLEEP_SEQ, 5),
        BUDDY_ANIM_SEQ(GOOSE_IDLE_FRAMES, GOOSE_IDLE_SEQ, 5),
        BUDDY_ANIM_SEQ(GOOSE_BUSY_FRAMES, GOOSE_BUSY_SEQ, 5),
        BUDDY_ANIM_SEQ(GOOSE_ATTENTION_FRAMES, GOOSE_ATTENTION_SEQ, 5),
        BUDDY_ANIM_SEQ(GOOSE_CELEBRATE_FRAMES, GOOSE_CELEBRATE_SEQ, 3),
        BUDDY_ANIM_SEQ(GOOSE_DIZZY_FRAMES, GOOSE_DIZZY_SEQ, 4),
        BUDDY_ANIM_SEQ(GOOSE_HEART_FRAMES, GOOSE_HEART_SEQ, 5),
    },
};
