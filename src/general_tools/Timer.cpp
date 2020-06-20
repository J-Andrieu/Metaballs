#include "Timer.h"
#include <chrono>
#include <thread>

/** Logs all named scope timers
 *  If a log file isn't set, then the timers are logged to the screen,
 *  otherwise the will be logged to the provided file. If Timers::toScreen
 *  is set to true, then they can be logged to both the console and the
 *  provided file.
 */
void Timers::logNamedTimers() {
    if (out_file == "") {
        for (auto i = named_timers.begin(); i < named_timers.end(); i++) {
            std::cout << i->m_scopeName << ": " << *(i->m_executionTime) << "us" << std::endl;
        }
    } else {
        std::ofstream out(out_file, std::ofstream::out | std::ofstream::app);
        time_t currentTime = std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
        out << "====================================" << std::endl;
        out << "Begin Timer Log: " << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %X") << std::endl;
        out << "====================================" << std::endl;
        if (Timers::toScreen) {
            for (auto i = named_timers.begin(); i < named_timers.end(); i++) {
                std::cout << i->m_scopeName << ": " << *(i->m_executionTime) << "us" << std::endl;
                out << i->m_scopeName << ": " << *(i->m_executionTime) << "us" << std::endl;
            }
        } else {
            for (auto i = named_timers.begin(); i < named_timers.end(); i++) {
                out << i->m_scopeName << ": " << *(i->m_executionTime) << "us" << std::endl;
            }
        }
    }
}

///Sets the log file for Timers::logNamedTimers()
void Timers::setLogFile(std::string filename) {
    Timers::out_file = filename;
}

///Timer default constructor
Timer::Timer( ) {
    m_initialTime = hresClock::now( );
    m_out = nullptr;
}

/** Timer constructor
 *  @param out A variable to store the timer's length of existence in
 */
Timer::Timer(long long& out) {
    m_initialTime = hresClock::now( );
    m_out = &out;
}

/** Timer constructor
 *  @param scopeName The name used to identify the timers scope
 * 
 *  @note This timer can be logged with Timers::logNamedTimers()
 */
Timer::Timer(std::string scopeName) {
    m_out = new long long;
    Timers::named_timers.push_back({scopeName, m_out});
    m_initialTime = hresClock::now();
}

///Timer destructor
Timer::~Timer() {
    if (m_out != nullptr)
        *m_out = std::chrono::duration_cast< std::chrono::microseconds > ( hresClock::now( ) - m_initialTime ).count( );
}

///Sets a new internal start time for the timer
void Timer::start( ) {
    m_initialTime = hresClock::now( );
}

///Returns microseconds since the timer was started
long long Timer::getMicrosecondsElapsed( ) {
    return std::chrono::duration_cast< std::chrono::microseconds > ( hresClock::now( ) - m_initialTime ).count( );
}

///Returns microseconds since the provided time point
long long Timer::getMicrosecondsSince ( hresClockTimePoint t1 ) {
    return std::chrono::duration_cast< std::chrono::microseconds > ( hresClock::now( ) - t1 ).count( );
}

/** Checks if it has been a requested period of time since the timer was started
 *  @param microseconds Desired period of time in microseconds
 */
bool Timer::compareElapsedMicroseconds ( const long long& microseconds ) {
    return getMicrosecondsElapsed( ) >= microseconds;
}

/** Sleeps for a period of microseconds
 *  @param microseconds How long to sleep for
 */
inline void Timer::sleepMicroseconds ( const long long& microseconds ) {
    std::this_thread::sleep_for((std::chrono::microseconds) microseconds);
}

///Returns a time point for the current time
hresClockTimePoint Timer::getCurrentTime( ) {
    return hresClock::now( );
}
