#include "species_data.h"

namespace {
static const char* const RABBIT_SLEEP_FRAMES[] = {
    "\n    (\\_/)\n   ( -.- )\n  (zzz___)\n   `\"\"\"\"`",
    "\n    (\\_/)\n   ( -_- )\n  (___zz_)\n   `\"\"\"\"`",
    "\n    (v_v)\n   ( -.- )\n  (___..)\n   `\"\"\"\"`",
    "\n\n   .-^^^-.\n  ( -- zZ )\n   `~~~~`",
    "\n    (\\_/)\n   ( -.- )\n  (~__zz_)\n   `\"\"\"\"`",
    "\n    (\\_/)\n   ( u.u )\n  (___oo_)\n   `\"\"\"\"`",
};
static const uint8_t RABBIT_SLEEP_SEQ[] = { 0, 1, 0, 1, 0, 1, 4, 1, 0, 1, 0, 1, 3, 3, 3, 3, 2, 2, 1, 5, 1, 1 };

static const char* const RABBIT_IDLE_FRAMES[] = {
    "    (\\_/)\n   ( o o )\n  =(  v  )=\n   (\")_(\")\n",
    "    (\\_/)\n   (o  o )\n  =(  v  )=\n   (\")_(\")\n",
    "    (\\_/)\n   ( o  o)\n  =(  v  )=\n   (\")_(\")\n",
    "    (\\_/)\n   ( ^ ^ )\n  =(  v  )=\n   (\")_(\")\n",
    "    (\\_/)\n   ( - - )\n  =(  v  )=\n   (\")_(\")\n",
    "    (/_\\)\n   ( o o )\n  =(  v  )=\n   (\")_(\")\n",
    "    (\\_/)\n   ( o o )\n  =(  ^  )=\n   (\")_(\")\n",
    "    (\\_/)\n   ( o o )\n  =(  *  )=\n   (\")_(\")\n",
    "    (\\_/)\n   ( o o )\n  =(  V  )= >--\n   (\")_(\")\n",
    "   (\\___/)\n  ( o   o )\n =(   v   )=\n   (\")_(\")\n",
};
static const uint8_t RABBIT_IDLE_SEQ[] = { 0, 0, 0, 1, 0, 2, 0, 4, 0, 5, 0, 0, 6, 7, 6, 7, 0, 0, 3, 3, 0, 4, 8, 8, 0, 0, 9, 9, 0, 0 };

static const char* const RABBIT_BUSY_FRAMES[] = {
    "    (\\_/)\n   ( v v )\n  =(  v  )=\n  /(\")_(\")\\\n  ~~~~~~~~",
    "    (\\_/)\n   ( v v )\n  =(  v  )=\n  \\(\")_(\")/\n  ~~~~~~~~",
    "  ? (\\_/)\n   ( o o )\n  =(  v  )=\n   (\")_(\")\n",
    "    (\\_/) >-\n   ( o o )==\n  =(  W  )=\n   (\")_(\")\n",
    "  * (\\_/) *\n   ( O O )\n  =(  ^  )=\n  /(\")_(\")\\\n",
    "    (\\_/)\n   ( - - )\n  =(  _  )=\n   (\")_(\")\n    ~~~~",
};
static const uint8_t RABBIT_BUSY_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 2, 0, 1, 0, 1, 3, 3, 2, 4, 0, 1, 0, 1, 5 };

static const char* const RABBIT_ATTENTION_FRAMES[] = {
    "   /|  |\\\n  /(\\_/)\\\n  ( O  O )\n  =(  v  )=\n   (\")_(\")",
    "   /|  |\\\n  /(\\_/)\\\n  (O   O )\n  =(  v  )=\n   (\")_(\")",
    "   /|  |\\\n  /(\\_/)\\\n  ( O   O)\n  =(  v  )=\n   (\")_(\")",
    "   /|  |\\\n  /(\\_/)\\\n  ( ^  ^ )\n  =(  v  )=\n   (\")_(\")",
    "  /|/  \\|\\\n  /(\\_/)\\\n  ( O  O )\n  =(  v  )=\n  /(\")_(\")\\",
    "   /|  |\\\n   (\\_/)\n  ( o  o )\n  =(  .  )=\n   (\")_(\")",
};
static const uint8_t RABBIT_ATTENTION_SEQ[] = { 0, 4, 0, 1, 0, 2, 0, 3, 4, 4, 0, 1, 2, 0, 5, 0 };

