#ifndef TERM_FORMATTER_H
#define TERM_FORMATTER_H

#include <string>
#include <sstream>
#include <variant>

namespace TermFormatter {
    enum Mod {
        //format
        Bright = 1,
        Dim = 2,
        Underlined = 4,
        Blink = 5,
        Reverse = 7,
        Hidden = 8,
        //format reset
        ResetAll = 0,
        ResetBright = 21,
        ResetDim = 22,
        ResetUnderlined = 24,
        ResetBlink = 25,
        ResetReversed = 27,
        ResetHidden = 28,
        //forground colors
        FG_Default = 39,
        FG_Black = 30,
        FG_Red = 31,
        FG_Green = 32,
        FG_Yellow = 33,
        FG_Blue = 34,
        FG_Magenta = 35,
        FG_Cyan = 36,
        FG_LightGray = 37,
        FG_DarkGray = 90,
        FG_LightRed = 91,
        FG_LightGreen = 92,
        FG_LightYellow = 93,
        FG_LightBlue = 94,
        FG_LightMagenta = 95,
        FG_LightCyan = 96,
        FG_White = 97,
        //background colors
        BG_Default = 49,
        BG_Black = 40,
        BG_Red = 41,
        BG_Green = 42,
        BG_Yellow = 43,
        BG_Blue = 44,
        BG_Magenta = 45,
        BG_Cyan = 46,
        BG_LightGray = 47,
        BG_DarkGray = 100,
        BG_LightRed = 101,
        BG_LightGreen = 102,
        BG_LightYellow = 103,
        BG_LightBlue = 104,
        BG_LightMagenta = 105,
        BG_LightCyan = 106,
        BG_White = 107
    };

    enum FG_BG {
        FG = 38,
        BG = 48
    };

    class CustomColor {
    public:
        friend class Formatter;

        //color at index in the 88/256 color tble
        CustomColor(FG_BG fg_bg, uint8_t index) {
            std::ostringstream init_str;
            init_str << fg_bg << ";5;" << (int) index;
            m_colorString = init_str.str();
        }

        //custom RGB
        CustomColor(FG_BG fg_bg, uint8_t r, uint8_t g, uint8_t b) {
            std::ostringstream init_str;
            init_str << fg_bg << ";2;" << (int) r << ";" << (int) g << ";" << (int) b;
            m_colorString = init_str.str();
        }

        friend std::ostream& operator<<(std::ostream& out, const CustomColor& color) {
            return out << "\x1b[" << color.m_colorString << "m";
        }
    private:
        std::string m_colorString;
    };

    class Formatter {
    public:
        Formatter(std::initializer_list<std::variant<Mod, CustomColor>> modifiers) {
            std::ostringstream init_str;
            init_str << "\x1b[";
            bool init = true;
            for (auto m : modifiers) {
                if (!init) {
                    init_str << ";";
                }
                if (std::holds_alternative<Mod>(m)) {
                    init_str << std::get<Mod>(m);
                } else if (std::holds_alternative<CustomColor>(m)) {
                    init_str << std::get<CustomColor>(m).m_colorString;
                }
                init = false;
            }
            init_str << "m";
            m_modString = init_str.str();
        }

        std::string getString() const {
            return m_modString;
        }
        
        operator std::string() const {
            return m_modString;
        }

        friend std::ostream& operator<<(std::ostream& out, const Formatter& formatter) {
            return out << formatter.m_modString;
        }

    private:
        std::string m_modString;
    };
};

#endif /* TERM_FORMATTER_H */