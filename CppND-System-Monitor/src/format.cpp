#include <string>
#include <iomanip>
#include "format.h"

using std::string;


string Format::ElapsedTime(long seconds) 
{ 
    long hours = seconds / 3600;
    long minutes = (seconds % 3600) / 60;
    long secs = seconds % 60;

    std::ostringstream timeStream;

    timeStream << std::setw(2) << std::setfill('0') << hours <<":"
               << std::setw(2) << std::setfill('0') << minutes <<":"
               << std::setw(2) << std::setfill('0') << secs;
    return timeStream.str();
}