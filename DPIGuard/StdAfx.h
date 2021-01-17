#pragma once

#include "TargetVer.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>

#include <cstdint>

#include <string>
#include <set>
#include <list>
#include <vector>

#include <algorithm>
#include <thread>
#include <memory>
#include <stdexcept>
#include <shared_mutex>
#include <system_error>

#include <windivert.h>
#include <yaml-cpp/yaml.h>
