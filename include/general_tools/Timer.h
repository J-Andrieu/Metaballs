/************************************************
 * @file Timer.h
 *
 * @brief Definition file for the Timer class
 *
 * @details Specifies types as well as the
 *          Timer class
 *
 ************************************************/
#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <ctime>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <thread>

typedef std::chrono::high_resolution_clock hresClock;
typedef hresClock::time_point hresClockTimePoint;

namespace Timers {
    void logNamedTimers();
    void setLogFile(std::string);

    typedef struct {
        std::string m_scopeName;
        long long* m_executionTime;
    } timerScope;

    static std::vector<timerScope> named_timers;
    static std::string out_file = "";
    static bool toScreen = false;
}

class Timer {
public:
    //class constructor
    Timer();
    Timer(long long& out);//scope resolution timer
    Timer(std::string scopeName);//named scope resolution timer
    ~Timer();

    //retrieve relative time
    void start(); //sets internal time point
    long long getMicrosecondsElapsed(); //returns microseconds since timer was created, or last start() call
    static long long getMicrosecondsSince (hresClockTimePoint t1); //returns microseconds since provided time point

    //wait
    bool compareElapsedMicroseconds (const long long& microseconds); //returns true if specified time has elapsed;
    static inline void sleepMicroseconds (const long long& microseconds); //waits for specified time period
    template<typename Rep, typename Period>
    static inline void wait(std::chrono::duration<Rep, Period> length) { //don't forget abt std::chrono's literals >.>
      //hresClockTimePoint end = hresClock::now() + length;
      //while(hresClock::now() < end);
      std::this_thread::sleep_for(length);
    }

    //retrieve current time
    static hresClockTimePoint getCurrentTime(); //returns current time as hresClockTimePoint

private:
    hresClockTimePoint m_initialTime;
    long long* m_out;
};


#endif

