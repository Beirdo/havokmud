
int IsHumanoid(struct char_data *ch)
{
    /*
     * these are all very arbitrary
     */
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_HUMAN:
    case RACE_ROCK_GNOME:
    case RACE_DEEP_GNOME:
    case RACE_FOREST_GNOME:
    case RACE_MOON_ELF:
    case RACE_GOLD_ELF:
    case RACE_WILD_ELF:
    case RACE_SEA_ELF:
    case RACE_AVARIEL:
    case RACE_DWARF:
    case RACE_DARK_DWARF:
    case RACE_HALFLING:
    case RACE_ORC:
    case RACE_LYCANTH:
    case RACE_UNDEAD:
    case RACE_UNDEAD_VAMPIRE:
    case RACE_UNDEAD_LICH:
    case RACE_UNDEAD_WIGHT:
    case RACE_UNDEAD_GHAST:
    case RACE_UNDEAD_SPECTRE:
    case RACE_UNDEAD_ZOMBIE:
    case RACE_UNDEAD_SKELETON:
    case RACE_UNDEAD_GHOUL:
    case RACE_GIANT:
    case RACE_GIANT_HILL:
    case RACE_GIANT_FROST:
    case RACE_GIANT_FIRE:
    case RACE_GIANT_CLOUD:
    case RACE_GIANT_STORM:
    case RACE_GIANT_STONE:
    case RACE_GOBLIN:
    case RACE_DEVIL:
    case RACE_TROLL:
    case RACE_VEGMAN:
    case RACE_MFLAYER:
    case RACE_ENFAN:
    case RACE_PATRYN:
    case RACE_SARTAN:
    case RACE_ROO:
    case RACE_SMURF:
    case RACE_TROGMAN:
    case RACE_LIZARDMAN:
    case RACE_SKEXIE:
    case RACE_TYTAN:
    case RACE_DROW:
    case RACE_GOLEM:
    case RACE_DEMON:
    case RACE_DRAAGDIM:
    case RACE_ASTRAL:
    case RACE_GOD:
    case RACE_HALF_ELF:
    case RACE_HALF_ORC:
    case RACE_HALF_OGRE:
    case RACE_HALF_GIANT:
    case RACE_GNOLL:
        return (TRUE);
        break;

    default:
        return (FALSE);
        break;
    }
}

int HasWings(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_BIRD:
    case RACE_AVARIEL:
    case RACE_DRAGON_RED:
    case RACE_DRAGON_BLACK:
    case RACE_DRAGON_GREEN:
    case RACE_DRAGON_WHITE:
    case RACE_DRAGON_BLUE:
    case RACE_DRAGON_SILVER:
    case RACE_DRAGON_GOLD:
    case RACE_DRAGON_BRONZE:
    case RACE_DRAGON_COPPER:
    case RACE_DRAGON_BRASS:
    case RACE_DRAGON_AMETHYST:
    case RACE_DRAGON_CRYSTAL:
    case RACE_DRAGON_EMERALD:
    case RACE_DRAGON_SAPPHIRE:
    case RACE_DRAGON_TOPAZ:
    case RACE_DRAGON_BROWN:
    case RACE_DRAGON_CLOUD:
    case RACE_DRAGON_DEEP:
    case RACE_DRAGON_MERCURY:
    case RACE_DRAGON_MIST:
    case RACE_DRAGON_SHADOW:
    case RACE_DRAGON_STEEL:
    case RACE_DRAGON_YELLOW:
        return (TRUE);
        break;
    default:
        return (FALSE);
    }
}

int IsRideable(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    if (IS_NPC(ch) && !IS_PC(ch)) {
        switch (GET_RACE(ch)) {
        case RACE_HORSE:
        case RACE_DRAGON:
        case RACE_DRAGON_RED:
        case RACE_DRAGON_BLACK:
        case RACE_DRAGON_GREEN:
        case RACE_DRAGON_WHITE:
        case RACE_DRAGON_BLUE:
        case RACE_DRAGON_SILVER:
        case RACE_DRAGON_GOLD:
        case RACE_DRAGON_BRONZE:
        case RACE_DRAGON_COPPER:
        case RACE_DRAGON_BRASS:
        case RACE_DRAGON_AMETHYST:
        case RACE_DRAGON_CRYSTAL:
        case RACE_DRAGON_EMERALD:
        case RACE_DRAGON_SAPPHIRE:
        case RACE_DRAGON_TOPAZ:
        case RACE_DRAGON_BROWN:
        case RACE_DRAGON_CLOUD:
        case RACE_DRAGON_DEEP:
        case RACE_DRAGON_MERCURY:
        case RACE_DRAGON_MIST:
        case RACE_DRAGON_SHADOW:
        case RACE_DRAGON_STEEL:
        case RACE_DRAGON_YELLOW:
        case RACE_DRAGON_TURTLE:
            return (TRUE);
            break;
        default:
            return (FALSE);
            break;
        }
    }
    return (FALSE);
}

