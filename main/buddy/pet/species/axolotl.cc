#include "species_data.h"

namespace {
static const char* const AXOLOTL_SLEEP_FRAMES[] = {
    "\n}~(______)~{\n}~( -  - )~{\n  ( .__. )\n  (_/  \\_)",
    "\n}~(______)~{\n}~( _  _ )~{\n  ( .__. )\n  (_/  \\_)",
    "\n~}(______){~\n~}( _  _ ){~\n  ( ____ )\n  ~_/  \\_~",
    "\n\n}~(.____.)~{\n }~(- __ -)~{\n  (__//__)",
    "\n\n  }~~~~~_\n }~( -- -)=\n  (__----)",
    "\n}~(______)~{\n}~( o  o )~{\n  ( oOOo )\n  (_/  \\_)",
};
static const uint8_t AXOLOTL_SLEEP_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 1, 0, 1, 0, 1, 3, 3, 4, 4, 3, 4, 3, 3, 1, 5, 1, 1 };

static const char* const AXOLOTL_IDLE_FRAMES[] = {
    "\n}~(______)~{\n}~( o  o )~{\n  ( .--. )\n  (_/  \\_)",
    "\n}~(______)~{\n}~(o   o )~{\n  ( .--. )\n  (_/  \\_)",
    "\n}~(______)~{\n}~( o   o)~{\n  ( .--. )\n  (_/  \\_)",
    "\n}~(______)~{\n}~( ^  ^ )~{\n  ( .--. )\n  (_/  \\_)",
    "\n}~(______)~{\n}~( -  - )~{\n  ( .--. )\n  (_/  \\_)",
    "\n}}~(_____)~{\n}}~(o  o )~{\n  ( .--. )\n  (_/  \\_)",
    "\n}~(_____)~{{\n}~(o  o )~{{\n  ( .--. )\n  (_/  \\_)",
    "\n}~(______)~{\n}~( o  o )~{\n  ( wwww )\n  (_/  \\_)",
    "\n}~(______)~{\n}~( o  o )~{\n  ( WWWW )\n  (_/  \\_)",
    "\n~}(______){~\n~}( o  o ){~\n  ( .--. )\n  ~_/  \\_~",
};
static const uint8_t AXOLOTL_IDLE_SEQ[] = { 0, 0, 0, 1, 0, 2, 0, 4, 0, 5, 0, 6, 0, 7, 8, 7, 8, 0, 0, 3, 3, 0, 4, 9, 9, 0, 0, 1, 2, 1, 2, 0 };

static const char* const AXOLOTL_BUSY_FRAMES[] = {
    "\n}~(______)~{\n}~( v  v )~{\n  (  --  )\n /(_/  \\_)\\",
    "\n}~(______)~{\n}~( v  v )~{\n  (  __  )\n \\(_/  \\_)/",
    "      ?\n}~(______)~{\n}~( ^  ^ )~{\n  (  ..  )\n  (_/  \\_)",
    "      /\n}~(_____)~{\n}~( o  o )~{\n  ( .--. ) /\n  (_/  \\_)",
    "      *\n}~(______)~{\n}~( O  O )~{\n  (  ^^  )\n /(_/  \\_)\\",
    "    ~~~\n}~(______)~{\n}~( -  - )~{\n  (  __  )\n  (_/  \\_)",
};
static const uint8_t AXOLOTL_BUSY_SEQ[] = { 0, 1, 0, 1, 0, 1, 2, 2, 0, 1, 0, 1, 3, 3, 2, 4, 0, 1, 0, 1, 5 };

static const char* const AXOLOTL_ATTENTION_FRAMES[] = {
    "    ^  ^\n}}~(______)~{{\n}}~( O  O )~{{\n  (  O   )\n  (_/  \\_)",
    "    ^  ^\n}}~(______)~{{\n}}~(O    O)~{{\n  (  O   )\n  (_/  \\_)",
    "    ^  ^\n}}~(______)~{{\n}}~(O    O)~{{\n  (   O  )\n  (_/  \\_)",
    "    ^  ^\n}}~(______)~{{\n}}~( ^  ^ )~{{\n  (  O   )\n  (_/  \\_)",
    "    ^  ^\n}}}~(____)~{{{\n}}}~( O  O)~{{{\n  (  O   )\n /(_/  \\_)\\",
    "    ^  ^\n}~(______)~{\n}~( o  o )~{\n  (  .   )\n  (_/  \\_)",
};
static const uint8_t AXOLOTL_ATTENTION_SEQ[] = { 0, 4, 0, 1, 0, 2, 0, 3, 4, 4, 0, 1, 2, 0, 5, 0 };