static const char* const RABBIT_CELEBRATE_FRAMES[] = {
    "    (\\_/)\n   ( ^ ^ )\n  =(  v  )=\n   (\")_(\")\n  ~~~~~~~~",
    "  \\ (\\_/) /\n   ( ^ ^ )\n  =(  v  )=\n   (\")_(\")\n",
    "  \\ (\\o/) /\n   ( ^ ^ )\n  =(  W  )=\n   ('')_('')\n",
    "    (\\_/)\n   ( <  < )\n  =(  v  )=/\n   (\")_(\")\n",
    "    (\\_/)\n   ( >  > )\n \\=(  v  )=\n   (\")_(\")\n",
    "   \\(\\_/)/\n   ( ^ ^ )\n  =(  W  )=\n  /(\")_(\")\\\n",
};
static const uint8_t RABBIT_CELEBRATE_SEQ[] = { 0, 1, 2, 1, 0, 3, 4, 3, 4, 0, 1, 2, 1, 0, 5, 5 };

static const char* const RABBIT_DIZZY_FRAMES[] = {
    "   (\\_/)\n  ( @ @ )\n =(  ~  )=\n  (\")_(\")\n",
    "    (\\_/)\n    ( @ @ )\n   =(  ~  )=\n    (\")_(\")\n",
    "    (\\_/)\n   ( x @ )\n  =(  ~v )=\n   (\")_(\")\n",
    "    (\\_/)\n   ( @ x )\n  =( v~  )=\n   (\")_(\")\n",
    "    (\\_/)\n   ( @ @ )\n  =(  -  )=\n  /(\")_(\")\\\n ~~~~~~~~~~",
};
static const uint8_t RABBIT_DIZZY_SEQ[] = { 0, 1, 0, 1, 2, 3, 0, 1, 0, 1, 4, 4, 2, 3 };

static const char* const RABBIT_HEART_FRAMES[] = {
    "    (\\_/)\n   ( ^ ^ )\n  =(  v  )=\n   (\")_(\")\n",
    "    (\\_/)\n   (#^ ^#)\n  =(  v  )=\n   (\")_(\")\n",
    "    (\\_/)\n   (<3 <3)\n  =(  v  )=\n   (\")_(\")\n",
    "   \\(\\_/)/\n   ( @ @ )\n  =(  v  )=\n  /(\")_(\")\\\n",
    "    (v_v)\n   ( - - )\n  =(  ^  )=\n   (\")_(\")\n",
};
static const uint8_t RABBIT_HEART_SEQ[] = { 0, 0, 1, 0, 2, 2, 0, 1, 0, 4, 0, 0, 3, 3, 0, 1, 0, 2, 1, 0 };

} // namespace

extern const SpeciesData kRabbitSpecies = {
    "Rabbit",
    {
        BUDDY_ANIM_SEQ(RABBIT_SLEEP_FRAMES, RABBIT_SLEEP_SEQ, 5),
        BUDDY_ANIM_SEQ(RABBIT_IDLE_FRAMES, RABBIT_IDLE_SEQ, 5),
        BUDDY_ANIM_SEQ(RABBIT_BUSY_FRAMES, RABBIT_BUSY_SEQ, 5),
        BUDDY_ANIM_SEQ(RABBIT_ATTENTION_FRAMES, RABBIT_ATTENTION_SEQ, 5),
        BUDDY_ANIM_SEQ(RABBIT_CELEBRATE_FRAMES, RABBIT_CELEBRATE_SEQ, 3),
        BUDDY_ANIM_SEQ(RABBIT_DIZZY_FRAMES, RABBIT_DIZZY_SEQ, 4),
        BUDDY_ANIM_SEQ(RABBIT_HEART_FRAMES, RABBIT_HEART_SEQ, 5),
    },
};