int IsAnimal(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_PREDATOR:
    case RACE_FISH:
    case RACE_BIRD:
    case RACE_HERBIV:
    case RACE_REPTILE:
    case RACE_LABRAT:
    case RACE_ROO:
    case RACE_INSECT:
    case RACE_ARACHNID:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}

int IsVeggie(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_PARASITE:
    case RACE_SLIME:
    case RACE_TREE:
    case RACE_VEGGIE:
    case RACE_VEGMAN:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}

int IsUndead(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_UNDEAD:
    case RACE_GHOST:
    case RACE_UNDEAD_VAMPIRE:
    case RACE_UNDEAD_LICH:
    case RACE_UNDEAD_WIGHT:
    case RACE_UNDEAD_GHAST:
    case RACE_UNDEAD_SPECTRE:
    case RACE_UNDEAD_ZOMBIE:
    case RACE_UNDEAD_SKELETON:
    case RACE_UNDEAD_GHOUL:

        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}

int IsLycanthrope(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_LYCANTH:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}

int IsDiabolic(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_DEMON:
    case RACE_DEVIL:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}

int IsReptile(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_REPTILE:
    case RACE_DRAGON:
    case RACE_DRAGON_RED:
    case RACE_DRAGON_BLACK:
    case RACE_DRAGON_GREEN:
    case RACE_DRAGON_WHITE:
    case RACE_DRAGON_BLUE:
    case RACE_DRAGON_SILVER:
    case RACE_DRAGON_GOLD:
    case RACE_DRAGON_BRONZE:
    case RACE_DRAGON_COPPER:
    case RACE_DRAGON_BRASS:
    case RACE_DRAGON_AMETHYST:
    case RACE_DRAGON_CRYSTAL:
    case RACE_DRAGON_EMERALD:
    case RACE_DRAGON_SAPPHIRE:
    case RACE_DRAGON_TOPAZ:
    case RACE_DRAGON_BROWN:
    case RACE_DRAGON_CLOUD:
    case RACE_DRAGON_DEEP:
    case RACE_DRAGON_MERCURY:
    case RACE_DRAGON_MIST:
    case RACE_DRAGON_SHADOW:
    case RACE_DRAGON_STEEL:
    case RACE_DRAGON_YELLOW:
    case RACE_DRAGON_TURTLE:
    case RACE_DINOSAUR:
    case RACE_SNAKE:
    case RACE_TROGMAN:
    case RACE_LIZARDMAN:
    case RACE_SKEXIE:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}

int HasHands(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    if (IsHumanoid(ch)) {
        return (TRUE);
    }
    if (IsUndead(ch)) {
        return (TRUE);
    }
    if (IsLycanthrope(ch)) {
        return (TRUE);
    }
    if (IsDiabolic(ch)) {
        return (TRUE);
    }
    if (GET_RACE(ch) == RACE_GOLEM || GET_RACE(ch) == RACE_SPECIAL) {
        return (TRUE);
    }
    return (FALSE);
}

int IsPerson(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_HUMAN:
    case RACE_MOON_ELF:
    case RACE_DROW:
    case RACE_DWARF:
    case RACE_DARK_DWARF:
    case RACE_HALFLING:
    case RACE_ROCK_GNOME:
    case RACE_DEEP_GNOME:
    case RACE_FOREST_GNOME:
    case RACE_GOLD_ELF:
    case RACE_WILD_ELF:
    case RACE_SEA_ELF:
    case RACE_GOBLIN:
    case RACE_ORC:
    case RACE_AVARIEL:
        return (TRUE);
        break;

    default:
        return (FALSE);
        break;

    }
}

