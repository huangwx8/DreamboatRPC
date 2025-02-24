#include <common/Logger.hh>
#include <chrono>
#include <iomanip>
#include <unistd.h>
#include <sstream>

// Function to get the current timestamp
std::string current_timestamp() {
    // Get the current time as a time_point
    auto now = std::chrono::system_clock::now();
    
    // Convert to time_t
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    
    // Use strftime to format the time
    std::tm local_time = *std::localtime(&now_time_t);
    
    // Format the timestamp into a string
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &local_time);
    
    return std::string(buffer);
}

FILE* logfile = nullptr;

void start_log(const char* filename)
{
    logfile = fopen(filename, "w+");
}

void log_dev(const char *format, ...)
{
    // Prepare the log message with the timestamp
    std::ostringstream log_stream;
    log_stream << "[" << current_timestamp() << "] ";

    // Use va_list to handle variable arguments
    va_list args;
    va_start(args, format);
    
    // Format the log message with the given format and arguments
    char buffer[1024]; // Buffer to hold the formatted message
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    // Append the formatted message to the timestamp
    log_stream << buffer;
    
    // Print the complete log message to the file descriptor
    fprintf(logfile, "%s", log_stream.str().c_str());
    
    // Clean up the va_list
    va_end(args);
}

void log_err(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    log_dev("Disastrous error! ");
    log_dev(format, args);

    va_end(args);
}

void stop_log()
{
    fclose(logfile);
}
