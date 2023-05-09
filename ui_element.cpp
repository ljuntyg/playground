#include "ui.h"

namespace ui
{
    UIElement::UIElement(int x, int y, int width, int height, const glm::vec4& color)
        : x(x), y(y), width(width), height(height), color(color) {}

    UIElement::~UIElement() {}

    void UIElement::addChild(std::shared_ptr<UIElement> element)
    {
        children.emplace_back(element);
        element->parent = shared_from_this();
        element->x += x; // Offset child x in relation to parent (this)
        element->y += y; // Offset child y in relation to parent (this)
    }
}