int IsGiantish(struct char_data *ch)
{

    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_ENFAN:

    case RACE_GOBLIN:           /* giantish for con's only... */
    case RACE_ORC:

    case RACE_GIANT:
    case RACE_GIANT_HILL:
    case RACE_GIANT_FROST:
    case RACE_GIANT_FIRE:
    case RACE_GIANT_CLOUD:
    case RACE_GIANT_STORM:
    case RACE_GIANT_STONE:
    case RACE_TYTAN:
    case RACE_TROLL:
    case RACE_DRAAGDIM:

    case RACE_HALF_ORC:
    case RACE_HALF_OGRE:
    case RACE_HALF_GIANT:
        return (TRUE);
    default:
        return (FALSE);
        break;
    }
}

int IsSmall(struct char_data *ch)
{

    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_SMURF:
    case RACE_ROCK_GNOME:
    case RACE_HALFLING:
    case RACE_GOBLIN:
    case RACE_ENFAN:
    case RACE_DEEP_GNOME:
    case RACE_FOREST_GNOME:
        return (TRUE);
    default:
        return (FALSE);
    }
}

int IsGiant(struct char_data *ch)
{

    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_GIANT:
    case RACE_GIANT_HILL:
    case RACE_GIANT_FROST:
    case RACE_GIANT_FIRE:
    case RACE_GIANT_CLOUD:
    case RACE_GIANT_STORM:
    case RACE_GIANT_STONE:
    case RACE_HALF_GIANT:
    case RACE_TYTAN:
    case RACE_GOD:
        return (TRUE);
    default:
        return (FALSE);
    }
}

int IsExtraPlanar(struct char_data *ch)
{

    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_DEMON:
    case RACE_DEVIL:
    case RACE_PLANAR:
    case RACE_ELEMENT:
    case RACE_ASTRAL:
    case RACE_GOD:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}
int IsOther(struct char_data *ch)
{

    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_MFLAYER:
    case RACE_SPECIAL:
    case RACE_GOLEM:
    case RACE_ELEMENT:
    case RACE_PLANAR:
    case RACE_LYCANTH:
        return (TRUE);
    default:
        return (FALSE);
        break;
    }
}

int IsDarkrace(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_DROW:
    case RACE_DARK_DWARF:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}

int IsGodly(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    if (GET_RACE(ch) == RACE_GOD) {
        return (TRUE);
    }
    if (GET_RACE(ch) == RACE_DEMON || GET_RACE(ch) == RACE_DEVIL) {
        if (GetMaxLevel(ch) >= 45) {
            return (TRUE);
        }
    }
    return( FALSE );
}

int IsDragon(struct char_data *ch)
{

    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_DRAGON:
    case RACE_DRAGON_RED:
    case RACE_DRAGON_BLACK:
    case RACE_DRAGON_GREEN:
    case RACE_DRAGON_WHITE:
    case RACE_DRAGON_BLUE:
    case RACE_DRAGON_SILVER:
    case RACE_DRAGON_GOLD:
    case RACE_DRAGON_BRONZE:
    case RACE_DRAGON_COPPER:
    case RACE_DRAGON_BRASS:
    case RACE_DRAGON_AMETHYST:
    case RACE_DRAGON_CRYSTAL:
    case RACE_DRAGON_EMERALD:
    case RACE_DRAGON_SAPPHIRE:
    case RACE_DRAGON_TOPAZ:
    case RACE_DRAGON_BROWN:
    case RACE_DRAGON_CLOUD:
    case RACE_DRAGON_DEEP:
    case RACE_DRAGON_MERCURY:
    case RACE_DRAGON_MIST:
    case RACE_DRAGON_SHADOW:
    case RACE_DRAGON_STEEL:
    case RACE_DRAGON_YELLOW:
    case RACE_DRAGON_TURTLE:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}