static const char* const AXOLOTL_CELEBRATE_FRAMES[] = {
    "\n}~(______)~{\n}~( ^  ^ )~{\n  (  ww  )\n /(_/  \\_)\\",
    "  \\(    )/\n}~(______)~{\n}~( ^  ^ )~{\n  (  ww  )\n  (_/  \\_)",
    "  \\^    ^/\n}~(______)~{\n}~( ^  ^ )~{\n  (  WW  )\n  (_/  \\_)",
    "\n}~(______)~{\n}~(<    <)~{\n  (  ww  ) /\n  (_/  \\_)",
    "\n}~(______)~{\n}~(>    >)~{\n\\ (  ww  )\n  (_/  \\_)",
    "    \\__/\n}~(______)~{\n}~( ^  ^ )~{\n/ (  WW  ) \\\n  (_/  \\_)",
};
static const uint8_t AXOLOTL_CELEBRATE_SEQ[] = { 0, 1, 2, 1, 0, 3, 4, 3, 4, 0, 1, 2, 1, 0, 5, 5 };

static const char* const AXOLOTL_DIZZY_FRAMES[] = {
    "\n}~(______)~{\n}~( @  @ )~{\n  ( ~~~~ )\n  (_/  \\_)",
    "\n }~(______)~{\n }~( @  @ )~{\n  ( ~~~~ )\n  (_/  \\_)",
    "\n}~(______)~{\n}~( x  @ )~{\n  ( ~vv~ )\n  (_/  \\_)",
    "\n}~(______)~{\n}~( @  x )~{\n  ( vv~~ )\n  (_/  \\_)",
    "\n~}(______){~\n~}( @  @ ){~\n  (  --  )\n /~_/  \\_~\\",
};
static const uint8_t AXOLOTL_DIZZY_SEQ[] = { 0, 1, 0, 1, 2, 3, 0, 1, 0, 1, 4, 4, 2, 3 };

static const char* const AXOLOTL_HEART_FRAMES[] = {
    "\n}~(______)~{\n}~( ^  ^ )~{\n  ( .vv. )\n  (_/  \\_)",
    "\n}~(______)~{\n}~(#^  ^#)~{\n  ( .vv. )\n  (_/  \\_)",
    "\n}~(______)~{\n}~(<3  <3)~{\n  ( .vv. )\n  (_/  \\_)",
    "\n~}(______){~\n~}( @  @ ){~\n  ( .vv. )\n /(_/  \\_)\\",
    "\n}~(______)~{\n}~( -  - )~{\n  ( ^^^^ )\n  (_/  \\_)",
    "\n}~(______)~{\n}~( ^  - )~{\n  ( .vv. )\n  (_/  \\_)",
};
static const uint8_t AXOLOTL_HEART_SEQ[] = { 0, 0, 1, 0, 2, 2, 0, 1, 0, 4, 0, 0, 3, 3, 0, 1, 0, 2, 5, 0 };

} // namespace

extern const SpeciesData kAxolotlSpecies = {
    "Axolotl",
    {
        BUDDY_ANIM_SEQ(AXOLOTL_SLEEP_FRAMES, AXOLOTL_SLEEP_SEQ, 5),
        BUDDY_ANIM_SEQ(AXOLOTL_IDLE_FRAMES, AXOLOTL_IDLE_SEQ, 5),
        BUDDY_ANIM_SEQ(AXOLOTL_BUSY_FRAMES, AXOLOTL_BUSY_SEQ, 5),
        BUDDY_ANIM_SEQ(AXOLOTL_ATTENTION_FRAMES, AXOLOTL_ATTENTION_SEQ, 5),
        BUDDY_ANIM_SEQ(AXOLOTL_CELEBRATE_FRAMES, AXOLOTL_CELEBRATE_SEQ, 3),
        BUDDY_ANIM_SEQ(AXOLOTL_DIZZY_FRAMES, AXOLOTL_DIZZY_SEQ, 4),
        BUDDY_ANIM_SEQ(AXOLOTL_HEART_FRAMES, AXOLOTL_HEART_SEQ, 5),
    },
};
