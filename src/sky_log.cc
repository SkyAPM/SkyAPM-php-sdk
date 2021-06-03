/*
 * Copyright 2021 SkyAPM
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
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
