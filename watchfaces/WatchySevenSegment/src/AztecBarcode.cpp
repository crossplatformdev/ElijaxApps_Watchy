#include <Arduino.h>
#include "../../../src/watchy/Watchy.h"
#include "../../../src/watchy/Display.h" 

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <bitset>
#include <sstream>
#include <iterator>


#include "AztecBarcode.h"

AztecCharacter aztecCharacterTable[] = {
    {DIGIT, DATA, '0', 2, 4},
    {DIGIT, DATA, '1', 3, 4},
    {DIGIT, DATA, '2', 4, 4},
    {DIGIT, DATA, '3', 5, 4},
    {DIGIT, DATA, '4', 6, 4},
    {DIGIT, DATA, '5', 7, 4},
    {DIGIT, DATA, '6', 8, 4},
    {DIGIT, DATA, '7', 9, 4},
    {DIGIT, DATA, '8', 10, 4},
    {DIGIT, DATA, '9', 11, 4},
    {UPPER, DATA, ',', 12, 4},
    {UPPER, DATA, '.', 13, 4},
    {DIGIT, DATA, ' ', 1,  4}
};    

ModeSwitch modeSwitchTable[] = {
    {UPPER, DIGIT, 30, 5},
    {DIGIT, UPPER, 14, 4},
};

AztecBarcode::AztecBarcode(std::string text,std::string label, GxEPD2_BW <WatchyDisplay, WatchyDisplay::HEIGHT> *display) {
    m_text = text;
    m_label = label;
    m_display = display;
    // lets initialize the matrix to all false
    for (int i = 0; i < AZTEC_DIMENSION; i++) {
        for (int j = 0; j < AZTEC_DIMENSION; j++) {
            m_matrix[i][j] = false;
        }
    }
    encodeBarcode();
}

AztecBarcode::AztecBarcode(String text, String label, GxEPD2_BW <WatchyDisplay, WatchyDisplay::HEIGHT> *display)
    : AztecBarcode(std::string(text.c_str()), std::string(label.c_str()), display) {
}

AztecBarcode::~AztecBarcode() {
    // Add destructor code here
}

void AztecBarcode::addFinder() {
    for (int i = m_finderSize * -1; i < m_finderSize; i++) {
        for (int j = m_finderSize * -1; j < m_finderSize; j++) {
            if ( (std::max(abs(i),abs(j)) + 1) % 2) {
                m_matrix[m_centre + i][m_centre + j] = true;
            } else {
                m_matrix[m_centre + i][m_centre + j] = false;
            }
        }
    }
}

void AztecBarcode::addReferenceGrid() {
    // not required for compact code so we'll leave for now
}

void AztecBarcode::addOrientationMarks() {
    // std::cout << "Adding Orientation Marks" << std::endl;
    // explicity setting true and false here, for my own understanding!
    // left top
    m_matrix[m_centre - m_finderSize][m_centre - m_finderSize] = true;       // very top, very left
    m_matrix[m_centre - m_finderSize + 1][m_centre - m_finderSize] = true;    // top, very left
    m_matrix[m_centre - m_finderSize][m_centre - m_finderSize + 1] = true;    // very top, left
    // right top
    m_matrix[m_centre + m_finderSize][m_centre - m_finderSize] = true;       // very top, very right
    m_matrix[m_centre + m_finderSize][m_centre - m_finderSize + 1] = true;    // top, very right 
    m_matrix[m_centre + m_finderSize - 1][m_centre - m_finderSize] = false;    // very top, right
    // right down
    m_matrix[m_centre + m_finderSize][m_centre + m_finderSize] = false;       // very down, very right
    m_matrix[m_centre + m_finderSize][m_centre + m_finderSize - 1] = true;     // down, very right
    m_matrix[m_centre + m_finderSize - 1][m_centre + m_finderSize] = false;     // very down, right
}

AztecCharacter AztecBarcode::getAztecCharacter(char c) {
    for (long unsigned int i = 0; i < sizeof(aztecCharacterTable) / sizeof(aztecCharacterTable[0]); i++) {
        if (aztecCharacterTable[i].character == c) {
            return aztecCharacterTable[i];
        }
    }
    // return a default character if not found
    AztecCharacter aztecCharacter = {DIGIT, DATA, ' ', 1, 4};
    return aztecCharacter;
}

