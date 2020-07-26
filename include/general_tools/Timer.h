#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

typedef std::chrono::high_resolution_clock hresClock;
typedef hresClock::time_point hresClockTimePoint;

/** Timers namespace
 *  @namespace Timers
 *
 *  @note Not required to access Timer class, intended for
 *        storing named scope timers for later access
 */
namespace Timers {
    void logNamedTimers();
    void setLogFile(std::string);

    /// Timer scope struct
    typedef struct {
        std::string m_scopeName;     ///< The name given to the time
        long long* m_executionTime;  ///< How long the timer was active
    } timerScope;

    static std::vector<timerScope>
        named_timers;  ///< Vector of named scope timer execution times
    static std::string out_file = "";  ///< File for storing a timer log
    static bool toScreen =
        false;  ///< Whether or not timers should be logged to the console
}  // namespace Timers

/** Timer class
 *  @class Timer
 *
 *  @note Should rewrite timer class to return std::chrono duration by default
 */
class Timer {
public:
    /// Class constructor
    Timer();
    Timer(long long& out);         ///< Scope resolution timer
    Timer(std::string scopeName);  ///< Named scope resolution timer
    /// Class destructor
    ~Timer();

    // retrieve relative time
    void start();                        ///< Sets internal time point
    long long getMicrosecondsElapsed();  ///< Returns microseconds since timer
                                         ///< was created, or last start() call
    static long long getMicrosecondsSince(
        hresClockTimePoint
            t1);  ///< Returns microseconds since provided time point

    // wait
    bool compareElapsedMicroseconds(
        const long long&
            microseconds);  ///< Returns true if specified time has elapsed;
    static inline void sleepMicroseconds(
        const long long& microseconds);  //< Waits for specified time period
    template <typename Rep, typename Period>
    /** Sleeps for the requuested amount of time
     *  @param length How long to wait for
     */
    static inline void wait(
        std::chrono::duration<Rep, Period>
            length) {  // don't forget abt std::chrono's literals >.>
        // hresClockTimePoint end = hresClock::now() + length;
        // while(hresClock::now() < end);
        std::this_thread::sleep_for(length);
    }

    // retrieve current time
    static hresClockTimePoint
    getCurrentTime();  ///< returns current time as hresClockTimePoint

private:
    hresClockTimePoint m_initialTime;
    long long* m_out;  ///< Storage location for length timer was active
};

#endif /* TIMER_H */
