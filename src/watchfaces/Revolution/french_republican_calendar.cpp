#include "french_republican_calendar.h"
#include "const.h"
#include <Arduino.h>

// Last full month
const unsigned int FrenchRepublicanCalendar::FRUCTIDOR = 12;
// Leap day
const unsigned int FrenchRepublicanCalendar::REVOLUTION = 18;

// The font used by the watch has been modified to support French characters using the following mapping:
// ! → é
// " → è
// # → ê
// $ → ô
// % → â
// & → ë
// ( → ï
// ) → û
// * → œ
// { → É
// | → Â
// } → Œ
// ' → ’
const char *FrenchRepublicanCalendar::MONTH_NAMES[] = {
    "Vend!miaire", "Brumaire",  "Frimaire",  "Niv$se", "Pluvi$se", "Vent$se", "Germinal", "Flor!al",     "Prairial",
    "Messidor",    "Thermidor", "Fructidor", "Vertu",  "G!nie",    "Travail", "Opinion",  "R!compenses", "R!volution"};

const char *FrenchRepublicanCalendar::WEEK_DAY_NAMES[] = {"Primidi", "Duodi",   "Tridi",  "Quartidi", "Quintidi",
                                                          "Sextidi", "Septidi", "Octidi", "Nonidi",   "D!cadi"};