void SetRacialStuff(struct char_data *mob)
{
    switch (GET_RACE(mob)) {
    case RACE_BIRD:
        SET_BIT(mob->specials.affected_by, AFF_FLYING);
        break;
    case RACE_FISH:
        SET_BIT(mob->specials.affected_by, AFF_WATERBREATH);
        break;
    case RACE_SEA_ELF:
        SET_BIT(mob->specials.affected_by, AFF_WATERBREATH);
        SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
        SET_BIT(mob->immune, IMM_CHARM);
    case RACE_MOON_ELF:
    case RACE_DROW:
    case RACE_GOLD_ELF:
    case RACE_WILD_ELF:
    case RACE_AVARIEL:
        SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
        SET_BIT(mob->immune, IMM_CHARM);
        break;
    case RACE_DWARF:
    case RACE_DARK_DWARF:
    case RACE_DEEP_GNOME:
    case RACE_ROCK_GNOME:
    case RACE_FOREST_GNOME:
    case RACE_MFLAYER:
    case RACE_TROLL:
    case RACE_ORC:
    case RACE_GOBLIN:
    case RACE_HALFLING:
    case RACE_GNOLL:
        SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
        break;
    case RACE_INSECT:
    case RACE_ARACHNID:
        if (IS_PC(mob)) {
            GET_STR(mob) = 18;
            GET_ADD(mob) = 100;
        }
        break;
    case RACE_LYCANTH:
        SET_BIT(mob->M_immune, IMM_NONMAG);
        break;
    case RACE_PREDATOR:
        if (mob->skills) {
            mob->skills[SKILL_HUNT].learned = 100;
        }
        break;
    case RACE_GIANT_FROST:
        SET_BIT(mob->immune, IMM_COLD);
        SET_BIT(mob->susc, IMM_FIRE);
        break;
    case RACE_GIANT_FIRE:
        SET_BIT(mob->immune, IMM_FIRE);
        SET_BIT(mob->susc, IMM_COLD);
        break;
    case RACE_GIANT_CLOUD:
        /*
         * should be gas... but no IMM_GAS
         */
        SET_BIT(mob->immune, IMM_SLEEP);
        SET_BIT(mob->susc, IMM_ACID);
        break;
    case RACE_GIANT_STORM:
        SET_BIT(mob->immune, IMM_ELEC);
        break;
    case RACE_GIANT_STONE:
        SET_BIT(mob->immune, IMM_PIERCE);
        break;
    case RACE_UNDEAD:
    case RACE_UNDEAD_VAMPIRE:
    case RACE_UNDEAD_LICH:
    case RACE_UNDEAD_WIGHT:
    case RACE_UNDEAD_GHAST:
    case RACE_UNDEAD_GHOUL:
    case RACE_UNDEAD_SPECTRE:
    case RACE_UNDEAD_ZOMBIE:
    case RACE_UNDEAD_SKELETON:
        SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
        SET_BIT(mob->M_immune, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD |
                               IMM_CHARM);
        break;
    case RACE_DRAGON_RED:
        SET_BIT(mob->M_immune, IMM_FIRE);
        SET_BIT(mob->susc, IMM_COLD);
        break;
    case RACE_DRAGON_BLACK:
        SET_BIT(mob->M_immune, IMM_ACID);
        break;
    case RACE_DRAGON_GREEN:
        SET_BIT(mob->M_immune, IMM_SLEEP);
        break;
    case RACE_DRAGON_WHITE:
        SET_BIT(mob->M_immune, IMM_COLD);
        SET_BIT(mob->susc, IMM_FIRE);
        break;
    case RACE_DRAGON_BLUE:
        SET_BIT(mob->M_immune, IMM_ELEC);
        break;
    case RACE_DRAGON_SILVER:
        SET_BIT(mob->M_immune, IMM_ENERGY);
        break;
    case RACE_DRAGON_GOLD:
        SET_BIT(mob->specials.affected_by, AFF_WATERBREATH); 
        SET_BIT(mob->M_immune, IMM_SLEEP + IMM_ENERGY);
        break;
    case RACE_DRAGON_BRONZE:
        SET_BIT(mob->M_immune, IMM_COLD + IMM_ACID);
        SET_BIT(mob->specials.affected_by, AFF_WATERBREATH);  
        break;
    case RACE_DRAGON_COPPER:
        SET_BIT(mob->M_immune, IMM_FIRE);
        break;
    case RACE_DRAGON_BRASS:
        SET_BIT(mob->M_immune, IMM_ELEC);
        break;
    case RACE_DRAGON_AMETHYST:
        SET_BIT(mob->specials.affected_by, AFF_WATERBREATH); 
        break;
    case RACE_DRAGON_CRYSTAL:
        SET_BIT(mob->M_immune, IMM_COLD);
        break;
    case RACE_DRAGON_EMERALD:
        break;
    case RACE_DRAGON_SAPPHIRE:
        SET_BIT(mob->M_immune, IMM_HOLD);
        break;
    case RACE_DRAGON_TOPAZ:
        SET_BIT(mob->M_immune, IMM_COLD);
        SET_BIT(mob->specials.affected_by, AFF_WATERBREATH); 
        break;
    case RACE_DRAGON_BROWN:
        SET_BIT(mob->M_immune, IMM_ACID);
        break;
    case RACE_DRAGON_CLOUD:
        SET_BIT(mob->M_immune, IMM_COLD);
        break;
    case RACE_DRAGON_DEEP:
        SET_BIT(mob->M_immune, IMM_CHARM);
        SET_BIT(mob->M_immune, IMM_SLEEP);
        SET_BIT(mob->M_immune, IMM_HOLD);
        SET_BIT(mob->M_immune, IMM_COLD);
        SET_BIT(mob->M_immune, IMM_FIRE);
        SET_BIT(mob->specials.affected_by, AFF_TRUE_SIGHT);
        SET_BIT(mob->specials.affected_by, AFF_DETECT_MAGIC);
        SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
        break;
    case RACE_DRAGON_MERCURY:
        SET_BIT(mob->M_immune, IMM_FIRE);
        break;
    case RACE_DRAGON_MIST:
        SET_BIT(mob->M_immune, IMM_FIRE);
        break;
    case RACE_DRAGON_SHADOW:
        SET_BIT(mob->M_immune, IMM_DRAIN);
        break;
    case RACE_DRAGON_STEEL:
        SET_BIT(mob->M_immune, IMM_POISON);
        break;
    case RACE_DRAGON_YELLOW:
        SET_BIT(mob->M_immune, IMM_FIRE);
        break;
    case RACE_DRAGON_TURTLE:
        SET_BIT(mob->specials.affected_by, AFF_WATERBREATH); 
        break;
    case RACE_HALF_ELF:
    case RACE_HALF_OGRE:
    case RACE_HALF_ORC:
    case RACE_HALF_GIANT:
        SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
        break;
    default:
        break;
    }

    /*
     * height and weight / Hatred Foes! /
     */
    if (IS_NPC(mob)) {
        switch (GET_RACE(mob)) {
        case RACE_HUMAN:
            break;
        case RACE_MOON_ELF:
        case RACE_GOLD_ELF:
        case RACE_WILD_ELF:
        case RACE_FOREST_GNOME:
            AddHatred(mob, OP_RACE, RACE_ORC);
            break;
        case RACE_SEA_ELF:
            break;
        case RACE_ROCK_GNOME:
            break;
        case RACE_DEEP_GNOME:
            AddHatred(mob, OP_RACE, RACE_DROW);
        case RACE_DWARF:
            AddHatred(mob, OP_RACE, RACE_GOBLIN);
            AddHatred(mob, OP_RACE, RACE_ORC);
            break;
        case RACE_HALFLING:
            break;
        case RACE_LYCANTH:
            AddHatred(mob, OP_RACE, RACE_HUMAN);
            break;
        case RACE_UNDEAD:
            break;
        case RACE_DARK_DWARF:
        case RACE_UNDEAD_VAMPIRE:
        case RACE_UNDEAD_LICH:
        case RACE_UNDEAD_WIGHT:
        case RACE_UNDEAD_GHAST:
        case RACE_UNDEAD_GHOUL:
        case RACE_UNDEAD_SPECTRE:
            AddHatred(mob, OP_GOOD, 1000);
            break;
        case RACE_UNDEAD_ZOMBIE:
        case RACE_UNDEAD_SKELETON:
        case RACE_VEGMAN:
        case RACE_MFLAYER:
            break;
        case RACE_DROW:
            /*
             * I think this doesn't work!!! - Beirdo
             */
            AddHatred(mob, OP_RACE, RACE_MOON_ELF | RACE_WILD_ELF |
                                    RACE_GOLD_ELF);
            break;
        case RACE_SKEXIE:
        case RACE_TROGMAN:
        case RACE_LIZARDMAN:
        case RACE_SARTAN:
        case RACE_PATRYN:
        case RACE_DRAAGDIM:
        case RACE_ASTRAL:
            break;

        case RACE_HORSE:
            mob->player.weight = 400;
            mob->player.height = 175;
            break;

        case RACE_ORC:
            AddHatred(mob, OP_GOOD, 1000);
            /*
             * I think this doesn't work!!! - Beirdo
             */
            AddHatred(mob, OP_RACE, RACE_MOON_ELF | RACE_GOLD_ELF |
                                    RACE_WILD_ELF);
            mob->player.weight = 150;
            mob->player.height = 140;
            break;

        case RACE_SMURF:
            mob->player.weight = 5;
            mob->player.height = 10;
            break;

        case RACE_GOBLIN:
        case RACE_GNOLL:
            AddHatred(mob, OP_GOOD, 1000);
            AddHatred(mob, OP_RACE, RACE_DWARF);
            break;

        case RACE_ENFAN:
            mob->player.weight = 120;
            mob->player.height = 100;
            break;

        case RACE_LABRAT:
        case RACE_INSECT:
        case RACE_ARACHNID:
        case RACE_REPTILE:
        case RACE_DINOSAUR:
        case RACE_FISH:
        case RACE_PREDATOR:
        case RACE_SNAKE:
        case RACE_HERBIV:
        case RACE_VEGGIE:
        case RACE_ELEMENT:
        case RACE_PRIMATE:
            break;

        case RACE_GOLEM:
            mob->player.weight = 10 + GetMaxLevel(mob) * GetMaxLevel(mob) * 2;
            mob->player.height = 20 + MIN(mob->player.weight, 600);
            break;

        case RACE_DRAGON:
        case RACE_DRAGON_RED:
        case RACE_DRAGON_BLACK:
        case RACE_DRAGON_GREEN:
        case RACE_DRAGON_WHITE:
        case RACE_DRAGON_BLUE:
        case RACE_DRAGON_SILVER:
        case RACE_DRAGON_GOLD:
        case RACE_DRAGON_BRONZE:
        case RACE_DRAGON_COPPER:
        case RACE_DRAGON_BRASS:
        case RACE_DRAGON_AMETHYST:
        case RACE_DRAGON_CRYSTAL:
        case RACE_DRAGON_EMERALD:
        case RACE_DRAGON_SAPPHIRE:
        case RACE_DRAGON_TOPAZ:
        case RACE_DRAGON_BROWN:
        case RACE_DRAGON_CLOUD:
        case RACE_DRAGON_DEEP:
        case RACE_DRAGON_MERCURY:
        case RACE_DRAGON_MIST:
        case RACE_DRAGON_SHADOW:
        case RACE_DRAGON_STEEL:
        case RACE_DRAGON_YELLOW:
        case RACE_DRAGON_TURTLE:
            mob->player.weight = MAX(60,
                                     GetMaxLevel(mob) * GetMaxLevel(mob) * 2);
            mob->player.height = 100 + MIN(mob->player.weight, 500);
            break;

        case RACE_BIRD:
        case RACE_PARASITE:
        case RACE_SLIME:
            mob->player.weight = GetMaxLevel(mob) * (GetMaxLevel(mob) / 5);
            mob->player.height = 10 * GetMaxLevel(mob);
            break;

        case RACE_GHOST:
            mob->player.weight = GetMaxLevel(mob) * (GetMaxLevel(mob) / 5);
            mob->player.height = 10 * GetMaxLevel(mob);
            break;

        case RACE_TROLL:
            AddHatred(mob, OP_GOOD, 1000);
            mob->player.height = 200 + GetMaxLevel(mob) * 15;
            mob->player.weight = (int) mob->player.height * 1.5;
            break;

        case RACE_GIANT:
        case RACE_GIANT_HILL:
        case RACE_GIANT_FROST:
        case RACE_GIANT_FIRE:
        case RACE_GIANT_CLOUD:
        case RACE_GIANT_STORM:
            mob->player.height = 200 + GetMaxLevel(mob) * 15;
            mob->player.weight = (int) mob->player.height * 1.5;
            AddHatred(mob, OP_RACE, RACE_DWARF);
            break;

        case RACE_DEVIL:
        case RACE_DEMON:
            AddHatred(mob, OP_GOOD, 1000);
            mob->player.height = 200 + GetMaxLevel(mob) * 15;
            mob->player.weight = (int) mob->player.height * 1.5;
            break;

        case RACE_PLANAR:
            mob->player.height = 200 + GetMaxLevel(mob) * 15;
            mob->player.weight = (int) mob->player.height * 1.5;
            break;

        case RACE_GOD:
        case RACE_TREE:
            break;

        case RACE_TYTAN:
            mob->player.weight = MAX(500,
                                     GetMaxLevel(mob) * GetMaxLevel(mob) * 10);
            mob->player.height = GetMaxLevel(mob) / 2 * 100;
            break;

        case RACE_HALF_ELF:
        case RACE_HALF_OGRE:
        case RACE_HALF_ORC:
        case RACE_HALF_GIANT:
            break;
        }
    }
}

