#pragma once

#include "ui_element.h"

struct UIEvent {
    enum class Type { Click, Hover, Change };

    Type type;
    UIElement* sender;
};