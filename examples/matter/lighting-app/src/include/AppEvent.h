/*
 * RT583 Matter Lighting App — Application event definitions.
 */

#pragma once

#include <cstdint>

using EventHandler = void (*)(const struct AppEvent &);

enum class AppEventType : uint8_t
{
    None = 0,
    Lighting,
    IdentifyStart,
    IdentifyStop,
};

enum class FunctionEvent : uint8_t
{
    NoneSelected = 0,
    FactoryReset,
};

struct LightingAction
{
    enum Type : uint8_t
    {
        On,
        Off,
        Toggle,
        Level,
    };
};

struct AppEvent
{
    union
    {
        struct
        {
            LightingAction::Type Action;
            uint8_t              Level; /* 0–254, used when Action == Level */
        } LightingEvent;
    };

    AppEventType Type{ AppEventType::None };
    EventHandler Handler{ nullptr };
};
