#ifndef CMDPARSER_CPP
#define CMDPARSER_CPP

#include "CMDParser.h"

CMDParser::CMDParser() {
    m_params.resize (0);
    bindVar<bool> ("-h", help, 0, "Displays the help page");
    generate_help = true;
    custom_help = "";
}

bool operator== (CMDParser::_container A, std::string B) {
    return A._flag == B;
}

bool CMDParser::parse (int argc, char* argv[]) {
    std::sort (m_params.begin(), m_params.end(), [] (auto A, auto B) {
        return A._flag < B._flag;
    });
    for (int i = 1; i < argc; i++) {
        auto pos = std::find (m_params.begin(), m_params.end(), std::string (argv[i]));
        if (pos == std::end (m_params)) {
            printf ("The provided flag (%s) is invalid\n", argv[i]);
            printHelp();
            return false;
        } else if (pos->_flag == "-h") {
            printHelp();
            return false;
        }

        if (pos->_lenType == DEFINED_LENGTH) {
            try {
                *(pos->_newLen) = std::stoi(argv[++i]);
            } catch (std::exception& e) {
                printf("An invalid parameter (%s) was passed to %s resulting in the following exception:\n%s\n", argv[i], pos->_flag.c_str(), e.what());
                printHelp();
                return false;
            }
            pos->_size = *(pos->_newLen);
        } else if (pos->_lenType == VARIABLE_LENGTH) {
            pos->_size = argc - i - 1;
            *(pos->_newLen) = pos->_size;
        }

        try {
            switch (pos->_type) {
                case INT: {
                        if (pos->_lenType != STATIC_LENGTH) {
                            ((int**) pos->_dest)[0] = new int[pos->_size];
                            pos->_dest = (void*) ((int**)pos->_dest)[0];
                        }
                        int* dest = (int*) pos->_dest;
                        for (size_t j = 0; j < pos->_size; j++) {
                            dest[j] = std::stoi (argv[++i]);
                        }
                    }
                    break;
                case DOUBLE: {
                        if (pos->_lenType != STATIC_LENGTH) {
                            ((double**) pos->_dest)[0] = new double[pos->_size];
                            pos->_dest = (void*) ((double**)pos->_dest)[0];
                        }
                        double* dest = (double*) pos->_dest;
                        for (size_t j = 0; j < pos->_size; j++) {
                            dest[j] = std::stof (argv[++i]);
                        }
                    }
                    break;
                case STRING: {
                        if (pos->_lenType != STATIC_LENGTH) {
                            ((std::string**) pos->_dest)[0] = new std::string[pos->_size];
                            pos->_dest = (void*) ((std::string**)pos->_dest)[0];
                        }
                        std::string* dest = (std::string*) pos->_dest;
                        for (size_t j = 0; j < pos->_size; j++) {
                            dest[j] = std::string (argv[++i]);
                        }
                    }
                    break;
                case BOOL: {
                        if (pos->_lenType != STATIC_LENGTH) {
                            ((bool**) pos->_dest)[0] = new bool[pos->_size];
                            pos->_dest = (void*) ((bool**)pos->_dest)[0];
                        }
                        bool* dest = (bool*) pos->_dest;
                        if (pos->_size > 0) {
                            for (size_t j = 0; j < pos->_size; j++) {
                                i++;
                                if (strcmp(argv[i], "true") == 0 || strcmp(argv[i], "1") == 0 || strcmp(argv[i], "t") == 0) {
                                    dest[j] = true;
                                } else {
                                    dest[j] = false;
                                }
                            }
                        } else {
                            dest[0] = true;
                        }
                    }
                    break;
            }
        } catch (std::exception& e) {
            printf("An invalid parameter (%s) was passed to %s resulting in the following exception:\n%s\n", argv[i], pos->_flag.c_str(), e.what());
            printHelp();
            return false;
        }
    }
    return true;
}

void CMDParser::printHelp() {
    if (custom_help.length() != 0) {
        printf("%s\n", custom_help.c_str());
    }

    if (generate_help) {
        if (custom_help.length() == 0) {
            printf ("Displaying Help Page:\n\n");
        }
        int flagPadding = 0;
        for (auto param : m_params) {
            if (flagPadding < param._flag.size()) {
                flagPadding = param._flag.size();
            }
        }
        flagPadding++;
        flagPadding *= -1;
        std::string format1 = std::string ("% ") + std::to_string (flagPadding) + std::string ("s|\t%s (takes %s argument%s)\n");
        std::string format2 = std::string ("% ") + std::to_string (flagPadding) + std::string ("s|\t%s %s\n");
        for (auto param : m_params) {
            if (param._lenType == STATIC_LENGTH) {
                printf (format1.c_str(), param._flag.c_str(), param._desc.c_str(), (param._size == 0 ? "no" : std::to_string (param._size).c_str()), (param._size == 1 ? "" : "s"));
            } else {
                printf (format2.c_str(), param._flag.c_str(), param._desc.c_str(), param._lenType == VARIABLE_LENGTH ? "(Will process the rest of the input)" : "(Must define number of elements before list)");
            }
        }
        printf ("\n");
    }
}
#endif // CMDPARSER_CPP

