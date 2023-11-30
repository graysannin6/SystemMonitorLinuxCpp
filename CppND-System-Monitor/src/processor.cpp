#include "processor.h"
#include "linux_parser.h"

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() { 
    // Obtain CPU utilization data using the LinuxParser namespace
    long activeJiffies = LinuxParser::ActiveJiffies();
    long idleJiffies = LinuxParser::IdleJiffies();
    
    // Calculate total jiffies since last check
    long totalJiffies = activeJiffies + idleJiffies;
    
    // Calculate the change in active and total jiffies since last check
    long activeDelta = activeJiffies - prevActiveJiffies_;
    long totalDelta = totalJiffies - prevTotalJiffies_;
    
    // Update the previous values for the next check
    prevActiveJiffies_ = activeJiffies;
    prevTotalJiffies_ = totalJiffies;
    
    // Calculate CPU utilization as the ratio of active change to total change
    return totalDelta != 0 ? static_cast<float>(activeDelta) / totalDelta : 0.0;
}
