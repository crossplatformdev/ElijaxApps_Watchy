#include "Watchy_Pokemon.h"

#include "../../sdk/UiSDK.h"

const unsigned char *pokemon_back[151] = {back_1, back_2, back_3, back_4, back_5, back_6, back_7, back_8, back_9, back_10, back_11, back_12, back_13, back_14, back_15, back_16, back_17, back_18, back_19, back_20, back_21, back_22, back_23, back_24, back_25, back_26, back_27, back_28, back_29, back_30, back_31, back_32, back_33, back_34, back_35, back_36, back_37, back_38, back_39, back_40, back_41, back_42, back_43, back_44, back_45, back_46, back_47, back_48, back_49, back_50, back_51, back_52, back_53, back_54, back_55, back_56, back_57, back_58, back_59, back_60, back_61, back_62, back_63, back_64, back_65, back_66, back_67, back_68, back_69, back_70, back_71, back_72, back_73, back_74, back_75, back_76, back_77, back_78, back_79, back_80, back_81, back_82, back_83, back_84, back_85, back_86, back_87, back_88, back_89, back_90, back_91, back_92, back_93, back_94, back_95, back_96, back_97, back_98, back_99, back_100, back_101, back_102, back_103, back_104, back_105, back_106, back_107, back_108, back_109, back_110, back_111, back_112, back_113, back_114, back_115, back_116, back_117, back_118, back_119, back_120, back_121, back_122, back_123, back_124, back_125, back_126, back_127, back_128, back_129, back_130, back_131, back_132, back_133, back_134, back_135, back_136, back_137, back_138, back_139, back_140, back_141, back_142, back_143, back_144, back_145, back_146, back_147, back_148, back_149, back_150, back_151};
const unsigned char *pokemon_front[151] = {front_1, front_2, front_3, front_4, front_5, front_6, front_7, front_8, front_9, front_10, front_11, front_12, front_13, front_14, front_15, front_16, front_17, front_18, front_19, front_20, front_21, front_22, front_23, front_24, front_25, front_26, front_27, front_28, front_29, front_30, front_31, front_32, front_33, front_34, front_35, front_36, front_37, front_38, front_39, front_40, front_41, front_42, front_43, front_44, front_45, front_46, front_47, front_48, front_49, front_50, front_51, front_52, front_53, front_54, front_55, front_56, front_57, front_58, front_59, front_60, front_61, front_62, front_63, front_64, front_65, front_66, front_67, front_68, front_69, front_70, front_71, front_72, front_73, front_74, front_75, front_76, front_77, front_78, front_79, front_80, front_81, front_82, front_83, front_84, front_85, front_86, front_87, front_88, front_89, front_90, front_91, front_92, front_93, front_94, front_95, front_96, front_97, front_98, front_99, front_100, front_101, front_102, front_103, front_104, front_105, front_106, front_107, front_108, front_109, front_110, front_111, front_112, front_113, front_114, front_115, front_116, front_117, front_118, front_119, front_120, front_121, front_122, front_123, front_124, front_125, front_126, front_127, front_128, front_129, front_130, front_131, front_132, front_133, front_134, front_135, front_136, front_137, front_138, front_139, front_140, front_141, front_142, front_143, front_144, front_145, front_146, front_147, front_148, front_149, front_150, front_151};
const unsigned char *bar[2] = {bar_0, bar_1};

