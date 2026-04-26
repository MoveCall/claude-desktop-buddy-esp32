#include "species_data.h"

namespace {
static const char* const MUSHROOM_SLEEP_FRAMES[] = {
    "\n .-o-OO-o-.\n(__________)\n   |-  - |\n   |____|",
    "\n .-O-oo-O-.\n(__________)\n   |-  - |\n   |____|",
    "\n .-o-oo-o-.\n(__________)\n   |_  _ |\n   |____|",
    "\n  .-o-OO-o-.\n  (________)\n    |-  -| /\n    |____|",
    "\n  .-o-OO-o-.\n  (________)\n    |z  z| /\n    |____|",
    "\n .-O-OO-O-.\n(__________)\n   |o  o |\n   |____|",
};
static const uint8_t MUSHROOM_SLEEP_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 1, 0, 1, 0, 1, 3, 4, 3, 4, 3, 4, 3, 3, 1, 5, 1, 1 };

static const char* const MUSHROOM_IDLE_FRAMES[] = {
    "\n .-o-OO-o-.\n(__________)\n   |o   o|\n   |____|",
    "\n .-o-OO-o-.\n(__________)\n   |o  o |\n   |____|",
    "\n .-o-OO-o-.\n(__________)\n   | o  o|\n   |____|",
    "\n .-o-OO-o-.\n(__________)\n   |^   ^|\n   |____|",
    "\n .-o-OO-o-.\n(__________)\n   |-   -|\n   |____|",
    "\n .-O-oo-O-.\n(__________)\n   |o   o|\n   |____|",
    "  . o .  .\n .-o-OO-o-.\n(__________)\n   |o   o|\n   |____|",
    " o  .  o .\n .-O-oo-O-.\n(__________)\n   |o   o|\n   |____|",
    "\n .~o-OO-o~.\n(~~~~~~~~~~)\n   |o   o|\n   |____|",
    "\n  .-o-OO-o-.\n  (________)\n   |o   o|\n   |____|",
};
static const uint8_t MUSHROOM_IDLE_SEQ[] = { 0, 0, 0, 1, 0, 2, 0, 4, 0, 5, 0, 0, 6, 7, 6, 7, 0, 0, 3, 3, 0, 4, 8, 8, 0, 0, 9, 9, 0, 0 };

static const char* const MUSHROOM_BUSY_FRAMES[] = {
    "\n .-o-OO-o-.\n(__________)\n   |v   v|\n  /|____|\\",
    "\n .-o-OO-o-.\n(__________)\n   |v   v|\n  \\|____|/",
    "      ?\n .-O-oo-O-.\n(__________)\n   |^   ^|\n   |____|",
    "    [_]\n .-o-OO-o|.\n(________|_)\n   |o   o|\n   |____|",
    "      *\n .-O-OO-O-.\n(__________)\n   |O   O|\n  /|____|\\",
    "    ~~~\n .-o-OO-o-.\n(__________)\n   |-   -|\n   |____|",
};
static const uint8_t MUSHROOM_BUSY_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 2, 0, 1, 0, 1, 3, 3, 2, 4, 0, 1, 0, 1, 5 };

static const char* const MUSHROOM_ATTENTION_FRAMES[] = {
    "\n /^o-OO-o^\\\n(__________)\n   |O   O|\n   |____|",
    "\n /^o-OO-o^\\\n(__________)\n   |O  O |\n   |____|",
    "\n /^o-OO-o^\\\n(__________)\n   | O  O|\n   |____|",
    "\n /^o-OO-o^\\\n(__________)\n   |^   ^|\n   |____|",
    "    ^  ^\n/^^o-OO-o^^\\\n(__________)\n   |O   O|\n  /|____|\\",
    "\n /^o-OO-o^\\\n(__________)\n   |o   o|\n   |____|",
};
static const uint8_t MUSHROOM_ATTENTION_SEQ[] = { 0, 4, 0, 1, 0, 2, 0, 3, 4, 4, 0, 1, 2, 0, 5, 0 };

static const char* const MUSHROOM_CELEBRATE_FRAMES[] = {
    "\n .-o-OO-o-.\n(__________)\n   |^   ^|\n  /|____|\\",
    "  \\(    )/\n .-o-OO-o-.\n(__________)\n   |^   ^|\n   |____|",
    "  \\^    ^/\n .-O-OO-O-.\n(__________)\n   |^   ^|\n   |____|",
    "\n .-o-OO-o-.\n(__________)\n  <|<   <|\n   |____|",
    "\n .-o-OO-o-.\n(__________)\n   |>   >|>\n   |____|",
    "    \\__/\n .-O-OO-O-.\n(__________)\n   |^   ^|\n /|____|\\",
};
static const uint8_t MUSHROOM_CELEBRATE_SEQ[] = { 0, 1, 2, 1, 0, 3, 4, 3, 4, 0, 1, 2, 1, 0, 5, 5 };

static const char* const MUSHROOM_DIZZY_FRAMES[] = {
    "\n.-o-OO-o-.\n(________)\n  |@   @|\n  |____|",
    "\n  .-o-OO-o-.\n  (________)\n    |@   @|\n    |____|",
    "\n .~o-OO-o~.\n(~~~~~~~~~~)\n   |x   @|\n   |~v~~|",
    "\n .~o-OO-o~.\n(~~~~~~~~~~)\n   |@   x|\n   |~~v~|",
    "\n .-o-OO-o-.\n(__________)\n   |@   @|\n /-|_---_|\\",
};
static const uint8_t MUSHROOM_DIZZY_SEQ[] = { 0, 1, 0, 1, 2, 3, 0, 1, 0, 1, 4, 4, 2, 3 };

static const char* const MUSHROOM_HEART_FRAMES[] = {
    "\n .-o-OO-o-.\n(__________)\n   |^   ^|\n   |____|",
    "\n .-o-OO-o-.\n(__________)\n  #|^   ^|#\n   |____|",
    "\n .-O-oo-O-.\n(__________)\n   |<3 <3|\n   |____|",
    "\n .-O-OO-O-.\n(__________)\n   |@   @|\n  /|____|\\",
    "\n .-o-OO-o-.\n(__________)\n   |-   -|\n   |^^^^|",
};
static const uint8_t MUSHROOM_HEART_SEQ[] = { 0, 0, 1, 0, 2, 2, 0, 1, 0, 4, 0, 0, 3, 3, 0, 1, 0, 2, 1, 0 };

} // namespace

extern const SpeciesData kMushroomSpecies = {
    "Mushroom",
    {
        BUDDY_ANIM_SEQ(MUSHROOM_SLEEP_FRAMES, MUSHROOM_SLEEP_SEQ, 5),
        BUDDY_ANIM_SEQ(MUSHROOM_IDLE_FRAMES, MUSHROOM_IDLE_SEQ, 5),
        BUDDY_ANIM_SEQ(MUSHROOM_BUSY_FRAMES, MUSHROOM_BUSY_SEQ, 5),
        BUDDY_ANIM_SEQ(MUSHROOM_ATTENTION_FRAMES, MUSHROOM_ATTENTION_SEQ, 5),
        BUDDY_ANIM_SEQ(MUSHROOM_CELEBRATE_FRAMES, MUSHROOM_CELEBRATE_SEQ, 3),
        BUDDY_ANIM_SEQ(MUSHROOM_DIZZY_FRAMES, MUSHROOM_DIZZY_SEQ, 4),
        BUDDY_ANIM_SEQ(MUSHROOM_HEART_FRAMES, MUSHROOM_HEART_SEQ, 5),
    },
};