const char *FrenchRepublicanCalendar::YEAR_DAY_NAMES[366][2] = {{"Raisin", "Grape"},
                                                                {"Safran", "Saffron"},
                                                                {"Ch%taigne", "Chestnut"},
                                                                {"Colchique", "Crocus"},
                                                                {"Cheval", "Horse"},
                                                                {"Balsamine", "Impatiens"},
                                                                {"Carotte", "Carrot"},
                                                                {"Amaranthe", "Amaranth"},
                                                                {"Panais", "Parsnip"},
                                                                {"Cuve", "Vat"},
                                                                {"Pomme de terre", "Potato"},
                                                                {"Immortelle", "Strawflower"},
                                                                {"Potiron", "Winter squash"},
                                                                {"R!s!da", "Mignonette"},
                                                                {"|ne", "Donkey"},
                                                                {"Belle de nuit", "Four o'clock flower"},
                                                                {"Citrouille", "Pumpkin"},
                                                                {"Sarrasin", "Buckwheat"},
                                                                {"Tournesol", "Sunflower"},
                                                                {"Pressoir", "Wine-Press"},
                                                                {"Chanvre", "Hemp"},
                                                                {"P#che", "Peach"},
                                                                {"Navet", "Turnip"},
                                                                {"Amaryllis", "Amaryllis"},
                                                                {"B*uf", "Ox"},
                                                                {"Aubergine", "Eggplant"},
                                                                {"Piment", "Chili pepper"},
                                                                {"Tomate", "Tomato"},
                                                                {"Orge", "Barley"},
                                                                {"Tonneau", "Barrel"},
                                                                {"Pomme", "Apple"},
                                                                {"C!leri", "Celery"},
                                                                {"Poire", "Pear"},
                                                                {"Betterave", "Beetroot"},
                                                                {"Oie", "Goose"},
                                                                {"H!liotrope", "Heliotrope"},
                                                                {"Figue", "Common fig"},
                                                                {"Scorson\"re", "Black Salsify"},
                                                                {"Alisier", "Chequer Tree"},
                                                                {"Charrue", "Plough"},
                                                                {"Salsifis", "Salsify"},
                                                                {"M%cre", "Water chestnut"},
                                                                {"Topinambour", "Jerusalem artichoke"},
                                                                {"Endive", "Endive"},
                                                                {"Dindon", "Turkey"},
                                                                {"Chervis", "Skirret"},
                                                                {"Cresson", "Watercress"},
                                                                {"Dentelaire", "Leadworts"},
                                                                {"Grenade", "Pomegranate"},
                                                                {"Herse", "Harrow"},
                                                                {"Bacchante", "Baccharis"},
                                                                {"Azerole", "Azarole"},
                                                                {"Garance", "Madder"},
                                                                {"Orange", "Orange"},
                                                                {"Faisan", "Pheasant"},
                                                                {"Pistache", "Pistachio"},
                                                                {"Macjonc", "Tuberous pea"},
                                                                {"Coing", "Quince"},
                                                                {"Cormier", "Service tree"},
                                                                {"Rouleau", "Roller"},
                                                                {"Raiponce", "Rampion"},
                                                                {"Turneps", "Turnip"},
                                                                {"Chicor!e", "Chicory"},
                                                                {"N\"fle", "Medlar"},
                                                                {"Cochon", "Pig"},
                                                                {"M%che", "Lamb's lettuce"},
                                                                {"Chou-fleur", "Cauliflower"},
                                                                {"Miel", "Honey"},
                                                                {"Geni\"vre", "Juniper"},
                                                                {"Pioche", "Pickaxe"},
                                                                {"Cire", "Wax"},
                                                                {"Raifort", "Horseradish"},
                                                                {"C\"dre", "Cedar tree"},
                                                                {"Sapin", "Fir"},
                                                                {"Chevreuil", "Roe deer"},
                                                                {"Ajonc", "Gorse"},
                                                                {"Cypr\"s", "Cypress Tree"},
                                                                {"Lierre", "Ivy"},
                                                                {"Sabine", "Savin Juniper"},
                                                                {"Hoyau", "Grub-hoe"},
                                                                {"{rable à sucre", "Sugar Maple"},
                                                                {"Bruy\"re", "Heather"},
                                                                {"Roseau", "Reed plant"},
                                                                {"Oseille", "Sorrel"},
                                                                {"Grillon", "Cricket"},
                                                                {"Pignon", "Pine nut"},
                                                                {"Li\"ge", "Cork"},
                                                                {"Truffe", "Truffle"},
                                                                {"Olive", "Olive"},
                                                                {"Pelle", "Shovel"},
                                                                {"Tourbe", "Peat"},
                                                                {"Houille", "Coal"},
                                                                {"Bitume", "Bitumen"},
                                                                {"Soufre", "Sulphur"},
                                                                {"Chien", "Dog"},
                                                                {"Lave", "Lava"},
                                                                {"Terre v!g!tale", "Topsoil"},
                                                                {"Fumier", "Manure"},
                                                                {"Salp#tre", "Saltpeter"},
                                                                {"Fl!au", "Flail"},
                                                                {"Granit", "Granite"},
                                                                {"Argile", "Clay"},
                                                                {"Ardoise", "Slate"},
                                                                {"Gr\"s", "Sandstone"},
                                                                {"Lapin", "Rabbit"},
                                                                {"Silex", "Flint"},
                                                                {"Marne", "Marl"},
                                                                {"Pierre à chaux", "Limestone"},
                                                                {"Marbre", "Marble"},
                                                                {"Van", "Winnowing basket"},
                                                                {"Pierre à pl%tre", "Gypsum"},
                                                                {"Sel", "Salt"},
                                                                {"Fer", "Iron"},
                                                                {"Cuivre", "Copper"},
                                                                {"Chat", "Cat"},
                                                                {"{tain", "Tin"},
                                                                {"Plomb", "Lead"},
                                                                {"Zinc", "Zinc"},
                                                                {"Mercure", "Mercury"},
                                                                {"Crible", "Sieve"},
                                                                {"Laur!ole", "Spurge-laurel"},
                                                                {"Mousse", "Moss"},
                                                                {"Fragon", "Butcher's Broom"},
                                                                {"Perce-neige", "Snowdrop"},
                                                                {"Taureau", "Bull"},
                                                                {"Laurier-thym", "Laurustinus"},
                                                                {"Amadouvier", "Tinder polypore"},
                                                                {"M!z!r!on", "Daphne mezereum"},
                                                                {"Peuplier", "Poplar"},
                                                                {"Coign!e", "Axe"},
                                                                {"Ell!bore", "Hellebore"},
                                                                {"Brocoli", "Broccoli"},
                                                                {"Laurier", "Bay laurel"},
                                                                {"Avelinier", "Filbert"},
                                                                {"Vache", "Cow"},
                                                                {"Buis", "Box Tree"},
                                                                {"Lichen", "Lichen"},
                                                                {"If", "Yew tree"},
                                                                {"Pulmonaire", "Lungwort"},
                                                                {"Serpette", "Billhook"},
                                                                {"Thlaspi", "Pennycress"},
                                                                {"Thimel!", "Rose Daphne"},
                                                                {"Chiendent", "Couch grass"},
                                                                {"Trainasse", "Common Knotgrass"},
                                                                {"Li\"vre", "Hare"},
                                                                {"Gu\"de", "Woad"},
                                                                {"Noisetier", "Hazel"},
                                                                {"Cyclamen", "Cyclamen"},
                                                                {"Ch!lidoine", "Celandine"},
                                                                {"Traîneau", "Sleigh"},
                                                                {"Tussilage", "Coltsfoot"},
                                                                {"Cornouiller", "Dogwood"},
                                                                {"Violier", "Matthiola"},
                                                                {"Tro\"ne", "Privet"},
                                                                {"Bouc", "Billygoat"},
                                                                {"Asaret", "Wild Ginger"},
                                                                {"Alaterne", "Italian Buckthorn"},
                                                                {"Violette", "Violet"},
                                                                {"Marceau", "Goat Willow"},
                                                                {"B#che", "Spade"},
                                                                {"Narcisse", "Narcissus"},
                                                                {"Orme", "Elm"},
                                                                {"Fumeterre", "Common fumitory"},
                                                                {"V!lar", "Hedge mustard"},
                                                                {"Ch\"vre", "Goat"},
                                                                {"{pinard", "Spinach"},
                                                                {"Doronic", "Doronicum"},
                                                                {"Mouron", "Pimpernel"},
                                                                {"Cerfeuil", "Chervil"},
                                                                {"Cordeau", "Twine"},
                                                                {"Mandragore", "Mandrake"},
                                                                {"Persil", "Parsley"},
                                                                {"Cochl!aria", "Scurvy-grass"},
                                                                {"P%querette", "Daisy"},
                                                                {"Thon", "Tuna"},
                                                                {"Pissenlit", "Dandelion"},
                                                                {"Sylvie", "Wood Anemone"},
                                                                {"Capillaire", "Maidenhair fern"},
                                                                {"Fr#ne", "Ash tree"},
                                                                {"Plantoir", "Dibber"},
                                                                {"Primev\"re", "Primrose"},
                                                                {"Platane", "Plane Tree"},
                                                                {"Asperge", "Asparagus"},
                                                                {"Tulipe", "Tulip"},
                                                                {"Poule", "Hen"},
                                                                {"Bette", "Chard"},
                                                                {"Bouleau", "Birch"},
                                                                {"Jonquille", "Daffodil"},
                                                                {"Aulne", "Alder"},
                                                                {"Couvoir", "Hatchery"},
                                                                {"Pervenche", "Periwinkle"},
                                                                {"Charme", "Hornbeam"},
                                                                {"Morille", "Morel"},
                                                                {"H#tre", "Beech Tree"},
                                                                {"Abeille", "Bee"},
                                                                {"Laitue", "Lettuce"},
                                                                {"M!l\"ze", "Larch"},
                                                                {"Cigu&", "Hemlock"},
                                                                {"Radis", "Radish"},
                                                                {"Ruche", "Hive"},
                                                                {"Gainier", "Judas tree"},
                                                                {"Romaine", "Romaine lettuce"},
                                                                {"Marronnier", "Horse chestnut"},
                                                                {"Roquette", "Arugula or Rocket"},
                                                                {"Pigeon", "Pigeon"},
                                                                {"Lilas", "Lilac"},
                                                                {"An!mone", "Anemone"},
                                                                {"Pens!e", "Pansy"},
                                                                {"Myrtille", "Bilberry"},
                                                                {"Greffoir", "Knife"},
                                                                {"Rose", "Rose"},
                                                                {"Ch#ne", "Oak Tree"},
                                                                {"Foug\"re", "Fern"},
                                                                {"Aub!pine", "Hawthorn"},
                                                                {"Rossignol", "Nightingale"},
                                                                {"Ancolie", "Common Columbine"},
                                                                {"Muguet", "Lily of the valley"},
                                                                {"Champignon", "Button mushroom"},
                                                                {"Hyacinthe", "Hyacinth"},
                                                                {"R%teau", "Rake"},
                                                                {"Rhubarbe", "Rhubarb"},
                                                                {"Sainfoin", "Sainfoin"},
                                                                {"B%ton d'or", "Wallflower"},
                                                                {"Chamerisier", "Fan Palm tree"},
                                                                {"Ver à soie", "Silkworm"},
                                                                {"Consoude", "Comfrey"},
                                                                {"Pimprenelle", "Salad burnet"},
                                                                {"Corbeille d'or", "Basket of Gold"},
                                                                {"Arroche", "Orache"},
                                                                {"Sarcloir", "Garden hoe"},
                                                                {"Statice", "Thrift"},
                                                                {"Fritillaire", "Fritillary"},
                                                                {"Bourrache", "Borage"},
                                                                {"Val!riane", "Valerian"},
                                                                {"Carpe", "Carp"},
                                                                {"Fusain", "Spindle"},
                                                                {"Civette", "Chive"},
                                                                {"Buglosse", "Bugloss"},
                                                                {"S!nev!", "Wild mustard"},
                                                                {"Houlette", "Shepherd's crook"},
                                                                {"Luzerne", "Alfalfa"},
                                                                {"H!m!rocalle", "Daylily"},
                                                                {"Tr\"fle", "Clover"},
                                                                {"Ang!lique", "Angelica"},
                                                                {"Canard", "Duck"},
                                                                {"M!lisse", "Lemon balm"},
                                                                {"Fromental", "Oat grass"},
                                                                {"Martagon", "Martagon lily"},
                                                                {"Serpolet", "Wild Thyme"},
                                                                {"Faux", "Scythe"},
                                                                {"Fraise", "Strawberry"},
                                                                {"B!toine", "Woundwort"},
                                                                {"Pois", "Pea"},
                                                                {"Acacia", "Acacia"},
                                                                {"Caille", "Quail"},
                                                                {"}illet", "Carnation"},
                                                                {"Sureau", "Elderberry"},
                                                                {"Pavot", "Poppy plant"},
                                                                {"Tilleul", "Linden or Lime tree"},
                                                                {"Fourche", "Pitchfork"},
                                                                {"Barbeau", "Cornflower"},
                                                                {"Camomille", "Camomile"},
                                                                {"Ch\"vrefeuille", "Honeysuckle"},
                                                                {"Caille-lait", "Bedstraw"},
                                                                {"Tanche", "Tench"},
                                                                {"Jasmin", "Jasmine"},
                                                                {"Verveine", "Verbena"},
                                                                {"Thym", "Thyme"},
                                                                {"Pivoine", "Peony"},
                                                                {"Chariot", "Hand Cart"},
                                                                {"Seigle", "Rye"},
                                                                {"Avoine", "Oat"},
                                                                {"Oignon", "Onion"},
                                                                {"V!ronique", "Speedwell"},
                                                                {"Mulet", "Mule"},
                                                                {"Romarin", "Rosemary"},
                                                                {"Concombre", "Cucumber"},
                                                                {"{chalote", "Shallot"},
                                                                {"Absinthe", "Wormwood"},
                                                                {"Faucille", "Sickle"},
                                                                {"Coriandre", "Coriander"},
                                                                {"Artichaut", "Artichoke"},
                                                                {"Girofle", "Clove"},
                                                                {"Lavande", "Lavender"},
                                                                {"Chamois", "Chamois"},
                                                                {"Tabac", "Tobacco"},
                                                                {"Groseille", "Redcurrant"},
                                                                {"Gesse", "Hairy Vetchling"},
                                                                {"Cerise", "Cherry"},
                                                                {"Parc", "Park"},
                                                                {"Menthe", "Mint"},
                                                                {"Cumin", "Cumin"},
                                                                {"Haricot", "Bean"},
                                                                {"Orcan\"te", "Alkanet"},
                                                                {"Pintade", "Guinea fowl"},
                                                                {"Sauge", "Sage Plant"},
                                                                {"Ail", "Garlic"},
                                                                {"Vesce", "Tare"},
                                                                {"Bl!", "Wheat"},
                                                                {"Chal!mie", "Shawm"},
                                                                {"{peautre", "Spelt"},
                                                                {"Bouillon blanc", "Common mullein"},
                                                                {"Melon", "Melon"},
                                                                {"Ivraie", "Ryegrass"},
                                                                {"B!lier", "Ram"},
                                                                {"Pr#le", "Horsetail"},
                                                                {"Armoise", "Mugwort"},
                                                                {"Carthame", "Safflower"},
                                                                {"M)re", "Blackberry"},
                                                                {"Arrosoir", "Watering can"},
                                                                {"Panic", "Switchgrass"},
                                                                {"Salicorne", "Common Glasswort"},
                                                                {"Abricot", "Apricot"},
                                                                {"Basilic", "Basil"},
                                                                {"Brebis", "Ewe"},
                                                                {"Guimauve", "Marshmallow"},
                                                                {"Lin", "Flax"},
                                                                {"Amande", "Almond"},
                                                                {"Gentiane", "Gentian"},
                                                                {"{cluse", "Lock"},
                                                                {"Carline", "Carline thistle"},
                                                                {"C%prier", "Caper"},
                                                                {"Lentille", "Lentil"},
                                                                {"Aun!e", "Inula"},
                                                                {"Loutre", "Otter"},
                                                                {"Myrte", "Myrtle"},
                                                                {"Colza", "Rapeseed"},
                                                                {"Lupin", "Lupin"},
                                                                {"Coton", "Cotton"},
                                                                {"Moulin", "Mill"},
                                                                {"Prune", "Plum"},
                                                                {"Millet", "Millet"},
                                                                {"Lycoperdon", "Puffball"},
                                                                {"Escourgeon", "Six-row Barley"},
                                                                {"Saumon", "Salmon"},
                                                                {"Tub!reuse", "Tuberose"},
                                                                {"Sucrion", "Winter Barley"},
                                                                {"Apocyn", "Apocynum"},
                                                                {"R!glisse", "Liquorice"},
                                                                {"{chelle", "Ladder"},
                                                                {"Past\"que", "Watermelon"},
                                                                {"Fenouil", "Fennel"},
                                                                {"{pine vinette", "Barberry"},
                                                                {"Noix", "Walnut"},
                                                                {"Truite", "Trout"},
                                                                {"Citron", "Lemon"},
                                                                {"Card\"re", "Teasel"},
                                                                {"Nerprun", "Buckthorn"},
                                                                {"Tagette", "Mexican Marigold"},
                                                                {"Hotte", "Harvesting basket"},
                                                                {"{glantier", "Wild Rose"},
                                                                {"Noisette", "Hazelnut"},
                                                                {"Houblon", "Hops"},
                                                                {"Sorgho", "Sorghum"},
                                                                {"{crevisse", "Crayfish"},
                                                                {"Bigarade", "Bitter orange"},
                                                                {"Verge d'or", "Goldenrod"},
                                                                {"Ma(s", "Maize or Corn"},
                                                                {"Marron", "Sweet Chestnut"},
                                                                {"Panier", "Pack Basket"},
                                                                {"Vertu", "Virtue"},
                                                                {"G!nie", "Talent"},
                                                                {"Travail", "Labour"},
                                                                {"Opinion", "Convictions"},
                                                                {"R!compenses", "Honors"},
                                                                {"R!volution", "Revolution"}};

