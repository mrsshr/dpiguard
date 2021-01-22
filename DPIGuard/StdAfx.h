#pragma once

#include "TargetVer.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>

#include <cstdint>
#include <cctype>

#include <array>
#include <string>
#include <list>
#include <vector>

#include <algorithm>
#include <condition_variable>
#include <thread>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <system_error>

#include <windivert.h>
#include <yaml-cpp/yaml.h>
