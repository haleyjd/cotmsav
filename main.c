/*

  Circle of the Moon Save RAM Manipulation

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

// basic types
typedef unsigned char byte;

#ifndef __cplusplus
typedef enum { false, true } bool;
#endif

// each save file is 976 bytes in length
#define SAVEFILESIZE 976

// there are eight save files
#define NUMSAVEFILES 8

// offsets to each save file; the first file is after a 16-byte header;
// there is no space between files

#define OFFSET_FILE1 0x10

unsigned int fileoffsets[NUMSAVEFILES] =
{
   OFFSET_FILE1,                    // file 1
   OFFSET_FILE1 + SAVEFILESIZE,     // file 2
   OFFSET_FILE1 + SAVEFILESIZE * 2, // file 3
   OFFSET_FILE1 + SAVEFILESIZE * 3, // file 4
   OFFSET_FILE1 + SAVEFILESIZE * 4, // file 5
   OFFSET_FILE1 + SAVEFILESIZE * 5, // file 6
   OFFSET_FILE1 + SAVEFILESIZE * 6, // file 7
   OFFSET_FILE1 + SAVEFILESIZE * 7, // file 8
};

// first byte of a save file is a flag that indicates whether or not the file 
// exists; 00 = no, FF = yes

#define OFFSET_EXISTS 0x0000
#define EXISTS_NO     0
#define EXISTS_YES    0xFF

// from 0x01 to 0x08 is the null-extended file name in a 1-base alphabet
// (ie., A == 1, B == 2, etc)

#define OFFSET_NAME 0x0001
#define NAME_LENGTH 8

// Offset 0x0009 is the single-byte checksum, which is calculated by adding
// all bytes of the savefile together with normal unsigned overflow behavior
// after zeroing the current value of the checksum. If the checksum is not
// correct when the game is started, the entire save RAM file will be cleared!

#define OFFSET_CHECKSUM 0x0009

// Note: At offset 0x000c running to 0x0023 are the map control bytes, which
// contain bit flags which for certain mark what items have been collected,
// and probably also control such things as what bosses have been defeated,
// changing enemy sets, broken walls, etc. I may never be able to figure out
// the value of all the bits due to there being so many of them :(

// Game Mode: single-byte value that determines the mode of this game
// (VK, Shooter, Magician, Fighter, or Thief)
#define OFFSET_GAMEMODE 0x008c

// Time: Stored as 60 FPS ticker value at offset 0x090

#define OFFSET_TIME 0x0090

// 03/13/07: Covered map % is at offset 0x00ac and is mul'd by 10
#define OFFSET_MAP_PCT 0x00ac

// 03/14/07: The map! Through careful observation and a little graph paper
// magic, I confirmed my initial suspicion that the map data was stored as
// a bit-packed array.

#define OFFSET_MAP        0x00c0
#define PACKED_MAP_WIDTH  8
#define MAP_WIDTH         64
#define MAP_HEIGHT        40

// Player stats -- start at offset 0x02C6
 
#define OFFSET_HP1          0x02C6
#define OFFSET_HP2          0x02CA
#define OFFSET_MP1          0x02CE
#define OFFSET_MP2          0x02D2
#define OFFSET_HEARTS_CUR   0x02D4
#define OFFSET_HEARTS_MAX   0x02D6
#define OFFSET_SUBWEAPON    0x02D8
#define OFFSET_STR_BASE     0x02DC
#define OFFSET_STR_EQUIP    0x02DE
#define OFFSET_STR_DSS      0x02E0
#define OFFSET_STR_UNKNOWN  0x02E2
#define OFFSET_DEF_BASE     0x02E4
#define OFFSET_DEF_EQUIP    0x02E6
#define OFFSET_DEF_DSS      0x02E8
#define OFFSET_DEF_UNKNOWN  0x02EA
#define OFFSET_INT_BASE     0x02EC
#define OFFSET_INT_EQUIP    0x02EE
#define OFFSET_INT_DSS      0x02F0
#define OFFSET_INT_UNKNOWN  0x02F2
#define OFFSET_LCK_BASE     0x02F4
#define OFFSET_LCK_EQUIP    0x02F6
#define OFFSET_LCK_DSS      0x02F8
#define OFFSET_LCK_UNKNOWN  0x02FA
#define OFFSET_LEVEL        0x02FC
#define OFFSET_EXP          0x0300
#define OFFSET_EQUIP_ATTRIB 0x0304
#define OFFSET_EQUIP_ACTION 0x0305
#define OFFSET_EQUIP_ARMOR  0x0306
#define OFFSET_EQUIP_ARM1   0x0307
#define OFFSET_EQUIP_ARM2   0x0308


// DSS cards: array of 20 booleans stored at 0x030c

#define OFFSET_CARDS 0x030c

// DSS abilities: array of 100 booleans stored at 0x0320

#define OFFSET_ABILITIES 0x0320

// Unknown Value: I'm still working on this; at offset 0x0384, just before
// the inventory, is a value that seems to change with no certain relationship
// to anything the player has done.

#define OFFSET_UNKNOWN1 0x0384

// The inventory; we'll actually start reading at the unknown byte just to
// simplify things; that value will end up in the "none" slot where it's not
// used.

#define OFFSET_INVENTORY 0x0384

// 03/16/07: Heart, HP, and MP Ups

#define OFFSET_HEART_UP 0x03C4
#define OFFSET_HP_UP    0x03C5
#define OFFSET_MP_UP    0x03C6

// Relics: Note -- the order of these needs to be verified

#define OFFSET_RELICS 0x03C7

// DSS cards enum
enum
{
   CARD_NONE,
   CARD_SALAMANDER,
   CARD_SERPENT,
   CARD_MANDRAGORA,
   CARD_GOLEM,
   CARD_COCKATRICE,
   CARD_MANTICORE,
   CARD_GRIFFIN,
   CARD_THUNDERBIRD,
   CARD_UNICORN,
   CARD_BLACKDOG,
   CARD_MERCURY,
   CARD_VENUS,
   CARD_JUPITER,
   CARD_MARS,
   CARD_DIANA,
   CARD_APOLLO,
   CARD_NEPTUNE,
   CARD_SATURN,
   CARD_URANUS,
   CARD_PLUTO,
   NUMDSS
};

typedef struct dsscard_s
{
   const char *name;
   const char *description;
} dsscard_t;

dsscard_t dsscards[NUMDSS] =
{
   { "None",       "Nothing." },
   { "Salamander", "A lizard bathed in flames. Embodiment of the fire spirit, Salamander.\n"
                       " Has the power of Fire." },
   { "Serpent",    "The Serpent is said to be a dragon swimming in the sea.\n"
                       " Has the power of Ice." },
   { "Mandragora", "The Mandragora is represented as a humanoid with roots instead of\n"
                       " feet. Has the power of plants." },
   { "Golem",      "The Golem is a mockery of man made from clay. Has the potential of Earth." },
   { "Cockatrice", "The Cockatrice is said to have the ability to turn things to stone\n."
                       " Has the power of Stone." },
   { "Manticore",  "The Manticore is said to have a body of a lion and the venemous tail\n"
                       " of a scorpion. Power of Poison." },
   { "Griffin",    "The Griffin is said to have the head and wings of an eagle and body\n"
                       " of a lion. Has the power of Wind." },
   { "Thunderbird", "The legendary Thunderbird is said to have been able to release\n"
                       " lightning. Has the power of Electricity." },
   { "Unicorn",    "The Unicorn is said to have been white with a single holy horn on its\n"
                       " head. Has the power of Light." },
   { "Black Dog",  "The Black Dog is said to consume darkness. Has the power of Darkness." },
   { "Mercury",    "Mercury, the messenger of the gods. Has the potential of strength." },
   { "Venus",      "Venus, goddess of love and beauty. Has the potential of enchantment." },
   { "Jupiter",    "Jupiter, god of the heavens and the leader of Olympus.\n" 
                       " Has the potential of defense." },
   { "Mars",       "Mars, god of war. Has the potential of change." },
   { "Diana",      "Diana, goddess of the moon and hunting. Has the potential of creation." },
   { "Apollo",     "Apollo, god of the sun, music, and prophecy.\n" 
                       " Has the potential to create explosives." },
   { "Neptune",    "Neptune, god of the seas. Has the potential of healing." },
   { "Saturn",     "Saturn, god of agriculture and the father of Jupiter.\n"
                       " Has the potential of a familiar." },
   { "Uranus",     "Uranus, former god of the heavens. Has the potential of summoning." },
   { "Pluto",      "Pluto, god of the underworld. Has the potential of special." },
};

#define NUMABILITIES 100

// Inventory enumeration
enum
{
   INV_NONE,
   
   INV_LEATHER_ARMOR,
   INV_BRONZE_ARMOR,
   INV_GOLD_ARMOR,
   INV_CHAIN_MAIL,
   INV_STEEL_ARMOR,
   INV_PLATINUM_ARMOR,
   INV_DIAMOND_ARMOR,
   INV_MIRROR_ARMOR,
   INV_NEEDLE_ARMOR,
   INV_DARK_ARMOR,
   INV_SHINING_ARMOR,
   
   INV_COTTON_ROBE,
   INV_SILK_ROBE,
   INV_RAINBOW_ROBE,
   INV_MAGIC_ROBE,
   INV_SAGE_ROBE,
   
   INV_COTTON_CLOTHES,
   INV_PRISON_GARB,
   INV_STYLISH_SUIT,
   INV_NIGHT_SUIT,
   INV_NINJA_GARB,
   INV_SOLDIER_FATIGUES,
   
   INV_DOUBLE_GRIPS,
   INV_STAR_BRACELET,
   
   INV_STRENGTH_RING,
   INV_HARD_RING,
   INV_INTELLIGENCE_RING,
   INV_LUCK_RING,
   INV_CURSED_RING,

   INV_STRENGTH_ARMBAND,
   INV_DEFENSE_ARMBAND,
   INV_SAGE_ARMBAND,
   INV_GAMBLER_ARMBAND,
   
   INV_WRIST_BAND,
   INV_GAUNTLET,
   INV_ARM_GUARD,
   INV_MAGIC_GAUNTLET,
   INV_MIRACLE_ARMBAND,
   
   INV_TOY_RING,
   INV_BEAR_RING,

   INV_POTION,
   INV_MEAT,
   INV_SPICED_MEAT,
   INV_POTION_HIGH,
   INV_POTION_EX,
   INV_ANTIDOTE,
   INV_CURE_CURSE,
   INV_MIND_RESTORE,
   INV_MIND_HIGH,
   INV_MIND_EX,
   INV_HEART,
   INV_HEART_HIGH,
   INV_HEART_EX,
   INV_HEART_MEGA,
   
   NUMINV
};

// Inventory Item structure

typedef struct inventoryitem_s
{
   const char *name;
   const char *description;
   int rarity;
   int atk;
   int def;
   int intel;
   int lck;
} inventoryitem_t;

// Inventory Item Definitions

inventoryitem_t inventory_items[NUMINV] =
{
   { "None",              "Nothing." },
      
   { "Leather Armor",     "Armor made from leather.",       1,    0,   30,    0,    0 },
   { "Bronze Armor",      "Armor made from bronze.",        2,    0,   50,    0,    0 },
   { "Gold Armor",        "Armor made of gold.",            2,    0,   80,    0,    0 },
   { "Chain Mail",        "Armor made from chains.",        3,    0,  100,    0,    0 },
   { "Steel Armor",       "Plate armor made of "
                             "interlinking metal loops.",   3,    0,  120,    0,    0 },
   { "Platinum Armor",    "Plate armor made of platinum.",  3,    0,  150,    0,    0 },
   { "Diamond Armor",     "Armor made from diamonds.",      4,    0,  210,    0,    0 },
   { "Mirror Armor",      "Armor polished to a " 
                             "mirror-like surface.",        4,    0,  300,    0,    0 },
   { "Needle Armor",      "Plate armor covered in spikes.", 4,   10,  400,    0,    0 },
   { "Dark Armor",        "Cursed armor.",                  5,  -10,  550,  -10,  -10 },
   { "Shining Armor",     "Armor that gleams with light.",  5,   10,  500,   10,   10 },

   { "Cotton Robe",       "Robe made of cotton.",           1,    0,   25,  100,    0 },
   { "Silk Robe",         "Robe made of silk.",             3,    0,   40,  140,    0 },
   { "Rainbow Robe",      "A colorful robe.",               4,    0,  140,  250,   15 },
   { "Magic Robe",        "A robe that has magic within.",  4,    0,  200,  300,    0 },
   { "Sage Robe",         "A robe said to have belonged "
                             "to a sage.",                  5,    0,  250,  500,    0 },

   { "Cotton Clothes",    "Clothes made of cotton.",        1,    0,   20,    0,    0 },
   { "Prison Garb",       "Clothes that were worn by a "
                             "prisoner.",                   1,    5,   20,    0,    0 },
   { "Stylish Suit",      "You'll be popular while "
                             "wearing this.",               2,   10,   40,    0,    0 },
   { "Night Suit",        "A dark, black suit.",            3,   20,   60,   10,    0 },
   { "Ninja Garb",        "A black ninja suit.",            3,   30,   80,    0,    0 },
   { "Soldier Fatigues",  "Fatigues normally worn by "
                             "soldiers.",                   4,   50,  120,    0,   10 },
   { "Double Grips",      "Their power is released when "
                             "both are equipped.",          5,   75,   75,   75,   75 },
   { "Star Bracelet",     "Its power depends on which arm "
                             "it is on.",                   4,   25,   25,   25,   25 },

   { "Strength Ring",     "Strength increases while "
                             "equipped.",                   2,   50,  -10,  -10,    0 },
   { "Hard Ring",         "Defense increases while "
                             "equipped.",                   2,  -10,   50,    0,  -10 },
   { "Intelligence Ring", "Intelligence increases "
                             "while equipped.",             2,  -10,    0,   50,  -10 },
   { "Luck Ring",         "Luck increases while equipped.", 3,    0,  -10,  -10,   50 },
   { "Cursed Ring",       "Luck decreases greatly while "
                             "equipped.",                   3,   30,   30,    0, -100 },

   { "Strength Armband",  "Strength increases greatly "
                             "while equipped.",             5,  100,  -25,  -25,  -25 },
   { "Defense Armband",   "Defense increases greatly "
                             "while equipped.",             5,  -25,  100,  -25,  -25 },
   { "Sage Armband",      "Intelligence increases greatly "
                             "while equipped.",             5,  -25,  -25,  100,  -25 },
   { "Gambler Armband",   "Luck increases greatly while "
                             "equipped.",                   5,  -25,  -25,  -25,  100 },

   { "Wrist Band",        "Cotton armband.",                1,    5,    0,    0,    0 },
   { "Gauntlet",          "Increases attack power while "
                             "equipped.",                   1,   15,    0,    0,    0 },
   { "Arm Guard",         "Protects the arm while "
                             "equipped.",                   2,    0,   10,    0,    0 },
   { "Magic Gauntlet",    "Magic power lies within the "
                             "gauntlet.",                   1,    0,    0,   10,    0 },
   { "Miracle Armband",   "Luck increases while equipped.", 3,    0,    0,    0,   10 },

   { "Toy Ring",          "Useless ring.",                  1                         },
   { "Bear Ring",         "Ring with the curse of the "
                             "bear.",                       5, -100, -100, -100, -100 },

   { "Potion",            "Restores 20 HP.",                2 },   
   { "Meat",              "Restores 50 HP.",                2 },
   { "Spiced Meat",       "Restores 100 HP.",               3 },
   { "Potion High",       "Restores 250 HP.",               4 },
   { "Potion Ex",         "Restores all HP.",               5 },
   { "Antidote",          "Cures Poison status.",           1 },
   { "Cure Curse",        "Cures Curse status.",            2 },
   { "Mind Restore",      "Recover 30% MP.",                2 },
   { "Mind High",         "Recover 50% MP.",                4 },
   { "Mind Ex",           "Recover 100% MP.",               5 },
   { "Heart",             "Gain 10 Hearts.",                1 },
   { "Heart High",        "Gain 25 Hearts.",                2 },
   { "Heart Ex",          "Gain 50 Hearts.",                4 },
   { "Heart Mega",        "Gain 100 Hearts.",               5 },
};

// Relics enum
enum
{
   RELIC_DASHBOOTS,
   RELIC_DOUBLEJUMP,
   RELIC_TACKLE,
   RELIC_KICKBOOTS,
   RELIC_HEAVYRING,
   RELIC_CLEANSING,
   RELIC_ROCWING,
   RELIC_LASTKEY,
   NUMRELICS
};

// Relic items

typedef struct relic_s
{
   const char *name;
   const char *description;
} relic_t;

relic_t relics[NUMRELICS] =
{
   { "Dash Boots",  "Allows user to run fast."                    },
   { "Double Jump", "Allows user to jump once again in midair."   },
   { "Tackle",      "Allows user to dash and break stone blocks." },
   { "Kick Boots",  "Allows user to kick off of vertical walls."  },
   { "Heavy Ring",  "Allows user to push heavy boxes."            },
   { "Cleansing",   "Purifies certain bodies of water."           },
   { "Roc Wing",    "Allows user to jump incredibly high."        },
   { "Last Key",    "Opens the door to Dracula's chambers."       },
};

//
// Subweapon enum
//
// haleyjd 03/12/07: I was able to verify all the subweapon numbers by noting
// the order of the tiles in video memory, although I had already suspected
// this order due to this being the way they were listed in the instruction
// booklet as well.
//
enum
{
   SUBWEAPON_NONE,
   SUBWEAPON_DAGGER,
   SUBWEAPON_AXE,
   SUBWEAPON_HOLYWATER,
   SUBWEAPON_CROSS,
   SUBWEAPON_POCKETWATCH,
   SUBWEAPON_HOMINGDAGGER, // Hack: actual value in save file is 257 (0x101)
   NUMSUBWEAPONS
};

// 03/16/07: For the homing dagger, the game stores an odd value. I change
// it into 6 for the purposes of this program, as that's easier to handle
#define SUBWEAPON_HOMINGDAGGER_FILEVAL 0x101

// Subweapons

const char *subweapons[NUMSUBWEAPONS] =
{
   "None",
   "Dagger",
   "Axe",
   "Holy Water",
   "Cross",
   "Pocket Watch",
   "Homing Dagger",
};

// header data
byte fileheader[16];

// At offset 11 in the header is a set of bit flags that indicate what modes of
// play are enabled.
#define HEADER_MODES_OFFSET 0x0B

// mode flags
enum
{
   MODE_FLAG_VAMPIREKILLER = 0x00,
   MODE_FLAG_SHOOTER       = 0x02,
   MODE_FLAG_MAGICIAN      = 0x04,
   MODE_FLAG_FIGHTER       = 0x08,
   MODE_FLAG_THIEF         = 0x10,
};

// raw mode values -- different from the header flags!
enum
{
   MODE_VAMPIREKILLER,
   MODE_SHOOTER,
   MODE_MAGICIAN,
   MODE_FIGHTER,
   MODE_THIEF,
   NUMMODES
};

// game mode names
const char *modenames[NUMMODES] =
{
   "Vampire Killer",
   "Shooter",
   "Magician",
   "Fighter",
   "Thief",
};

//
// savefile_t
//
// This struct stores both the raw data read from file and processed data
// that is easier to work with and display.
//
typedef struct savefile_s
{
   byte data[SAVEFILESIZE]; // raw data from the disk file
   bool exists;             // if true, this file is valid
   byte checksum;           // original checksum stored at offset 0x0009
   char name[9];            // converted file name
   long time;               // elapsed time in tics (60 Hz)
   long mode;               // 03/13/07: game mode being played
   long map_pct;            // 03/13/07: map percentage
   
   // haleyjd 03/14/07: the unpacked map
   byte map[MAP_WIDTH][MAP_HEIGHT];

   // player stats

   long  hp;                // hit points
   long  mp;                // magic points
   short hearts_current;    // current hearts
   short hearts_max;        // maximum hearts
   long  subweapon;         // subweapon type (0 - 5)
   short str[4];            // strength values
   short def[4];            // defense values
   short intel[4];          // int values
   short lck[4];            // luck values
   long  lv;                // level
   long  exp;               // total experience

   // current equip

   byte  attribute_card;    // attribute card selected
   byte  action_card;       // action card selected
   byte  armor;             // armor equipped
   byte  arm_first;         // first arm equip
   byte  arm_second;        // second arm equip

   // DSS crap

   bool  dss_owned[NUMDSS];      // owned DSS cards
   bool  dss_used[NUMABILITIES]; // used DSS abilities

   // Inventory

   byte  inventory[NUMINV]; // armor, arm equips, usable items
   byte  relics[NUMRELICS]; // dash, jump, tackle, kick, heavy, cleansing, key
   byte  numheartups;       // 03/16/07: number of heart ups collected
   byte  numhpups;          // ditto for HP ups
   byte  nummpups;          // ditto for MP ups

} savefile_t;

// the savefiles
savefile_t savefiles[NUMSAVEFILES];

//
// SaveFileError
//
// Prints an error message and exits
//
void SaveFileError(const char *str, ...)
{
   va_list va;

   va_start(va, str);

   vprintf(str, va);

   va_end(va);

   exit(1);
}

//
// SaveFileShort
//
// Constructs a short int from 2 consecutive bytes of the savefile starting
// at the provided offset. Save data is little endian, this code makes no
// assumptions about host endianness.
//
short SaveFileShort(savefile_t *sf, unsigned int offset)
{
   short ret = 0;

   ret = sf->data[offset];

   ret |= ((short)sf->data[offset + 1]) << 8;

   return ret;
}

//
// SaveFileLong
//
// Constructs a long int from 4 consecutive bytes of the savefile starting
// at the provided offset. Save data is little endian; this code makes no
// assumptions about host endianness.
//
long SaveFileLong(savefile_t *sf, unsigned int offset)
{
   long ret;

   ret = sf->data[offset];

   ret |= ((long)sf->data[offset + 1]) <<  8;
   ret |= ((long)sf->data[offset + 2]) << 16;
   ret |= ((long)sf->data[offset + 3]) << 24;

   return ret;
}

//
// CalculateChecksum
//
// Re-calculates the checksum for a savefile.
// Returns true if the new checksum is the same as the previous one
//
bool CalculateChecksum(savefile_t *file)
{
   byte checksum = 0;
   int i;

   // clear the old checksum byte first
   file->data[OFFSET_CHECKSUM] = 0;

   for(i = 0; i < SAVEFILESIZE; ++i)
      checksum += file->data[i];

   // store it in the data
   file->data[OFFSET_CHECKSUM] = checksum;

   return (checksum == file->checksum);
}

// 03/14/07: use a string to convert all font characters
const char font_convert_table[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ&.'-!*";

//
// ReadPlayerName
//
// Small routine to read out and convert the player name into ASCII
//
// 03/13/07: rewritten to support all characters in CotM name font
// (except 'Jr' which cannot be represented; I replace it with a ?)
//
void ReadPlayerName(savefile_t *sf)
{
   int i;
   byte c;

   for(i = OFFSET_NAME; i < OFFSET_NAME + NAME_LENGTH; ++i)
   {
      c = sf->data[i];

      if(c >= 0 && c <= 32)
         sf->name[i - OFFSET_NAME] = font_convert_table[c];
      else
         sf->name[i - OFFSET_NAME] = '?';
   }

   // 03/16/07: for beautification, strip spaces off the end
   for(i = NAME_LENGTH - 1; i >= 0; --i)
   {
      if(sf->name[i] != ' ')
         break;
      sf->name[i] = '\0';
   }
}

//
// ReadPlayerStats
//
// Reads the player's basic stats and current equipment.
//
void ReadPlayerStats(savefile_t *sf)
{
   sf->hp             = SaveFileLong(sf, OFFSET_HP1);
   sf->mp             = SaveFileLong(sf, OFFSET_MP1);
   sf->hearts_current = SaveFileShort(sf, OFFSET_HEARTS_CUR);
   sf->hearts_max     = SaveFileShort(sf, OFFSET_HEARTS_MAX);
   sf->subweapon      = SaveFileLong(sf, OFFSET_SUBWEAPON);
   sf->str[0]         = SaveFileShort(sf, OFFSET_STR_BASE);
   sf->str[1]         = SaveFileShort(sf, OFFSET_STR_EQUIP);
   sf->str[2]         = SaveFileShort(sf, OFFSET_STR_DSS);
   sf->str[3]         = SaveFileShort(sf, OFFSET_STR_UNKNOWN);
   sf->def[0]         = SaveFileShort(sf, OFFSET_DEF_BASE);
   sf->def[1]         = SaveFileShort(sf, OFFSET_DEF_EQUIP);
   sf->def[2]         = SaveFileShort(sf, OFFSET_DEF_DSS);
   sf->def[3]         = SaveFileShort(sf, OFFSET_DEF_UNKNOWN);
   sf->intel[0]       = SaveFileShort(sf, OFFSET_INT_BASE);
   sf->intel[1]       = SaveFileShort(sf, OFFSET_INT_EQUIP);
   sf->intel[2]       = SaveFileShort(sf, OFFSET_INT_DSS);
   sf->intel[3]       = SaveFileShort(sf, OFFSET_INT_UNKNOWN);
   sf->lck[0]         = SaveFileShort(sf, OFFSET_LCK_BASE);
   sf->lck[1]         = SaveFileShort(sf, OFFSET_LCK_EQUIP);
   sf->lck[2]         = SaveFileShort(sf, OFFSET_LCK_DSS);
   sf->lck[3]         = SaveFileShort(sf, OFFSET_LCK_UNKNOWN);
   sf->lv             = SaveFileLong(sf, OFFSET_LEVEL);
   sf->exp            = SaveFileLong(sf, OFFSET_EXP);
   sf->attribute_card = sf->data[OFFSET_EQUIP_ATTRIB];
   sf->action_card    = sf->data[OFFSET_EQUIP_ACTION];
   sf->armor          = sf->data[OFFSET_EQUIP_ARMOR];
   sf->arm_first      = sf->data[OFFSET_EQUIP_ARM1];
   sf->arm_second     = sf->data[OFFSET_EQUIP_ARM2];
   sf->numheartups    = sf->data[OFFSET_HEART_UP]; // 03/16/07: Up items
   sf->numhpups       = sf->data[OFFSET_HP_UP];
   sf->nummpups       = sf->data[OFFSET_MP_UP];

   // adjust the action card index by 10 (action cards come after attributes)
   if(sf->action_card)
      sf->action_card += 10;

   // 03/16/07: hack - adjust subweapon if it is the homing dagger
   if(sf->subweapon == SUBWEAPON_HOMINGDAGGER_FILEVAL)
      sf->subweapon = SUBWEAPON_HOMINGDAGGER;
}

//
// ReadDSS
//
// Reads out the DSS data
//
void ReadDSS(savefile_t *sf)
{
   int i;

   for(i = 1; i < NUMDSS; ++i)
      sf->dss_owned[i] = sf->data[OFFSET_CARDS + (i - 1)];

   for(i = 0; i < NUMABILITIES; ++i)
      sf->dss_used[i] = sf->data[OFFSET_ABILITIES + i];
}

//
// ReadInventory
//
// Reads out the inventory data
//
void ReadInventory(savefile_t *sf)
{
   int i;

   for(i = 0; i < NUMINV; ++i)
      sf->inventory[i] = sf->data[OFFSET_INVENTORY + i];
}

//
// ReadRelics
//
void ReadRelics(savefile_t *sf)
{
   int i;
   
   for(i = 0; i < NUMRELICS; ++i)
      sf->relics[i] = sf->data[OFFSET_RELICS + i];
}

//
// ReadMap
//
// haleyjd 03/14/07: Decompresses the map data into a byte array
//
void ReadMap(savefile_t *sf)
{
   int word, row;
   int offset = OFFSET_MAP;

   memset(sf->map, 0, MAP_WIDTH * MAP_HEIGHT);

   for(row = 0; row < MAP_HEIGHT; ++row)
   {
      for(word = 0; word < PACKED_MAP_WIDTH; ++word, ++offset)
      {
         int bit;

         for(bit = 0; bit < 8; ++bit)
         {
            sf->map[8 * word + bit][row] = 
               ((sf->data[offset] >> bit) & 1) ? '*' : ' ';
         }
      }
   }
}

//
// ReadSaveFiles
//
// Reads all the save files from the input file one by one.
//
void ReadSaveFiles(FILE *f)
{
   int i;
   savefile_t *sf;
   size_t c;
   bool found_file = false;

   // init everything to zero
   memset(savefiles, 0, NUMSAVEFILES * sizeof(savefile_t));

   // 03/13/07: read 16-byte file header first
   if(fread(fileheader, 1, sizeof(fileheader), f) != sizeof(fileheader))
      SaveFileError("Error: couldn't read 16-byte header\n");

   // verify header contents
   if(strncmp(fileheader, "DRACULA AGB", 11))
      SaveFileError("Error: this is not a valid CotM save RAM file!\n");

   for(i = 0; i < NUMSAVEFILES; ++i)
   {
      sf = &savefiles[i];

      // seek to position
      if(fseek(f, fileoffsets[i], SEEK_SET))
         SaveFileError("Error: couldn't seek to position for savefile %d\n", i);

      // read the raw save data
      if((c = fread(sf->data, 1, SAVEFILESIZE, f)) != SAVEFILESIZE)
      {
         SaveFileError("Error: read %d of %d bytes for file %d\n", 
                       c, SAVEFILESIZE, i);
      }

      // check if this file exists; 
      // if not, we have no more processing to do for this one.
      if(!(sf->exists = (sf->data[OFFSET_EXISTS] == EXISTS_YES)))
         continue;

      // 03/13/07: mark if we've found at least one valid file...
      found_file = true;

      // get & convert player name
      ReadPlayerName(sf);

      // set original checksum
      sf->checksum = sf->data[OFFSET_CHECKSUM];

      // get time
      sf->time = SaveFileLong(sf, OFFSET_TIME);

      // 03/13/07: get game mode
      sf->mode = SaveFileLong(sf, OFFSET_GAMEMODE);

      // 03/13/07: get map percentage
      sf->map_pct = SaveFileLong(sf, OFFSET_MAP_PCT);

      // 03/14/07: read map
      ReadMap(sf);

      // get stats
      ReadPlayerStats(sf);

      // get dss
      ReadDSS(sf);

      // get inventory
      ReadInventory(sf);

      // get relics
      ReadRelics(sf);
   }

   // 03/13/07: don't go on if all files are empty
   if(!found_file)
   {
      SaveFileError("Error: There must be at least one valid game in the "
                    "savefile.\n");
   }
}

// the current file the player has selected to view
int current_file;

//
// SelectFile
//
// Allows the user to pick a file which the main menu will use for all functions.
//
void SelectFile(void)
{
   int filenum;
   bool exitflag = false;
   char c, choice;

   while(!exitflag)
   {
      printf("\nChoose a file\n"
             "--------------------------------------------------\n"
             "1. %s\n"
             "2. %s\n"
             "3. %s\n"
             "4. %s\n"
             "5. %s\n"
             "6. %s\n"
             "7. %s\n"
             "8. %s\n\n",
             savefiles[0].exists ? savefiles[0].name : "no file",
             savefiles[1].exists ? savefiles[1].name : "no file",
             savefiles[2].exists ? savefiles[2].name : "no file",
             savefiles[3].exists ? savefiles[3].name : "no file",
             savefiles[4].exists ? savefiles[4].name : "no file",
             savefiles[5].exists ? savefiles[5].name : "no file",
             savefiles[6].exists ? savefiles[6].name : "no file",
             savefiles[7].exists ? savefiles[7].name : "no file");

      printf("Current file selected: #%d\n", current_file + 1);
      
      fflush(stdout);
      
      while((c = getchar()) != '\n')
         choice = c;
      
      switch(choice)
      {
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
         filenum = choice - '0' - 1;
         if(!savefiles[filenum].exists)
         {
            puts("This file doesn't exist, pick a different one.\n");
            break;
         }
         current_file = filenum;
         exitflag = true;
         break;
      default:
         puts("Bad choice, try again.\n");
         break;
      }
   }
}

//
// ViewStats
//
// Prints out all the basic stats for the current file.
//
void ViewStats(void)
{
   savefile_t *sf = &savefiles[current_file];
   char timestr[16];
   int s;

   // create time string
   memset(timestr, 0, sizeof(timestr));
   
   // CotM uses a 60 Hz timer
   s = sf->time / 60;
   sprintf(timestr, "%02d:%02d:%02d", s / 3600, (s % 3600) / 60, s % 60);

   // 03/12/07: found the correct sequence of subweapon names

   printf("\nFile %d: %s - Stats\n"
          "------------------------------------------------------------\n"
          "Game mode: %s\n"
          "Elapsed time: %s\n"
          "Map coverage: %.1f%%\n"
          "LV: %d  EXP: %d\n"
          "HP: %d  MP: %d  Hearts: %d/%d  Subweapon: %s\n\n"
          "Base Stats:  STR = %4d, DEF = %4d, INT = %4d, LCK = %4d\n"
          "Equip Stats: STR = %4d, DEF = %4d, INT = %4d, LCK = %4d\n"
          "DSS Stats:   STR = %4d, DEF = %4d, INT = %4d, LCK = %4d\n"
          "???? Stats:  STR = %4d, DEF = %4d, INT = %4d, LCK = %4d\n\n",
          current_file + 1, sf->name,
          modenames[sf->mode],
          timestr,
          ((float)sf->map_pct) / 10.0f,
          sf->lv, sf->exp,
          sf->hp, sf->mp, sf->hearts_current, sf->hearts_max, 
          subweapons[sf->subweapon],
          sf->str[0], sf->def[0], sf->intel[0], sf->lck[0],
          sf->str[1], sf->def[1], sf->intel[1], sf->lck[1],
          sf->str[2], sf->def[2], sf->intel[2], sf->lck[2],
          sf->str[3], sf->def[3], sf->intel[3], sf->lck[3]);

   fflush(stdout);
   
   puts("Press enter to return.");
   while(getchar() != '\n');
}

//
// ViewEquip
//
// Prints out currently equipped items for the current file.
//
void ViewEquip(void)
{
   savefile_t *sf = &savefiles[current_file];
   inventoryitem_t *armor = &inventory_items[sf->armor];
   inventoryitem_t *arm1  = &inventory_items[sf->arm_first];
   inventoryitem_t *arm2  = &inventory_items[sf->arm_second];

   printf("\nFile %d: %s - Current Equipment\n"
          "------------------------------------------------------------\n"
          "Action Card    = %s\n"
          "\"%s\"\n"
          "Attribute Card = %s\n"
          "\"%s\"\n"
          "Armor: %s\n"
          "* %s\n"
          "* STR: %+4d, DEF: %+4d, INT: %+4d, LCK: %+4d, Rarity: %d\n"
          "Arm 1: %s\n"
          "* %s\n"
          "* STR: %+4d, DEF: %+4d, INT: %+4d, LCK: %+4d, Rarity: %d\n"
          "Arm 2: %s\n"
          "* %s\n"
          "* STR: %+4d, DEF: %+4d, INT: %+4d, LCK: %+4d, Rarity: %d\n\n",
          current_file + 1, sf->name,
          dsscards[sf->action_card].name, 
          dsscards[sf->action_card].description,
          dsscards[sf->attribute_card].name,
          dsscards[sf->attribute_card].description,
          armor->name, armor->description,
          armor->atk, armor->def, armor->intel, armor->lck, armor->rarity,
          arm1->name, arm1->description,
          arm1->atk, arm1->def, arm1->intel, arm1->lck, arm1->rarity,
          arm2->name, arm2->description,
          arm2->atk, arm2->def, arm2->intel, arm2->lck, arm2->rarity);

   fflush(stdout);
   
   puts("Press enter to return.");
   while(getchar() != '\n');
}

//
// ViewDSS
//
// Displays the player's DSS card stock and allows viewing of generic info
// on all the cards.
//
void ViewDSS(void)
{
   bool exitflag = false;
   char c, choice;
   savefile_t *sf = &savefiles[current_file];

   while(!exitflag)
   {
      printf("\nFile %d: %s - Owned DSS Cards\n"
             "------------------------------------------------------------\n"
             "1. Mercury: %c      B. Salamander:  %c\n"
             "2. Venus:   %c      C. Serpent:     %c\n"
             "3. Jupiter: %c      D. Mandragora:  %c\n"
             "4. Mars:    %c      E. Golem:       %c\n"
             "5. Diana:   %c      F. Cockatrice:  %c\n" 
             "6. Apollo:  %c      G. Manticore:   %c\n"
             "7. Neptune: %c      H. Griffin:     %c\n"
             "8. Saturn:  %c      I. Thunderbird: %c\n"
             "9. Uranus:  %c      J. Unicorn:     %c\n"
             "A. Pluto:   %c      K. Black Dog:   %c\n\n",
             current_file + 1, sf->name,
             sf->dss_owned[CARD_MERCURY]     ? 'X' : ' ',
             sf->dss_owned[CARD_SALAMANDER]  ? 'X' : ' ',
             sf->dss_owned[CARD_VENUS]       ? 'X' : ' ',
             sf->dss_owned[CARD_SERPENT]     ? 'X' : ' ',
             sf->dss_owned[CARD_JUPITER]     ? 'X' : ' ',
             sf->dss_owned[CARD_MANDRAGORA]  ? 'X' : ' ',
             sf->dss_owned[CARD_MARS]        ? 'X' : ' ',
             sf->dss_owned[CARD_GOLEM]       ? 'X' : ' ',
             sf->dss_owned[CARD_DIANA]       ? 'X' : ' ',
             sf->dss_owned[CARD_COCKATRICE]  ? 'X' : ' ',
             sf->dss_owned[CARD_APOLLO]      ? 'X' : ' ',
             sf->dss_owned[CARD_MANTICORE]   ? 'X' : ' ',
             sf->dss_owned[CARD_NEPTUNE]     ? 'X' : ' ',
             sf->dss_owned[CARD_GRIFFIN]     ? 'X' : ' ',
             sf->dss_owned[CARD_SATURN]      ? 'X' : ' ',
             sf->dss_owned[CARD_THUNDERBIRD] ? 'X' : ' ',
             sf->dss_owned[CARD_URANUS]      ? 'X' : ' ',
             sf->dss_owned[CARD_UNICORN]     ? 'X' : ' ',
             sf->dss_owned[CARD_PLUTO]       ? 'X' : ' ',
             sf->dss_owned[CARD_BLACKDOG]    ? 'X' : ' ');

      fflush(stdout);
      
      puts("Enter a character for more info or press enter to return.");
      while((c = getchar()) != '\n')
         choice = c;

      choice = toupper(choice);
      
      if(choice >= '1' && choice <= '9' || choice == 'A')
      {
         int cardnum;

         if(choice >= '1' && choice <= '9')
            cardnum = choice - '0' + 10;
         else
            cardnum = 20;

         printf("\nAction Card - %s\n"
                "------------------------------------------------------------\n"
                "%s\n\n", 
                dsscards[cardnum].name, dsscards[cardnum].description);

         fflush(stdout);
         
         puts("Press enter to return.");
         while(getchar() != '\n');
      }
      else if(choice >= 'B' && choice <= 'K')
      {
         int cardnum = choice - 'A';

         printf("\nAttribute Card - %s\n"
                "------------------------------------------------------------\n"
                "%s\n\n",
                dsscards[cardnum].name, dsscards[cardnum].description);

         fflush(stdout);
         
         puts("Press enter to return.");
         while(getchar() != '\n');
      }
      else
         exitflag = true;

      choice = 0;
   }
}

//
// ViewInventoryRange
//
// This gets pretty hairy. Automatically generates a menu for all the
// inventory items within the indicated range inclusive.
//
void ViewInventoryRange(const char *rangename, int minitem, int maxitem)
{
   savefile_t *sf = &savefiles[current_file];
   char c, choice = 0;
   bool exitflag = false;
   int i, invnum;

   while(!exitflag)
   {
      printf("\nFile %d: %s - %s\n"
             "--------------------------------------------------\n",
             current_file + 1, sf->name, rangename);

      for(i = minitem; i <= maxitem; ++i)
      {
         int menuidx = i - minitem + 1;

         printf("%c. %-17s [%2d]\n",
                menuidx > 9 ? 'A' + (menuidx - 10) : '0' + menuidx,
                inventory_items[i].name,
                sf->inventory[i]);
      }
      putchar('\n');

      fflush(stdout);
      
      puts("Select an inventory item or press enter to return.");
      while((c = getchar()) != '\n')
         choice = c;

      choice = toupper(choice);

      invnum = 
         (choice >= '1' && choice <= '9') ? minitem + choice - '1' :
         (choice >= 'A' && choice <= 'Z') ? minitem + choice - 'A' + 9 :
         ((exitflag = true), 0);

      if(invnum > maxitem)
      {
         exitflag = true;
         invnum = 0;
      }

      if(invnum != 0)
      {
         printf("\nItem - %s [%2d]\n"
                "------------------------------------------------------------\n"
                "Description: %s\n"
                "STR: %+4d, DEF: %+4d, INT: %+4d, LCK: %+4d, Rarity: %d\n\n",
                inventory_items[invnum].name,
                sf->inventory[invnum],
                inventory_items[invnum].description,
                inventory_items[invnum].atk,
                inventory_items[invnum].def,
                inventory_items[invnum].intel,
                inventory_items[invnum].lck,
                inventory_items[invnum].rarity);

         fflush(stdout);
         
         puts("Press enter to return.");
         while(getchar() != '\n');
      }

      invnum = 0;
      choice = 0;
   }
}

//
// ViewRelics
//
// Displays the player's magic items (aka relics in most CV games; I use this
// term to disambiguate them from everything else that's magical, which is
// pretty much everything in the game).
//
void ViewRelics(void)
{
   savefile_t *sf = &savefiles[current_file];
   int i;

   printf("\nFile %d: %s - Relics\n"
          "------------------------------------------------------------\n",
          current_file + 1, sf->name);
   
   for(i = 0; i < NUMRELICS; ++i)
   {
      printf("%d. %-11s [%c]\n   %s\n",
             i+1, relics[i].name, sf->relics[i] ? 'X' : ' ', 
             relics[i].description);
   }
   putchar('\n');

   fflush(stdout);
   
   puts("Press enter to return.");
   while(getchar() != '\n');
}

//
// ViewUps
//
// 03/16/07: Displays information on the player's collected Heart, HP, and MP
// up items.
//
void ViewUps(void)
{
   savefile_t *sf = &savefiles[current_file];

   printf("\nFile %d: %s - Max Increase Items\n"
          "------------------------------------------------------------\n"
          "Heart Max Increase: %d\n"
          "HP Max Increase:    %d\n"
          "MP Max Increase:    %d\n\n",
          current_file + 1, sf->name,
          sf->numheartups, sf->numhpups, sf->nummpups);

   fflush(stdout);
   
   puts("Press enter to return.");
   while(getchar() != '\n');
}

//
// ViewInventory
//
// Routine to display various subsets of the player's inventory.
//
void ViewInventory(void)
{
   savefile_t *sf = &savefiles[current_file];
   char c, choice;
   bool exitflag = false;

   while(!exitflag)
   {
      printf("\nFile %d: %s - Inventory\n"
             "--------------------------------------------------\n"
             "1. View Armor\n"
             "2. View Robes\n"
             "3. View Clothes\n"
             "4. View Armbands\n"
             "5. View Paired Armbands\n"
             "6. View Rings\n"
             "7. View Odd Rings\n"
             "8. View Usable Items\n"
             "9. View Relics\n"
             "A. View Max Increase Items\n\n",
             current_file + 1, sf->name);

      fflush(stdout);
      
      puts("Select an inventory class or press enter to return.");
      while((c = getchar()) != '\n')
         choice = c;
      
      choice = toupper(choice);

      switch(choice)
      {
      case '1':
         ViewInventoryRange("Armor", INV_LEATHER_ARMOR, INV_SHINING_ARMOR);
         break;
      case '2':
         ViewInventoryRange("Robes", INV_COTTON_ROBE, INV_SAGE_ROBE);
         break;
      case '3':
         ViewInventoryRange("Clothes", INV_COTTON_CLOTHES, INV_SOLDIER_FATIGUES);
         break;
      case '4':
         ViewInventoryRange("Armbands", INV_STRENGTH_ARMBAND, INV_MIRACLE_ARMBAND);
         break;
      case '5':
         ViewInventoryRange("Paired Armbands", INV_DOUBLE_GRIPS, INV_STAR_BRACELET);
         break;
      case '6':
         ViewInventoryRange("Rings", INV_STRENGTH_RING, INV_CURSED_RING);
         break;
      case '7':
         ViewInventoryRange("Odd Rings", INV_TOY_RING, INV_BEAR_RING);
         break;
      case '8':
         ViewInventoryRange("Usable Items", INV_POTION, INV_HEART_MEGA);
         break;
      case '9':
         ViewRelics();
         break;
      case 'A':
         ViewUps();
         break;
      default:
         exitflag = true;
         break;
      }

      choice = 0;
   }
}

//
// ViewMap
//
// 03/14/07: Ability to view map, although it's not too pretty in text mode :)
//
void ViewMap(void)
{
   savefile_t *sf = &savefiles[current_file];
   int block, row;

   for(row = 0; row < MAP_HEIGHT; ++row)
   {
      for(block = 0; block < MAP_WIDTH; ++block)
         putchar(sf->map[block][row]);

      putchar('\n');
   }

   fflush(stdout);
   
   puts("\nPress enter to return.");
   while(getchar() != '\n');
}

void ViewChecksum(void)
{
   savefile_t *sf = &savefiles[current_file];
   bool match;

   match = CalculateChecksum(sf);

   if(!match)
      printf("\nWarning! The calculated value %d doesn't match the original "
             "checksum value of %d. Using this file will erase your save "
             "games!\n\n", sf->data[OFFSET_CHECKSUM], sf->checksum);
   else
      printf("\nThe file checksum value is %d.\n\n", sf->data[OFFSET_CHECKSUM]);

   fflush(stdout);
   
   puts("Press enter to return.");
   while(getchar() != '\n');
}

void MainMenu(void)
{
   char c, choice;
   bool exitflag = false;

   while(!exitflag)
   {
      puts("\nCastlevania: Circle of the Moon Save RAM Inspector\n"
           "--------------------------------------------------\n"
           "1. Select file\n"
           "2. View stats\n"
           "3. View current equipment\n"
           "4. View DSS\n"
           "5. View inventory\n"
           "6. View map\n"
           "7. Recalculate file checksum\n"
           "8. Exit\n");

      printf("Current file selected: #%d\n", current_file + 1);
      
      fflush(stdout);
      
      while((c = getchar()) != '\n')
         choice = c;
      
      switch(choice)
      {
      case '1':
         SelectFile();
         break;
      case '2':
         ViewStats();
         break;
      case '3':
         ViewEquip();
         break;
      case '4':
         ViewDSS();
         break;
      case '5':
         ViewInventory();
         break;
      case '6':
         ViewMap();
         break;
      case '7':
         ViewChecksum();
         break;
      case '8':
         puts("Bye!\n");
         exitflag = true;
         break;
      default:
         puts("Bad choice, try again.\n\n");
         break;
      }

      choice = 0;
   }
}

//
// Main Program
//
// Opens the input file, creates the savefile_t structures from it, and runs the
// menu loop.
//
int main(int argc, char *argv[])
{
   FILE *f;

   if(argc >= 2)
   {
      if((f = fopen(argv[1], "rb")))
      {
         ReadSaveFiles(f);

         fclose(f); // done with physical file

         MainMenu();
      }
      else
         puts("Error: couldn't open the indicated input file.\n");
   }
   else
      puts("I need a file name, doofus!\n");

   return 0;
}

/*
int main(int argc, char *argv[])
{
   FILE *f;
   unsigned char c[976];
   unsigned char checksum = 0;
   int i;

   if(argc >= 2)
   {
      if((f = fopen(argv[1], "rb")))
      {
         fseek(f, 0x10, SEEK_SET);

         fread(c, 1, 976, f);

         c[9] = 0; // zero existing checksum

         for(i = 0; i < 976; ++i)
            checksum += c[i];

         printf("The checksum byte should be %d\n", checksum);

         fclose(f);
      }
      else
         puts("Error: couldn't open the indicated input file.\n");
   }
   else
      puts("I need a file name, doofus!\n");

   return 0;
}
*/