FrenchRepublicanCalendar::FrenchRepublicanCalendar(Language lang) : lang(lang), year(1), monthNumber(1), dayInMonth(1)
{
}

void FrenchRepublicanCalendar::update(const time_t &currentTime)
{
    // January 1st, 1970 (start of the Unix epoch) was the 11 Nivôse of year 178 (using the Romme method)
    unsigned int year = 178;
    unsigned int month = 4;
    unsigned int absoluteDays = 11 + (int)currentTime / (60 * 60 * 24);

#ifdef DEBUG
    Serial.printf("Total absolute days: %d\n", absoluteDays);
#endif

    // Full years
    while (absoluteDays >= this->daysInFrenchYear(year)) {
        absoluteDays -= this->daysInFrenchYear(year);
        year++;
    }

    this->year = year;

#ifdef DEBUG
    Serial.printf("Absolute days after removing full years: %d\n", absoluteDays);
#endif

    // Full Months
    while (absoluteDays >= this->daysInFrenchMonth(year, month)) {
        absoluteDays -= this->daysInFrenchMonth(year, month);
        month++;
    }
    this->monthNumber = month;

#ifdef DEBUG
    Serial.printf("Absolute days after removing full months (and the day of the French month): %d\n", absoluteDays);
#endif

    this->dayInMonth = absoluteDays;
}

