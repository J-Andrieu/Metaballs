#ifndef CMDPARSER_H
#define CMDPARSER_H

#include <string>
#include <cstring>
#include <exception>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <cstdlib>

class CMDParser {
public:
    CMDParser();

    typedef enum {
        STATIC_LENGTH, //only the previously defined number of arguments are accepted
        VARIABLE_LENGTH, //any length of axtra arguments may be read in (can only be one, must be final parameter entered)
        DEFINED_LENGTH //accepts an integer to define the number of variables
    } argType;

    template <typename T>
    void bindVar (std::string flag, T& destination, size_t numEntries, std::string description) {
        datatype type;
        if (std::is_same<T, int>() || std::is_convertible<T, int*>()) {
            type = INT;
        } else if (std::is_same<T, std::string>() || std::is_convertible<T, std::string*>()) {
            type = STRING;
        } else if (std::is_same<T, double>() || std::is_convertible<T, double*>()) {
            type = DOUBLE;
        } else if (std::is_same<T, bool>() || std::is_convertible<T, bool*>()) {
            type = BOOL;
        }
        m_params.push_back ({type, (void*) &destination, numEntries, description, flag, STATIC_LENGTH, nullptr});
    }

    template <typename T>
    void bindVar (std::string flag, T& destination, std::string description, argType lengthType, size_t &numEntries) {
        datatype type;
        if (std::is_same<T, int>() || std::is_convertible<T, int*>()) {
            type = INT;
        } else if (std::is_same<T, std::string>() || std::is_convertible<T, std::string*>()) {
            type = STRING;
        } else if (std::is_same<T, double>() || std::is_convertible<T, double*>()) {
            type = DOUBLE;
        } else if (std::is_same<T, bool>() || std::is_convertible<T, bool*>()) {
            type = BOOL;
        }
        numEntries = 0;
        T* arrayLoc = &destination;
        m_params.push_back ({type, (void*) arrayLoc, 0, description, flag, lengthType, &numEntries});
    }

    bool parse (int argc, char* argv[]);

    //the default is for CMDParser to generate the help menu
    void setHelpMessage(std::string msg = "");
    void generateHelp(bool val = true);

    void printHelp();

private:
    typedef enum {
        INT,
        DOUBLE,
        STRING,
        BOOL
    } datatype;

    typedef struct {
        datatype _type;
        void* _dest;
        size_t _size;
        std::string _desc;
        std::string _flag;
        argType _lenType;
        size_t* _newLen;
    } _container;

    friend bool operator== (_container A, std::string B);

    std::vector<_container> m_params;

    bool help;
    bool generate_help;
    std::string custom_help;
};
#endif // CMDPARSER_H