int GetNewRace(struct char_file_u *s)
{
    int             return_race,
                    try_again;

    do {
        return_race = number(1, raceCount);

        switch (return_race) {
            /*
             * we allow these races to be used in reincarnations
             */
        case RACE_HUMAN:
        case RACE_MOON_ELF:
        case RACE_DWARF:
        case RACE_HALFLING:
        case RACE_ROCK_GNOME:
        case RACE_FOREST_GNOME:
        case RACE_ORC:
        case RACE_DROW:
        case RACE_MFLAYER:
        case RACE_DARK_DWARF:
        case RACE_DEEP_GNOME:
        case RACE_GNOLL:
        case RACE_GOLD_ELF:
        case RACE_WILD_ELF:
        case RACE_SEA_ELF:
        case RACE_LIZARDMAN:
        case RACE_HALF_ELF:
        case RACE_HALF_OGRE:
        case RACE_HALF_ORC:
            try_again = FALSE;
            break;
        case RACE_HALF_GIANT:
        case RACE_GIANT_HILL:
        case RACE_GIANT_FROST:
        case RACE_GIANT_FIRE:
        case RACE_GIANT_CLOUD:
        case RACE_GIANT_STORM:
            if (!number(0,9)) {
                try_again = TRUE;
                break;
            } else {
                try_again = FALSE;
                break;
            }
        case RACE_ROO:
        case RACE_PRIMATE:
        case RACE_GOBLIN:
        case RACE_TROLL:
        case RACE_AVARIEL:
            try_again = FALSE;
            break;
            /*
             * not a valid race, try again
             */
        default:
            try_again = TRUE;
            break;
        }
    } while (try_again);

    return (return_race);
}