bool FrenchRepublicanCalendar::isFrenchLeapYear(unsigned int year) const
{
    // We only need to care about years after 1970
    // Use the Romme method
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

int FrenchRepublicanCalendar::daysInFrenchYear(unsigned int year) const
{
    return isFrenchLeapYear(year) ? 366 : 365;
}

int FrenchRepublicanCalendar::daysInFrenchMonth(unsigned int year, unsigned int month) const
{
    if (month <= this->FRUCTIDOR) {
        return 30;
    } else if (month == this->REVOLUTION) {
        return isFrenchLeapYear(year) ? 1 : 0;
    } else {
        return 1;
    }
}

const char *FrenchRepublicanCalendar::getMonthName() const
{
#ifdef DEMO
    return this->MONTH_NAMES[0];
#else
    if (this->monthNumber < 1 || this->monthNumber > sizeof(this->MONTH_NAMES)) {
        return "";
    }

    return this->MONTH_NAMES[this->monthNumber - 1];
#endif
}

unsigned int FrenchRepublicanCalendar::getDay() const
{
#ifdef DEMO
    return 1;
#else
    return this->dayInMonth;
#endif
}

const char *FrenchRepublicanCalendar::getWeekDayName() const
{
#ifdef DEMO
    return this->WEEK_DAY_NAMES[0];
#else
    return this->dayInMonth % 10 == 0 ? this->WEEK_DAY_NAMES[9] : this->WEEK_DAY_NAMES[this->dayInMonth % 10 - 1];
#endif
}

const char *FrenchRepublicanCalendar::getYearDayName() const
{
#ifdef DEMO
    return this->YEAR_DAY_NAMES[0][0];
#else
    const unsigned int dayIndex = (this->monthNumber - 1) * 30 + dayInMonth - 1;

    switch (this->lang) {
    case Language::English:
        return this->YEAR_DAY_NAMES[dayIndex][1];
    default:
        return this->YEAR_DAY_NAMES[dayIndex][0];
    }
#endif
}

bool FrenchRepublicanCalendar::sansculottides() const
{
    return this->monthNumber > FRUCTIDOR;
}

unsigned int FrenchRepublicanCalendar::getYear() const
{
    return this->year;
}

const String FrenchRepublicanCalendar::ROMAN_NUMBER_MATRIX[14][2] = {
    {"1000", "M"}, {"900", "CM"}, {"500", "D"}, {"400", "CD"}, {"100", "C"}, {"90", "XC"}, {"50", "L"},
    {"40", "XL"},  {"10", "X"},   {"9", "IX"},  {"5", "V"},    {"4", "IV"},  {"1", "I"},   {"0", ""}};

unsigned int FrenchRepublicanCalendar::floorRoman(unsigned int number) const
{
    unsigned int index = 0;

    while (index < sizeof(this->ROMAN_NUMBER_MATRIX)) {
        if (this->ROMAN_NUMBER_MATRIX[index][0].toInt() <= number) {
            break;
        }
        index++;
    }

    return index;
}

const String FrenchRepublicanCalendar::getRomanizedYear() const
{
    unsigned int year = this->year;
    unsigned int index = 0;
    String output = "";

    while (this->floorRoman(year) < 13) {
        index = this->floorRoman(year);
        output += this->ROMAN_NUMBER_MATRIX[index][1];
        year -= this->ROMAN_NUMBER_MATRIX[index][0].toInt();
    }

    return output;
}