#ifdef FR
const char *pokemon_names[151] = {"BULBIZARRE","HERBIZARRE","FLORIZARRE","SALAMECHE","REPTINCEL","DRACAUFEU","CARAPUCE","CARABAFFE","TORTANK","CHENIPAN","CHRYSACIER","PAPILUSION","ASPICOT","COCONFORT","DARDARGNAN","ROUCOOL","ROUCOUPS","ROUCARNAGE","RATTATA","RATTATAC","PIAFABEC","RAPASDEPIC","ABO","ARBOK","PIKACHU","RAICHU","SABELETTE","SABLAIREAU","NIDORAN","NIDORINA","NIDOQUEEN","NIDORAN","NIDORINO","NIDOKING","MELOFEE","MELODELFE","GOUPIX","FEUNARD","RONDOUDOU","GRODOUDOU","NOSFERAPTI","NOSFERALTO","MYSTHERBE","ORTIDE","RAFFLESIA","PARAS","PARASECT","MIMITOSS","AEROMITE","TAUPIQUEUR","TRIOPIKEUR","MIAOUSS","PERSIAN","PSYKOKWAK","AKWAKWAK","FEROSINGE","COLOSSINGE","CANINOS","ARCANIN","PTITARD","TETARTE","TARTARD","ABRA","KADABRA","ALAKAZAM","MACHOC","MACHOPEUR","MACKOGNEUR","CHETIFLOR","BOUSTIFLOR","EMPIFLOR","TENTACOOL","TENTACRUEL","RACAILLOU","GRAVALANCH","GROLEM","PONYTA","GALOPA","RAMOLOSS","FLAGADOSS","MAGNETI","MAGNETON","CANARTICHO","DODUO","DODRIO","OTARIA","LAMANTINE","TADMORV","GROTADMORV","KOKIYAS","CRUSTABRI","FANTOMINUS","SPECTRUM","ECTOPLASMA","ONIX","SOPORIFIK","HYPNOMADE","KRABBY","KRABBOSS","VOLTORBE","ELECTRODE","NOEUNOEUF","NOADKOKO","OSSELAIT","OSSATUEUR","KICKLEE","TYGNON","EXCELANGUE","SMOGO","SMOGOGO","RHINOCORNE","RHINOFEROS","LEVEINARD","SAQUEDENEU","KANGOUREX","HYPOTREMPE","HYPOCEAN","POISSIRENE","POISSOROY","STARI","STAROSS","M. MIME","INSECATEUR","LIPPOUTOU","ELEKTEK","MAGMAR","SCARABRUTE","TAUROS","MAGICARPE","LEVIATOR","LOKHLASS","METAMORPH","EVOLI","AQUALI","VOLTALI","PYROLI","PORYGON","AMONITA","AMONISTAR","KABUTO","KABUTOPS","PTERA","RONFLEX","ARTIKODIN","ELECTHOR","SULFURA","MINIDRACO","DRACO","DRACOLOSSE","MEWTWO","MEW"};
#else
const char *pokemon_names[151] = {"BULBASAUR","IVYSAUR","VENUSAUR","CHARMANDER","CHARMELEON","CHARIZARD","SQUIRTLE","WARTORTLE","BLASTOISE","CATERPIE","METAPOD","BUTTERFREE","WEEDLE","KAKUNA","BEEDRILL","PIDGEY","PIDGEOTTO","PIDGEOT","RATTATA","RATICATE","SPEAROW","FEAROW","EKANS","ARBOK","PIKACHU","RAICHU","SANDSHREW","SANDSLASH","NIDORAN","NIDORINA","NIDOQUEEN","NIDORAN","NIDORINO","NIDOKING","CLEFAIRY","CLEFABLE","VULPIX","NINETALES","JIGGLYPUFF","WIGGLYTUFF","ZUBAT","GOLBAT","ODDISH","GLOOM","VILEPLUME","PARAS","PARASECT","VENONAT","VENOMOTH","DIGLETT","DUGTRIO","MEOWTH","PERSIAN","PSYDUCK","GOLDUCK","MANKEY","PRIMEAPE","GROWLITHE","ARCANINE","POLIWAG","POLIWHIRL","POLIWRATH","ABRA","KADABRA","ALAKAZAM","MACHOP","MACHOKE","MACHAMP","BELLSPROUT","WEEPINBELL","VICTREEBEL","TENTACOOL","TENTACRUEL","GEODUDE","GRAVELER","GOLEM","PONYTA","RAPIDASH","SLOWPOKE","SLOWBRO","MAGNEMITE","MAGNETON","FARFETCH'D","DODUO","DODRIO","SEEL","DEWGONG","GRIMER","MUK","SHELLDER","CLOYSTER","GASTLY","HAUNTER","GENGAR","ONIX","DROWZEE","HYPNO","KRABBY","KINGLER","VOLTORB","ELECTRODE","EXEGGCUTE","EXEGGUTOR","CUBONE","MAROWAK","HITMONLEE","HITMONCHAN","LICKITUNG","KOFFING","WEEZING","RHYHORN","RHYDON","CHANSEY","TANGELA","KANGASKHAN","HORSEA","SEADRA","GOLDEEN","SEAKING","STARYU","STARMIE","MR. MIME","SCYTHER","JYNX","ELECTABUZZ","MAGMAR","PINSIR","TAUROS","MAGIKARP","GYARADOS","LAPRAS","DITTO","EEVEE","VAPOREON","JOLTEON","FLAREON","PORYGON","OMANYTE","OMASTAR","KABUTO","KABUTOPS","AERODACTYL","SNORLAX","ARTICUNO","ZAPDOS","MOLTRES","DRATINI","DRAGONAIR","DRAGONITE","MEWTWO","MEW"};
#endif

