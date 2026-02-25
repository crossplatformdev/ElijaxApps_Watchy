#ifndef AZTECBARCODE_H
#define AZTECBARCODE_H

#include <Arduino.h>
#include "../../../watchy/Watchy.h"

#include <string>
#include <vector>
#include <unordered_map>

// Aztec code is level 2, compact. These values are appropriate for this level
#define AZTEC_DIMENSION 19
#define AZTEC_DATA_SIZE 6
#define CODEWORD_SIZE 40
#define AZTEC_PRIME_POLY 67
#define LAYER_COUNT 2
#define AZTEC_MESSAGE_POLY 19


enum AztecCharacterMode {
    UPPER,
    LOWER,
    DIGIT,
    MIXED,
    PUNCT
};

enum AztecSide {
    TOP,
    RIGHT,
    BOTTOM,
    LEFT
};

enum AztecCharacterType {
    DATA,
    CONTROL
};

struct AztecCharacter {
    AztecCharacterMode mode;
    AztecCharacterType type;
    char character;
    int value;
    int bits;
};

struct ModeSwitch {
    AztecCharacterMode from;
    AztecCharacterMode to;
    int value;
    int bits;
};

class AztecBarcode {
    public:
        AztecBarcode(std::string text,std::string label, GxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT> *display);
        AztecBarcode(String text,String label, GxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT> *display);
        ~AztecBarcode();
        void draw();
    
    private:
        std::string m_text, m_label;
        GxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT>* m_display;
        bool m_matrix[AZTEC_DIMENSION][AZTEC_DIMENSION];
        int m_finderSize = 5; // we are using a compact code for now
        int m_centre = AZTEC_DIMENSION / 2;
        int m_dw_count = 0;
        std::vector<AztecCharacter> m_outputCharacters;
        std::vector<bool> m_outputBits;
        std::vector<int> m_dataCodewords, m_codewords;
        void addFinder();
        void addReferenceGrid();
        void addOrientationMarks();
        AztecCharacter getAztecCharacter(char c);
        AztecCharacter getModeSwitch(AztecCharacterMode from, AztecCharacterMode to);
        void addData();
        void generateOutputBits();
        std::vector<int>get_data_codewords(long unsigned int codeword_size);
        int prod(int a, int b, const std::unordered_map<int, int>& log, const std::unordered_map<int, int>& alog, int gf);
        void reed_solomon(std::vector<int>& wd, int nd, int nc, int gf, int pp);
        void applyCodeWords();
        std::vector<int>getModeMessage();
        void addModeInfo();
        void encodeBarcode();
    };
    



#endif // AZTECBARCODE_H