#include "species_data.h"

namespace {
static const char* const CACTUS_SLEEP_FRAMES[] = {
    "\n    ____\n   |--  |\n   |_  _|\n    |  |",
    "\n    ____\n . |--  |.\n   |_  _|\n    |  |",
    "\n    ____\n   |~~  |\n   |_  _|\n   ~|  |~",
    "\n    ____\n \\ |--  |\n  \\|_  _|\n    |  |",
    "\n    ____\n   |--  | /\n   |_  _|/\n    |  |",
    "\n\n    ____\n   |--  |\n   |_  _|",
};
static const uint8_t CACTUS_SLEEP_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 1, 0, 1, 0, 1, 3, 3, 4, 4, 3, 4, 0, 0, 1, 5, 1, 1 };

static const char* const CACTUS_IDLE_FRAMES[] = {
    "\n n  ____  n\n | |o  o| |\n |_|    |_|\n   |    |",
    "\n n  ____  n\n | |o   o|\n |_|    |_|\n   |    |",
    "\n n  ____  n\n | | o  o|\n |_|    |_|\n   |    |",
    "\n n  ____  n\n | |^  ^| |\n |_|    |_|\n   |    |",
    "\n n  ____  n\n | |-  -| |\n |_|    |_|\n   |    |",
    "  \\\n   \\____  n\n n |o  o| |\n |_|    |_|\n   |    |",
    "         /\n n  ____/\n | |o  o| n\n |_|    |_|\n   |    |",
    "\nn   ____  n\n| |o  o| |\n |_|    |_|\n   |    |",
    "\n n  ____   n\n | |o  o| |\n |_|    |_|\n    |    |",
    "      o\n n  ____  n\n | |- -| |\n |_|  o |_|\n   |    |",
};
static const uint8_t CACTUS_IDLE_SEQ[] = { 0, 0, 0, 1, 0, 2, 0, 4, 0, 3, 0, 0, 5, 5, 0, 6, 6, 0, 0, 7, 8, 7, 8, 0, 9, 9, 0, 4, 0, 0, 1, 2, 0 };

static const char* const CACTUS_BUSY_FRAMES[] = {
    "\n n  ____  n\n | |v  v| |\n |_|  --|_|\n   |    |",
    "\n n  ____  n\n | |- -| |\n |_|  __|_|\n   |    |",
    "      ?\n n  ____  n\n | |^  ^| |\n |_|  ..|_|\n   |    |",
    "\n n  ____  /=\n | |o  o|/\n |_|  --|_|\n   |    |",
    "      *\n n  ____  n\n | |O  O| |\n |_|  ^^|_|\n   |    |",
    "      .\n n  ____  n\n | |o  o| |\n |_|  --|_|\n   |    |",
};
static const uint8_t CACTUS_BUSY_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 2, 0, 1, 0, 1, 3, 3, 2, 4, 0, 1, 0, 1, 5 };

static const char* const CACTUS_ATTENTION_FRAMES[] = {
    " *  ____  *\n n *|O  O|* n\n |*|    |*|\n |_|    |_|\n  *|    |*",
    " *  ____  *\n n *|O  O|* n\n |*|O    |\n |_|    |_|\n  *|    |*",
    " *  ____  *\n n *|O  O|* n\n |*|    O|*|\n |_|    |_|\n  *|    |*",
    " *  ^^^^  *\n n *|O  O|* n\n |*|    |*|\n |_|    |_|\n  *|    |*",
    "*** **** ***\n*n*|O  O|*n*\n*|*|    |*|*\n*|_|    |_|*\n *|    |*",
    " *  ____  *\n n *|o  o|* n\n |*|    |*|\n |_|  . |_|\n  *|    |*",
};
static const uint8_t CACTUS_ATTENTION_SEQ[] = { 0, 4, 0, 1, 0, 2, 0, 3, 4, 4, 0, 1, 2, 0, 5, 0 };

static const char* const CACTUS_CELEBRATE_FRAMES[] = {
    "\n n  ____  n\n | |^  ^| |\n |_|  ww|_|\n  /|    |\\",
    "    .--.\n \\  ____  /\n  \\|^  ^|/\n |_|  ww|_|\n   |    |",
    "    (**)\n  \\ ____ /\n \\ |^  ^| /\n  ||  ww||\n   |    |",
    "\n<n  ____  n\n | |<  <| |\n |_|    |_|\n  /|    |",
    "\n n  ____  n>\n | |>  >| |\n |_|    |_|\n   |    |\\",
    "    @--@\n n  \\__/  n\n | |^  ^| |\n |_|  WW|_|\n  /|    |\\",
};
static const uint8_t CACTUS_CELEBRATE_SEQ[] = { 0, 1, 2, 1, 0, 3, 4, 3, 4, 0, 1, 2, 1, 0, 5, 5 };

static const char* const CACTUS_DIZZY_FRAMES[] = {
    "\nn  ____   n\n| |@  @|  |\n |_|~~  |_|\n   |    |",
    "\n n   ____  n\n |  |@  @| |\n |_|  ~~|_|\n   |    |",
    "\n n  ____  n\n | |x  @| |\n |_|  ~v|_|\n   /    \\",
    "\n n  ____  n\n | |@  x| |\n |_|  v~|_|\n   \\    /",
    "\n n  ____  n\n | |@  @| |\n |_|  --|_|\n  /-|  |-\\",
};
static const uint8_t CACTUS_DIZZY_SEQ[] = { 0, 1, 0, 1, 2, 3, 0, 1, 0, 1, 4, 4, 2, 3 };

static const char* const CACTUS_HEART_FRAMES[] = {
    "    @\n n  ____  n\n | |^  ^| |\n |_|  ww|_|\n   |    |",
    "    @\n n  ____  n\n |#|^  ^|#|\n |_|  ww|_|\n   |    |",
    "    @--@\n n  \\__/  n\n | |<3<3| |\n |_|  ww|_|\n   |    |",
    "       @\n n  ____  n\n | |@  @| |\n |_|  ww|_|\n  /|    |\\",
    "    @\n n  ____  n\n | |- -| |\n |_|  ^^|_|\n   |    |",
};
static const uint8_t CACTUS_HEART_SEQ[] = { 0, 0, 1, 0, 2, 2, 0, 1, 0, 4, 0, 0, 3, 3, 0, 1, 0, 2, 1, 0 };

} // namespace

extern const SpeciesData kCactusSpecies = {
    "Cactus",
    {
        BUDDY_ANIM_SEQ(CACTUS_SLEEP_FRAMES, CACTUS_SLEEP_SEQ, 5),
        BUDDY_ANIM_SEQ(CACTUS_IDLE_FRAMES, CACTUS_IDLE_SEQ, 5),
        BUDDY_ANIM_SEQ(CACTUS_BUSY_FRAMES, CACTUS_BUSY_SEQ, 5),
        BUDDY_ANIM_SEQ(CACTUS_ATTENTION_FRAMES, CACTUS_ATTENTION_SEQ, 5),
        BUDDY_ANIM_SEQ(CACTUS_CELEBRATE_FRAMES, CACTUS_CELEBRATE_SEQ, 3),
        BUDDY_ANIM_SEQ(CACTUS_DIZZY_FRAMES, CACTUS_DIZZY_SEQ, 4),
        BUDDY_ANIM_SEQ(CACTUS_HEART_FRAMES, CACTUS_HEART_SEQ, 5),
    },
};