AztecCharacter AztecBarcode::getModeSwitch(AztecCharacterMode from, AztecCharacterMode to) {
    int value = 0, bits = 0;
    for (long unsigned int i = 0; i < sizeof(modeSwitchTable) / sizeof(modeSwitchTable[0]); i++) {
        if (modeSwitchTable[i].from == from && modeSwitchTable[i].to == to) {
            value = modeSwitchTable[i].value;
            bits = modeSwitchTable[i].bits;
        }
    }
    // return a default character if not found
    AztecCharacter aztecCharacter = {from, CONTROL, ' ', value, bits};
    return aztecCharacter;
}

void AztecBarcode::addData() {
    // iterate over m_text and add to m_outputCharacters
    // std::cout << "Adding Data" << std::endl;
    // std::cout << "Text: " << m_text << std::endl;
    AztecCharacter aztecCharacter = getModeSwitch(UPPER, DIGIT);
    m_outputCharacters.push_back(aztecCharacter);

    for (long unsigned int i = 0; i < m_text.length(); i++) {
        char c = m_text[i];
        aztecCharacter = getAztecCharacter(c);
        m_outputCharacters.push_back(aztecCharacter);
    }
}   

void AztecBarcode::generateOutputBits() {
    for (long unsigned int i = 0; i < m_outputCharacters.size(); i++) {
        AztecCharacter aztecCharacter = m_outputCharacters[i];
        int mask = 1 << (aztecCharacter.bits - 1);
        for (int j = 0; j < aztecCharacter.bits; j++) {
            m_outputBits.push_back(aztecCharacter.value & mask);
            mask = mask >> 1;
        }
    }
}

std::vector<int> AztecBarcode::get_data_codewords(long unsigned int codeword_size) {
    std::vector<int> codewords;
    std::vector<bool> sub_bits;

    for (bool bit : m_outputBits) { 
        sub_bits.push_back(bit);

        // If first bits of sub sequence are zeros, add 1 as the last bit
        if (sub_bits.size() == codeword_size - 1 && 
            std::all_of(sub_bits.begin(), sub_bits.end(), [](bool b) { return !b; })) {
            sub_bits.push_back(true); // Add '1'
        }

        // If first bits of sub sequence are ones, add 0 as the last bit
        if (sub_bits.size() == codeword_size - 1 && 
            std::all_of(sub_bits.begin(), sub_bits.end(), [](bool b) { return b; })) {
            sub_bits.push_back(false); // Add '0'
        }

        // Convert bits to decimal int and add to result codewords
        if (sub_bits.size() >= codeword_size) {
            int codeword = 0;
            for (size_t i = 0; i < sub_bits.size(); ++i) {
                if (sub_bits[i]) {
                    codeword += (1 << (sub_bits.size() - 1 - i));
                }
            }
            codewords.push_back(codeword);
            sub_bits.clear();
        }
    }

    if (!sub_bits.empty()) {
        // Update and add final bits
        while (sub_bits.size() < codeword_size) {
            sub_bits.push_back(true); // Pad with '1's
        }

        // Change final bit to zero if all bits are ones
        if (std::all_of(sub_bits.begin(), sub_bits.end(), [](bool b) { return b; })) {
            sub_bits.back() = false; // Change last bit to '0'
        }

        // Convert to decimal
        int codeword = 0;
        for (size_t i = 0; i < sub_bits.size(); ++i) {
            if (sub_bits[i]) {
                codeword += (1 << (sub_bits.size() - 1 - i));
            }
        }
        codewords.push_back(codeword);
    }

    return codewords;
}

// Helper function to perform multiplication in GF(2^m)
int AztecBarcode::prod(int a, int b, const std::unordered_map<int, int>& log, const std::unordered_map<int, int>& alog, int gf) {
    if (a == 0 || b == 0) {
        return 0;
    }
    return alog.at((log.at(a) + log.at(b)) % (gf - 1));
}

