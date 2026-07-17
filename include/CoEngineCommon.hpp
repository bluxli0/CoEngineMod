#pragma once

#include <Geode/Geode.hpp>
#include <memory>
#include <string>
#include <chrono>
#include <functional>

// Global Engine Namespace Axis
namespace CoEngine {

    // High-performance alias definitions for safe modern memory handling
    template <typename T>
    using Ref = std::shared_ptr<T>;
    
    template <typename T, typename ... Args>
    constexpr Ref<T> CreateRef(Args&& ... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    using Scope = std::unique_ptr<T>;
    
    template <typename T, typename ... Args>
    constexpr Scope<T> CreateScope(Args&& ... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    // Explicit Engine System Error Codes
    enum class StatusCode : int32_t {
        OK = 0,
        ERROR_GENERIC = -1,
        ERROR_INITIALIZATION_FAILED = -2,
        ERROR_OUT_OF_BOUNDS = -3,
        ERROR_MEMORY_ALLOCATION = -4,
        ERROR_CONTEXT_MISSING = -5
    };
}

// ============================================================================
// PRODUCTION LOGGING SYSTEM
// ============================================================================
// Wraps Geode's internal logger to inject standardized mod tags and tracking info

#ifndef CO_DISABLE_LOGGING
    #define CO_LOG_INFO(fmt, ...)  geode::log::info("[CoEngine][INFO] " fmt, ##__VA_ARGS__)
    #define CO_LOG_WARN(fmt, ...)  geode::log::warn("[CoEngine][WARN] " fmt, ##__VA_ARGS__)
    #define CO_LOG_ERROR(fmt, ...) geode::log::error("[CoEngine][ERROR] " fmt, ##__VA_ARGS__)
#else
    #define CO_LOG_INFO(fmt, ...)  void(0)
    #define CO_LOG_WARN(fmt, ...)  void(0)
    #define CO_LOG_ERROR(fmt, ...) void(0)
#endif

// Assertion utility for strict engine pipeline debugging
#define CO_ASSERT(condition, msg) \
    do { \
        if (!(condition)) { \
            CO_LOG_ERROR("Assertion Failed: %s | File: %s | Line: %d", msg, __FILE__, __LINE__); \
            std::ccallback_t(); \
        } \
    } while (false)
