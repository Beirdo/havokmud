#include "config.h"
#include "environment.h"
#include "platform.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <arpa/telnet.h>
#include <stdlib.h>
#include <unistd.h>

#include "protos.h"

/*
 * this is how we tell which race gets which class !
 * to add a new class seletion add the CLASS_NAME above the
 * message 'NEW CLASS SELETIONS HERE'
 */

const int       avariel_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_CLERIC + CLASS_WARRIOR,
    CLASS_MAGIC_USER + CLASS_WARRIOR,
    CLASS_MAGIC_USER + CLASS_CLERIC,
    CLASS_MAGIC_USER + CLASS_CLERIC + CLASS_WARRIOR
};

const int       moon_elf_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_DRUID,
    CLASS_MONK,
    CLASS_PALADIN,
    CLASS_RANGER,
    CLASS_PSI,
    CLASS_WARRIOR + CLASS_MAGIC_USER,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_WARRIOR + CLASS_THIEF
};

const int       gold_elf_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_DRUID,
    CLASS_MONK,
    CLASS_PALADIN,
    CLASS_RANGER,
    CLASS_PSI,
    CLASS_WARRIOR + CLASS_MAGIC_USER,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_WARRIOR + CLASS_THIEF
};

const int       sea_elf_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_DRUID,
    CLASS_MONK,
    CLASS_PALADIN,
    CLASS_RANGER,
    CLASS_PSI,
    CLASS_WARRIOR + CLASS_MAGIC_USER,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_WARRIOR + CLASS_THIEF
};

const int       wild_elf_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_DRUID,
    CLASS_RANGER,
    CLASS_PSI,
    CLASS_WARRIOR + CLASS_DRUID,
    CLASS_WARRIOR + CLASS_MAGIC_USER,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_WARRIOR + CLASS_THIEF
};

const int       dwarf_class_choice[] = {
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_PALADIN,
    CLASS_RANGER,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_WARRIOR + CLASS_CLERIC
};

const int       halfling_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_THIEF + CLASS_WARRIOR,
    CLASS_DRUID,
    CLASS_MONK
};

const int       rock_gnome_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_WARRIOR + CLASS_MAGIC_USER,
    CLASS_MAGIC_USER + CLASS_THIEF,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_CLERIC
};

const int       forest_gnome_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_RANGER,
    CLASS_DRUID,
    CLASS_RANGER + CLASS_DRUID,
    CLASS_CLERIC + CLASS_THIEF,
    CLASS_WARRIOR + CLASS_THIEF
};

const int       human_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_DRUID,
    CLASS_MONK,
    CLASS_BARBARIAN,
    CLASS_PALADIN,
    CLASS_RANGER,
    CLASS_PSI,
    CLASS_NECROMANCER,
    CLASS_WARRIOR + CLASS_DRUID,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_WARRIOR + CLASS_MAGIC_USER,
    CLASS_WARRIOR + CLASS_CLERIC,
    CLASS_NECROMANCER + CLASS_WARRIOR,
    CLASS_WARRIOR + CLASS_CLERIC + CLASS_MAGIC_USER,
    CLASS_THIEF + CLASS_CLERIC + CLASS_MAGIC_USER
};

const int       half_elf_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_DRUID,
    CLASS_MONK,
    CLASS_PALADIN,
    CLASS_RANGER,
    CLASS_PSI,
    CLASS_WARRIOR + CLASS_MAGIC_USER,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_WARRIOR + CLASS_DRUID,
    CLASS_MAGIC_USER + CLASS_THIEF,
    CLASS_WARRIOR + CLASS_MAGIC_USER + CLASS_THIEF,
    CLASS_CLERIC + CLASS_MAGIC_USER,
    CLASS_WARRIOR + CLASS_MAGIC_USER + CLASS_CLERIC,
    CLASS_WARRIOR + CLASS_CLERIC
};