void AztecBarcode::reed_solomon(std::vector<int>& wd, int nd, int nc, int gf, int pp) {
    // Generate log and anti-log tables
    /*
    std::cout << "Reed Solomon start nd "
                << nd
                << " nc: "
                << nc
                << " gf: "
                << gf
                << " pp: " 
                << pp
                << std::endl;
    */
    std::unordered_map<int, int> log;
    std::unordered_map<int, int> alog;
    log[0] = 1 - gf;
    alog[0] = 1;
    for (int i = 1; i < gf; ++i) {
        alog[i] = alog[i - 1] * 2;
        if (alog[i] >= gf) {
            alog[i] ^= pp;
        }
        log[alog[i]] = i;
    }
    // std::cout << "Log and alog tables generated" << std::endl;

    // Generate polynomial coefficients
    std::vector<int> c(nc + 1, 0);
    c[0] = 1;
    for (int i = 1; i <= nc; ++i) {
        c[i] = 0;
    }
    for (int i = 1; i <= nc; ++i) {
        c[i] = c[i - 1];
        for (int j = i - 1; j > 0; --j) {
            if (alog.find(i) == alog.end() || log.find(c[j]) == log.end()) {
                std::cerr << "Error: alog or log value out of range" << std::endl;
                return;
            }
            c[j] = c[j - 1] ^ prod(c[j], alog[i], log, alog, gf);
        }
        if (alog.find(i) == alog.end() || log.find(c[0]) == log.end()) {
            std::cerr << "Error: alog or log value out of range" << std::endl;
            return;
        }
        c[0] = prod(c[0], alog[i], log, alog, gf);
    }
    // std::cout << "Polynomial coefficients generated" << std::endl;

    // Generate codewords
    wd.resize(nd + nc);
    for (int i = nd; i < nd + nc; ++i) {
        wd[i] = 0;
    }
    for (int i = 0; i < nd; ++i) {
        assert(wd[i] >= 0 && wd[i] < gf);
        int k = wd[nd] ^ wd[i];
        for (int j = 0; j < nc; ++j) {
            wd[nd + j] = prod(k, c[nc - j - 1], log, alog, gf);
            if (j < nc - 1) {
                wd[nd + j] ^= wd[nd + j + 1];
            }
        }
    }
}

void AztecBarcode::applyCodeWords() {

    int ring_radius = 5;
    int num = 2;
    AztecSide side = TOP;
    int layer_index = 0;
    int pos_x = m_centre - ring_radius;
    int pos_y = m_centre - ring_radius - 1;

    std::string full_bits;
    for (int cw : m_codewords) {
        std::string bits = std::bitset<32>(cw).to_string().substr(32 - AZTEC_DATA_SIZE);
        full_bits += bits;
    }
    std::reverse(full_bits.begin(), full_bits.end());
    // std::cout << "Full bits: " << full_bits << std::endl;

    for (size_t i = 0; i < full_bits.length(); i += 2) {
        num += 1;
        int max_num = (ring_radius * 2) + (layer_index * 4) + 4;
        // std::cout << "Num: " << num << " Max num: " << max_num << std::endl;
        // std::cout << "Pos x: " << pos_x << " Pos y: " << pos_y << std::endl;
        std::vector<bool> bits_pair;
        for (size_t j = i; j < i + 2 && j < full_bits.length(); ++j) {
            bits_pair.push_back(full_bits[j] == '1' ? true : false);
        }

        if (layer_index >= 2) {
            // throw std::runtime_error("Maximum layer count for current size is exceeded!");
            return;
        }
        if (side == TOP) {
            // move right
            // std::cout << "Pos x: " << pos_x << " Pos y: " << pos_y << std::endl;
            m_matrix[pos_x][pos_y] = bits_pair[0];
            m_matrix[pos_x][pos_y - 1] = bits_pair[1];
            pos_x += 1;
            if (num > max_num) {
                num = 2;
                side = RIGHT;
                pos_x -= 2;
                pos_y += 1;
            }
        } else if (side == RIGHT) {
            // move down
            m_matrix[pos_x][pos_y] = bits_pair[0];
            m_matrix[pos_x + 1][pos_y] = bits_pair[1];
            pos_y += 1;
            if (num > max_num) {
                num = 2;
                side = BOTTOM;
                pos_x -= 1;
                pos_y -= 2;
            }
        } else if (side == BOTTOM) {
            //move left
            m_matrix[pos_x][pos_y] = bits_pair[0];
            m_matrix[pos_x][pos_y + 1] = bits_pair[1];
            pos_x -= 1;
            if (num > max_num) {
                num = 2;
                side = LEFT;
                pos_x += 2;
                pos_y -= 1;
            }
        } else if (side == LEFT) {
            // move up
            m_matrix[pos_x][pos_y] = bits_pair[0];
            m_matrix[pos_x - 1][pos_y] = bits_pair[1];
            pos_y -= 1;
            if (num > max_num) {
                num = 2;
                side = TOP;
                layer_index += 1;
                pos_x -= 1;
            }
        }
    }
}

