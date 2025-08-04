#pragma once
#include <Avalanche.hpp>

using namespace avalanche;

#define AVAL_GEODE_MOD AVAL_MOD

#define AVAL_LOG_ENABLED (AVAL_GEODE_MOD && AVAL_GEODE_MOD->getSettingValue<bool>("dev-debug"))

#define AVAL_LOG_DEBUG(...) do { if (AVAL_LOG_ENABLED) log::debug(__VA_ARGS__); } while(0)
#define AVAL_LOG_INFO(...) do { if (AVAL_LOG_ENABLED) log::info(__VA_ARGS__); } while(0)
#define AVAL_LOG_WARN(...) do { if (AVAL_LOG_ENABLED) log::warn(__VA_ARGS__); } while(0)
#define AVAL_LOG_ERROR(...) do { if (AVAL_LOG_ENABLED) log::error(__VA_ARGS__); } while(0)