int MaxDexForRace(struct char_data *ch)
{
    switch (GET_RACE(ch)) {
    case RACE_MOON_ELF:
    case RACE_WILD_ELF:
    case RACE_GOLD_ELF:
    case RACE_HALFLING:
    case RACE_FOREST_GNOME:
        return (19);
        break;
    case RACE_DROW:
    case RACE_AVARIEL:
        return (20);
        break;
    case RACE_DWARF:
    case RACE_HALF_OGRE:
    case RACE_GIANT_CLOUD:
    case RACE_GIANT_HILL:
        return (17);
        break;
    case RACE_HALF_GIANT:
    case RACE_GIANT_FROST:
    case RACE_GIANT_FIRE:
    case RACE_GIANT_STORM:
    case RACE_GIANT_STONE:
        return (16);
        break;
    default:
        return (18);
        break;
    }
}

int MaxIntForRace(struct char_data *ch)
{
    switch (GET_RACE(ch)) {
    case RACE_GOLD_ELF:
    case RACE_ROCK_GNOME:
        return (19);
        break;
    case RACE_HALF_GIANT:
    case RACE_HALF_OGRE:
    case RACE_FOREST_GNOME:
        return (17);
        break;

    default:
        return (18);
        break;
    }
}

int MaxWisForRace(struct char_data *ch)
{
    switch (GET_RACE(ch)) {
    case RACE_GOLD_ELF:
    case RACE_WILD_ELF:
    case RACE_ROCK_GNOME:
    case RACE_HALF_GIANT:
        return (17);
        break;
    case RACE_FOREST_GNOME:
        return (19);
        break;

    default:
        return (18);
        break;
    }
}

