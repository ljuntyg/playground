#include "ui.h"

namespace ui 
{
    UIManager::UIManager(std::shared_ptr<UIRenderer> renderer) : renderer(renderer) {}
    
    UIManager::~UIManager() {}

    void UIManager::addElement(std::shared_ptr<UIElement> element)
    {
        elements.push_back(element);
    }

    void UIManager::render()
    {
        for (const auto& element : elements)
        {
            element->render(*renderer);
        }
    }
}