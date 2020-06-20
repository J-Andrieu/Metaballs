#ifndef CMDPARSER_H
#define CMDPARSER_H

#include <string>
#include <cstring>
#include <exception>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <cstdlib>

/** CMDParser class for parsing command line
 *  @class CMDParser
 * 
 *  @note Only handles the following types: int, double, bool, std::string
 */
class CMDParser {
public:
    CMDParser();

    ///Enum to define listed argument length type
    typedef enum {
        STATIC_LENGTH, //only the previously defined number of arguments are accepted
        VARIABLE_LENGTH, //any length of extra arguments may be read in (can only be one, must be final parameter entered)
        DEFINED_LENGTH //accepts an integer to define the number of variables
    } argType;

    /** Function to bind variables to flags
     *  @param flag The string that identifies this variable on the command line.
     *  @param destination The variable to store the value from the command line.
     *  @param numEntries The number of values expected to be stored in destination.
     *  @param description The description to be used in the help menu.
     * 
     *  @note This function is templated in order to properly 
     *        typecast from void* to the correct datatype of destination
     */
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

    /** Function to bind variables to flags (intended for non-predetermined length parameters)
     *  @param flag The string that identifies this variable on the command line.
     *  @param destination The variable to store the value from the command line.
     *  @param description The description to be used in the help menu.
     *  @param lengthType Determines the intended behaviour for readin in a list of values.
     *  @param numEntries The number of values expected to be stored in destination.
     * 
     *  @note This function is templated in order to properly 
     *        typecast from void* to the correct datatype of destination
     */
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

    //Enum used to internally determine type stored
    typedef enum {
        INT,
        DOUBLE,
        STRING,
        BOOL
    } datatype;

    //Struct for storing an argument
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