int MaxConForRace(struct char_data *ch)
{
    switch (GET_RACE(ch)) {
    case RACE_HALF_ORC:
    case RACE_DWARF:
    case RACE_HALF_OGRE:
    case RACE_DARK_DWARF:
        return (19);
        break;
    case RACE_MOON_ELF:
    case RACE_GOLD_ELF:
    case RACE_SEA_ELF:
    case RACE_DROW:
    case RACE_AVARIEL:
        return (17);
        break;
    default:
        return (18);
        break;
    }
}

int MaxChrForRace(struct char_data *ch)
{
    switch (GET_RACE(ch)) {
    case RACE_DEEP_GNOME:
        return (16);
        break;
    case RACE_HALF_ORC:
    case RACE_ORC:
    case RACE_DROW:
    case RACE_DWARF:
    case RACE_DARK_DWARF:
        return (17);
        break;
    default:
        return (18);
        break;
    }
}

int MaxStrForRace(struct char_data *ch)
{
    if (GetMaxLevel(ch) >= SILLYLORD) {
        return (25);
    }
    switch (GET_RACE(ch)) {
    case RACE_TROLL:
    case RACE_HALF_GIANT:
        return (19);
        break;

    case RACE_HALFLING:
    case RACE_GOBLIN:
        return (17);
        break;
    default:
        return (18);
        break;
    }
}