std::vector<int>AztecBarcode::getModeMessage(){
    std::string mode_word;
    std::vector<int> init_codewords;
    int total_cw_count;

    mode_word = std::bitset<2>(LAYER_COUNT - 1).to_string() +
                std::bitset<6>(m_dw_count - 1).to_string();
    for (size_t i = 0; i < 8; i += 4) {
        std::string sub_word = mode_word.substr(i, 4);
        init_codewords.push_back(std::stoi(mode_word.substr(i, 4), nullptr, 2));
    }
    total_cw_count = 7;

    int init_cw_count = init_codewords.size();
    std::vector<int> codewords = init_codewords;
    codewords.resize(total_cw_count,0);
    reed_solomon(codewords,
                init_cw_count,
                total_cw_count - init_cw_count,
                16,
                AZTEC_MESSAGE_POLY);

    return codewords;
}

void AztecBarcode::addModeInfo() {

    auto mode_data_values = getModeMessage();
    std::string mode_data_bits;
    for (int v : mode_data_values) {
        mode_data_bits += std::bitset<4>(v).to_string();
    }

    int ring_radius = 5;
    int side_size = 7;

    std::istringstream bits_stream(mode_data_bits);
    int x = 0, y = 0, index = 0;
    while (true) {
        // Read one bit
        char bit;
        bits_stream >> bit;
        if (!bits_stream) break;

        // Determine position based on index
        if (0 <= index && index < side_size) {
            // Top
            x = index + 2 - ring_radius;
            y = -ring_radius;
        } else if (side_size <= index && index < side_size * 2) {
            // Right
            x = ring_radius;
            y = (index % side_size) + 2 - ring_radius;
        } else if (side_size * 2 <= index && index < side_size * 3) {
            // Bottom
            x = ring_radius - (index % side_size) - 2;
            y = ring_radius;
        } else if (side_size * 3 <= index && index < side_size * 4) {
            // Left
            x = -ring_radius;
            y = ring_radius - (index % side_size) - 2;
        }

        // Set pixel in the matrix
        m_matrix[m_centre + x][m_centre + y] = (bit == '1') ? true : false;
        index++;
    }
   
}

void AztecBarcode::encodeBarcode() {
    
    addFinder();
    addReferenceGrid();
    addOrientationMarks();
    addData();
    generateOutputBits(); 
    m_dataCodewords = get_data_codewords(6);
    m_dw_count = m_dataCodewords.size();
    m_codewords = m_dataCodewords; // Let's keep a copy of the data codewords
                                    // and work on the codewords to generate the output
    reed_solomon(m_codewords,
                m_codewords.size(),
                CODEWORD_SIZE - m_codewords.size(),
                std::pow(2, AZTEC_DATA_SIZE),
                AZTEC_PRIME_POLY);
    applyCodeWords();
    addModeInfo();
}

void AztecBarcode::draw(){
    // Let's add a title
    int16_t x1=0,y1=0,pixelSize;
    uint16_t textWidth,textHeight;

    m_display->setFont(&FreeMonoBold9pt7b);
    m_display->setTextColor(GxEPD_BLACK);
    m_display->getTextBounds(m_label.c_str(),0,0,&x1,&y1,&textWidth,&textHeight);
    m_display->fillRect(0,0,textWidth,textHeight,GxEPD_WHITE);
    m_display->setCursor(0,textHeight);
    m_display->print(m_label.c_str());    

    // Let's calculate the pixel size
    pixelSize = (m_display->width() / AZTEC_DIMENSION) - 2;
    int xoffset = (m_display->width() - (AZTEC_DIMENSION * pixelSize)) / 2;
    int yoffset = (m_display->height() - (AZTEC_DIMENSION * pixelSize)) / 2;

    // Let's draw the barcode
    for (int i = 0; i < AZTEC_DIMENSION; i++) {
        for (int j = 0; j < AZTEC_DIMENSION; j++) {
            if (m_matrix[i][j]) {
                m_display->fillRect(xoffset + i*pixelSize,yoffset + j*pixelSize,pixelSize,pixelSize,GxEPD_BLACK);
            } 
            // Assume white background....
        }
    }

    // and let's print the barcode value at the bottom of the display
    m_display->getTextBounds(m_text.c_str(),0,0,&x1,&y1,&textWidth,&textHeight);
    m_display->fillRect(0,m_display->height()-textHeight,textWidth,textHeight,GxEPD_WHITE);
    m_display->setCursor(0,m_display->height()-1); // seems to clip the bottom of the text if we don't do this
    m_display->print(m_text.c_str());

}
