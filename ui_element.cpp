#include "ui.h"

namespace ui
{
    UIElement::UIElement(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {}

    UIElement::~UIElement() {}
}