void SetDefaultLang(struct char_data *ch)
{
    int             skill;
    int             race;
    int             index;

    race = GET_RACE(ch);
    index = races[race].nativeLanguage;

    ch->player.speaks = languages[index].langSpeaks;

    skill = languages[index].langSkill;
    if( skill ) {
        ch->skills[skill].learned = 95;
        SET_BIT(ch->skills[skill].flags, SKILL_KNOWN);
    }
}


/*
 * Good side just means they are of the first races and on the
 * general good side of life, it does NOT refer to alignment
 * only good side people can kill bad side people PC's
 */
int IsGoodSide(struct char_data *ch)
{
    switch (GET_RACE(ch)) {

    case RACE_HUMAN:
    case RACE_MOON_ELF:
    case RACE_AVARIEL:
    case RACE_GOLD_ELF:
    case RACE_WILD_ELF:
    case RACE_SEA_ELF:
    case RACE_DWARF:
    case RACE_HALFLING:
    case RACE_ROCK_GNOME:
    case RACE_HALF_ELF:
    case RACE_HALF_OGRE:
    case RACE_HALF_ORC:
    case RACE_HALF_GIANT:
        return (TRUE);
    }

    return (FALSE);
}

/*
 * this just means the PC is a troll/orc or the like not related to
 * to alignment what-so-ever
 * only bad side people can hit and kill good side people PC's
 */
int IsBadSide(struct char_data *ch)
{
    switch (GET_RACE(ch)) {
    case RACE_GOBLIN:
    case RACE_DARK_DWARF:
    case RACE_ORC:
    case RACE_TROLL:
    case RACE_MFLAYER:
    case RACE_DROW:
        return (TRUE);
    }

    return (FALSE);
}

/*
 * good side can fight bad side people pc to pc fighting only, not used
 * for NPC fighting to pc fighting
 */
int CanFightEachOther(struct char_data *ch, struct char_data *ch2)
{
    if ((IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF)) ||
        (IS_NPC(ch2) && !IS_SET(ch2->specials.act, ACT_POLYSELF))) {
        return (TRUE);
    }
    if (IS_SET(SystemFlags, SYS_NOKILL)) {
        return (FALSE);
    }
    if (roomFindNum(ch->in_room) && roomFindNum(ch2->in_room)) {
        if (IS_SET(roomFindNum(ch->in_room)->room_flags, ARENA_ROOM) &&
            IS_SET(roomFindNum(ch2->in_room)->room_flags, ARENA_ROOM)) {
            return (TRUE);
        }
    } else {
        return (FALSE);
    }

    if (IS_SET(SystemFlags, SYS_WLD_ARENA) && IS_AFFECTED2(ch, AFF2_QUEST) &&
        IS_AFFECTED2(ch2, AFF2_QUEST)) {
        return (TRUE);
    }

    if ((IsGoodSide(ch) && IsGoodSide(ch2)) ||
        (IsBadSide(ch) && IsBadSide(ch2))) {
        return (FALSE);
    } else if (IS_SET(ch->player.user_flags, RACE_WAR) &&
               IS_SET(ch2->player.user_flags, RACE_WAR)) {
        return (TRUE);
    }

    return (FALSE);
}


char           *DescAge(int age, int race)
{
    if (age > races[race].venerable) {
        return "Venerable";
    } else if (age > races[race].ancient) {
        return "Ancient";
    } else if (age > races[race].old) {
        return "Old";
    } else if (age > races[race].middle) {
        return "Middle Aged";
    } else if (age > races[race].mature) {
        return "Mature";
    } else if (age > races[race].young) {
        return "Young";
    } else {
        return "ERROR";
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