const int       half_orc_class_choice[] = {
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_CLERIC + CLASS_THIEF,
    CLASS_WARRIOR + CLASS_CLERIC + CLASS_THIEF
};

const int       half_ogre_class_choice[] = {
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_CLERIC + CLASS_WARRIOR
};

const int       half_giant_class_choice[] = {
    CLASS_WARRIOR,
    CLASS_BARBARIAN
};

const int       orc_class_choice[] = {
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_CLERIC,
    CLASS_BARBARIAN,
    CLASS_THIEF + CLASS_CLERIC
};

const int       goblin_class_choice[] = {
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_CLERIC,
    CLASS_BARBARIAN,
    CLASS_WARRIOR + CLASS_THIEF
};

const int       troll_class_choice[] = {
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_CLERIC,
    CLASS_BARBARIAN,
    CLASS_WARRIOR + CLASS_CLERIC
};

const int       default_class_choice[] = {
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_CLERIC,
    CLASS_MAGIC_USER,
    CLASS_BARBARIAN,
    CLASS_PALADIN,
    CLASS_RANGER,
    CLASS_PSI
};

const int       dark_dwarf_class_choice[] = {
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_WARRIOR + CLASS_CLERIC,
    CLASS_NECROMANCER + CLASS_WARRIOR
};

const int       drow_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_DRUID,
    CLASS_MONK,
    CLASS_PSI,
    CLASS_WARRIOR + CLASS_MAGIC_USER,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_THIEF,
    CLASS_CLERIC + CLASS_MAGIC_USER,
    CLASS_NECROMANCER + CLASS_WARRIOR,
    CLASS_MAGIC_USER + CLASS_WARRIOR + CLASS_THIEF
};

/*
 * these are the allowable PC races
 */
const struct pc_race_choice race_choice[] = {
    { RACE_DWARF, dwarf_class_choice, NELEMENTS(dwarf_class_choice) },
    { RACE_MOON_ELF, moon_elf_class_choice, NELEMENTS(moon_elf_class_choice) },
    { RACE_GOLD_ELF, gold_elf_class_choice, NELEMENTS(gold_elf_class_choice) },
    { RACE_WILD_ELF, wild_elf_class_choice, NELEMENTS(wild_elf_class_choice) },
    { RACE_AVARIEL, avariel_class_choice, NELEMENTS(avariel_class_choice) },
    { RACE_HUMAN, human_class_choice, NELEMENTS(human_class_choice) },
    { RACE_ROCK_GNOME, rock_gnome_class_choice, 
      NELEMENTS(rock_gnome_class_choice) },
    { RACE_FOREST_GNOME, forest_gnome_class_choice, 
      NELEMENTS(forest_gnome_class_choice) },
    { RACE_HALFLING, halfling_class_choice, NELEMENTS(halfling_class_choice) },
    { RACE_HALF_ELF, half_elf_class_choice, NELEMENTS(half_elf_class_choice) },
    { RACE_HALF_ORC, half_orc_class_choice, NELEMENTS(half_orc_class_choice) },
    { RACE_HALF_OGRE, half_ogre_class_choice, NELEMENTS(half_ogre_class_choice) },
    { RACE_HALF_GIANT, half_giant_class_choice, 
      NELEMENTS(half_giant_class_choice) },
    /* Put all races with racial hatreds after here, DROW *must* be first */
    { RACE_DROW, drow_class_choice, NELEMENTS(drow_class_choice) },
    { RACE_ORC, orc_class_choice, NELEMENTS(orc_class_choice) },
    { RACE_GOBLIN, goblin_class_choice, NELEMENTS(goblin_class_choice) },
    { RACE_TROLL, troll_class_choice, NELEMENTS(troll_class_choice) },
    { RACE_DARK_DWARF, dark_dwarf_class_choice, 
      NELEMENTS(dark_dwarf_class_choice) }
};

const int race_choice_count = NELEMENTS(race_choice);

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