#define SPRITE_W 72

#define PKM1_UI_W 96
#define PKM1_UI_H 16
#define PKM1_UI_X 14
#define PKM1_UI_Y 31
#define PKM1_X 120
#define PKM1_Y 6

#define PKM2_UI_W 96
#define PKM2_UI_H 26
#define PKM2_UI_X 90
#define PKM2_UI_Y 103
#define PKM2_X 8
#define PKM2_Y 62

#define PKM3_UI_W 200
#define PKM3_UI_H 58
#define PKM3_UI_X 1
#define PKM3_UI_Y 137

#ifdef FR
#define LVL_TXT ":N"
#else
#define LVL_TXT ":L"
#endif

#define MIN_VBAT 3.7
#define MAX_VBAT 4.1
#define INV_VBAT_DIF 2.5


void WatchyPokemon::drawWatchFace(){

    UiSDK::initScreen(display);
    const uint16_t fgColor = UiSDK::getWatchfaceFg(BASE_POLARITY);

#ifndef SIM
    readWorldTime();
#endif

    //Steps
    // Step counter managed by core Watchy
    uint32_t stepCount = int(sensor.getCounter() * .01);

    //Voltage
    float VBAT = getBatteryVoltage();
    uint32_t bat_level = VBAT < MIN_VBAT ? 0 : (int)(100 * INV_VBAT_DIF * (VBAT - MIN_VBAT));

    //Save battery life
#ifndef SIM
    WiFi.mode(WIFI_OFF);
    btStop();
#endif

    UiSDK::setFont(display, &FreeMonoBold7pt7b);
    UiSDK::setTextColor(display, fgColor);

    int pkm1_id = int(randomHour(0) * 151);
    bool pkm1_shiny = int(randomHour(8231) * 4096) == 0;
    int pkm2_id = int(randomDay(0) * 151);
    bool pkm2_shiny = int(randomDay(3872) * 4096) == 0;

    // PKM
    UiSDK::drawBitmap(display, PKM2_X, PKM2_Y, pokemon_back[pkm2_id], SPRITE_W, SPRITE_W, fgColor);
    UiSDK::drawBitmap(display, PKM2_UI_X, PKM2_UI_Y, pokemon_2, PKM2_UI_W, PKM2_UI_H, fgColor);

    UiSDK::setCursor(display, PKM2_UI_X + 10, PKM2_UI_Y - 11);
    UiSDK::print(display, pokemon_names[pkm2_id]);
    if (pkm2_shiny) {
        UiSDK::print(display, '*');
    }
      

    UiSDK::setCursor(display, PKM2_UI_X + 40, PKM2_UI_Y - 1);
    UiSDK::print(display, LVL_TXT);
    UiSDK::print(display, bat_level);

    // TODO change "color" of bar

    for (int8_t i = 0; i < int(60 - (currentTime.Hour * 60 + currentTime.Minute) / 24); i++)
    {
        UiSDK::drawBitmap(display, PKM2_UI_X + 30 + i, PKM2_UI_Y + 3, bar[i % 2], 8, 4, fgColor);
    }

    // DATE = PV
    UiSDK::setCursor(display, PKM2_UI_X + 40, PKM2_UI_Y + 19);
    if(currentTime.Day < 10){
        UiSDK::print(display, ' ');
    }
    UiSDK::print(display, currentTime.Day);
    UiSDK::print(display, "/ ");
    if(currentTime.Month < 10){
        UiSDK::print(display, ' ');
    }
    UiSDK::print(display, currentTime.Month);

    // ENEMY
    UiSDK::drawBitmap(display, PKM1_X, PKM1_Y, pokemon_front[pkm1_id] , SPRITE_W, SPRITE_W, fgColor);
    UiSDK::drawBitmap(display, PKM1_UI_X, PKM1_UI_Y, pokemon_1, PKM1_UI_W, PKM1_UI_H, fgColor);

    UiSDK::setCursor(display, PKM1_UI_X + 6, PKM1_UI_Y - 11);
    UiSDK::print(display, pokemon_names[pkm1_id]);
    if (pkm1_shiny) {
        UiSDK::print(display, '*');
    }

    UiSDK::setCursor(display, PKM1_UI_X + 36, PKM1_UI_Y - 1);
    UiSDK::print(display, LVL_TXT);
    UiSDK::print(display, stepCount);

    for (int8_t i = 0; i < (60 - currentTime.Minute); i++)
    {
        UiSDK::drawBitmap(display, PKM1_UI_X + 26 + i, PKM1_UI_Y + 3, bar[i % 2], 8, 4, fgColor);
    }

    // BOTTOM
    UiSDK::drawBitmap(display, PKM3_UI_X, PKM3_UI_Y, pokemon_3, PKM3_UI_W, PKM3_UI_H, fgColor);

    // HOUR
    UiSDK::setFont(display, &FreeMonoBold10pt7b);
    UiSDK::setCursor(display, PKM3_UI_X + 13, PKM3_UI_Y + 34);
    if(currentTime.Hour < 10){
        UiSDK::print(display, '0');
    }
    UiSDK::print(display, currentTime.Hour);
    UiSDK::print(display, ':');
    if(currentTime.Minute < 10){
        UiSDK::print(display, '0');
    }
    UiSDK::print(display, currentTime.Minute);

    // CURSOR
    int pos = int(randomMinute(0) * 4);
    int posX = pos % 2;
    int posY = int(pos / 2);
    #ifdef FR
    UiSDK::drawBitmap(display, PKM3_UI_X + 85 + posX * 59, PKM3_UI_Y + 16 + posY * 20, cursor, 8, 9, fgColor);
    #else
    UiSDK::drawBitmap(display, PKM3_UI_X + 89 + posX * 61, PKM3_UI_Y + 16 + posY * 20, cursor, 8, 9, fgColor);
    #endif
}

