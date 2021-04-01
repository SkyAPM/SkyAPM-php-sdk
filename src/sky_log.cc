#include <string>
#include <sstream>
#include <ostream>
#include <iostream>
#include <fstream>
#include "php_skywalking.h"

static std::ofstream log_fs;

void sky_log(std::string log) {
  if (SKYWALKING_G(log_enable) && !log_fs.is_open()) {
      log_fs.open(SKYWALKING_G(log_path), std::ios::app);
  }
  
  if (log_fs.is_open()) {
    log_fs << log << std::endl;
  }
}
