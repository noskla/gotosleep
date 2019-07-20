#include <iostream>
#include <string>
#include <regex>
#include <ctime>
#include <random>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#endif

#include "conf.h"

const char* win_letter = std::getenv("SystemDrive");
std::string command;


std::string current_os() {

#ifdef _WIN32
  return "windows";
#elif __linux__
  return "linux";
#else
  return "unknown";
#endif

}

void crpl_sleep(int seconds) {
#ifdef _WIN32
  Sleep(seconds);
#elif __linux__
  usleep(seconds * 1000);
#endif
}

void out_debug(std::string message) {

  if (conf.at("debug")) {
    std::cout << "[debug] " << message << std::endl;
  }

}


int main(int argc, char *argv[]) {

  // operating system
  std::string os = current_os();
  out_debug("Running " + os);
  if (os == "unknown") {
    std::cout << "[warning] Unknown operating system, attempting to run as Linux anyway." << std::endl;
    os == "linux";
  }

  // arguments
  if (argc < 2) { 
    std::cout << "Missing arguments. Type gotosleep --help for more information." << std::endl;
    return 0;
  }
  
  std::string argument = std::string(argv[1]);

  // thanks cpp for switch not working well with char* and std::string
  if ((argument == "-h") || (argument == "--help")) {
    std::cout << "-h, --help      Displays this message." << std::endl;
    std::cout << "-c, --hour      Shutdown on given hour (24-hour clock)." << std::endl;
    std::cout << "-t, --time      Shutdown after given time." << std::endl;
    std::cout << "-r, --random    Shutdown at random time between 10 minutes to 5 hours." << std::endl;
    std::cout << "-v, --version   Show version information." << std::endl;
    std::cout << "Use \"start /B gotosleep <args>\" on Windows or \"sudo gotosleep <args> &\" on Linux to run program in the background." << std::endl;
    std::cout << std::endl << "Examples:" << std::endl;
    std::cout << "   gotosleep --hour 17:12:00" << std::endl;
    std::cout << "   gotosleep --timer 2:10:00" << std::endl;
    std::cout << "   gotosleep --random" << std::endl;
    return 0;
  }


  else if ((argument == "-c") || (argument == "--hour")) {
    // no argument
    if (argv[2] == NULL) {
      std::cout << "No time argument passed." << std::endl;
      std::cout << "Example: gotosleep --hour 17:12:00" << std::endl;
      return 0;
    }

    std::string scheduled_time = std::string(argv[2]);
    if (std::regex_match(scheduled_time, std::regex("^\\d+:[0-5][0-9]:[0-5][0-9]$"))) {

      // convert string with time to array of integers
      std::istringstream iss(scheduled_time);
      int time [3]; int i = 0;
      std::string token;
      while (std::getline(iss, token, ':')) {
        time[i] = std::stoi(token);
        i++;
      }

      out_debug("Selected time: " + (std::to_string(time[0]) + ":" + std::to_string(time[1]) + ":" + std::to_string(time[2])));
      
      if (time[0] >= 24) {
        std::cout << "Please use --time argument to choose time longer than 24 hours." << std::endl;
        return 0;
      }

      time_t t = std::time(nullptr);
      tm* tm_struct = std::localtime(&t);
      long int seconds_since_midnight = ((tm_struct->tm_hour) * 3600) + ((tm_struct->tm_min) * 60) + (tm_struct->tm_sec);
      out_debug("seconds_since_midnight (time passed since today at 00:00:00): " + std::to_string(seconds_since_midnight));

      long int int_t = static_cast<long int> (std::time(nullptr));
      out_debug("int_t (current timestamp): " + std::to_string(int_t));
      
      long int midnight = (int_t - seconds_since_midnight);
      out_debug("midnight (today at 00:00:00) " + std::to_string(midnight));
      
      long int time_in_seconds = ((time[0] * 3600) + (time[1] * 60) + time[2]);
      out_debug("time_in_seconds: " + std::to_string(time_in_seconds));

      if (seconds_since_midnight > time_in_seconds) {
        time_in_seconds += (24 * 3600);
      //  out_debug(std::string(time_in_seconds) + " > " + std::string(seconds_since_midnight));
        out_debug("Selected time appears to be tomorrow. Adding 24 hours...");
      }

      long int run_at = (midnight + time_in_seconds);

      out_debug("Program will be triggered on timestamp " + std::to_string(run_at) + ". (run_at)");

      while (true) {
        long int current_timestamp = static_cast<long int> (std::time(nullptr));
        if (current_timestamp >= run_at)
          break;
        crpl_sleep(1000);
        out_debug("Ping");
      }

      std::cout << "Shutting down..." << std::endl;

    } else {

      std::cout << "Wrong time syntax." << std::endl;
      std::cout << "Example: gotosleep --hour 17:12:00" << std::endl;
      return 0;

    }
  }

  else if ((argument == "-t") || (argument == "--time")) {
  
    if (argv[2] == NULL) {
      std::cout << "No time argument passed." << std::endl;
      std::cout << "Example: gotosleep --time 2:10:00" << std::endl;
      return 0;
    }

    std::string time = std::string(argv[2]);
    if (std::regex_match(time, std::regex("^\\d+:[0-5][0-9]:[0-5][0-9]$"))) {

      // convert string with time to array of integers
      std::istringstream iss(time);
      int int_time [3]; int i = 0;
      std::string token;
      while (std::getline(iss, token, ':')) {
        int_time[i] = std::stoi(token);
        i++;
      }

      out_debug(std::to_string(int_time[0]) + " hours, " + std::to_string(int_time[1]) + " minutes, " + std::to_string(int_time[2]) + " seconds.");

      long int time_to_wait = ((int_time[0] * 3600) + (int_time[1] * 60) + int_time[2]);

      crpl_sleep(time_to_wait * 1000);
      std::cout << "Shutting down..." << std::endl;

    } else {
      std::cout << "Wrong syntax." << std::endl;
      std::cout << "Example: gotosleep --time 2:10:00" << std::endl;
      return 0;
    }
  
  }

  else if ((argument == "-r") || (argument == "--random")) {
  
    int ten_minutes = 600;
    int five_hours = 18000;
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(ten_minutes, five_hours);

    int randomized_time = distr(eng);
    out_debug("Randomized time: " + std::to_string(randomized_time));

    crpl_sleep(randomized_time * 1000);
    std::cout << "Shutting down..." << std::endl;
  
  }

  else if ((argument == "-v") || (argument == "--version")) {
  
    std::cout << "Running gotosleep " << std::to_string(_ver) << std::endl;
    std::cout << "Made by Jarosław _kana C.: https://github.com/canimar/" << std::endl;
    return 0;
  
  }

  else {
  
    std::cout << "Unknown argument. Please check \"gotosleep --help\" to get started." << std::endl;
    return 0;
  
  }


  // prepare command
  if (os == "windows") {

    // windows
    
    out_debug(std::string("Found main drive ") + std::string(win_letter));
    std::string win_path = "\\windows\\system32\\shutdown";
    std::string win_args = " /s";
    command = (win_letter + win_path + win_args);

  } else if (os == "linux") {
  
    // linux
    command = "/bin/shutdown -P now";
  
  }

  // run
  std::cout << command << std::endl;
  system(command.c_str());

  return 0;

}