double WatchyPokemon::randomMinute(uint32_t d)
{
    uint32_t seed = currentTime.Year;
    seed = seed * 12 + currentTime.Month;
    seed = seed * 31 + currentTime.Day;
    seed = seed * 24 + currentTime.Hour;
    seed = seed * 60 + currentTime.Minute;
    seed += d;
    

    double v = pow(seed, 6.0 / 7.0);
    v *= sin(v) + 1;

    return v - floor(v);
}

double WatchyPokemon::randomHour(uint32_t d)
{
    uint32_t seed = currentTime.Year;
    seed = seed * 12 + currentTime.Month;
    seed = seed * 31 + currentTime.Day;
    seed = seed * 24 + currentTime.Hour;
    seed += d;

    double v = pow(seed, 6.0 / 7.0);
    v *= sin(v) + 1;

    return v - floor(v);
}

double WatchyPokemon::randomDay(uint32_t d)
{
    uint32_t seed = currentTime.Year;
    seed = seed * 12 + currentTime.Month;
    seed = seed * 31 + currentTime.Day;
    seed += d;

    double v = pow(seed, 6.0 / 7.0);
    v *= sin(v) + 1;

    return v - floor(v);
}

void showWatchFace_Pokemon(Watchy &watchy) {
    WatchyPokemon face(watchy.settings);
    face.currentTime = watchy.currentTime;
    face.drawWatchFace();
}
