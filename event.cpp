#include "event.h"

namespace event
{
    Event::~Event() {}

    WindowResizeEvent::WindowResizeEvent(int newWidth, int newHeight) : newWidth(newWidth), newHeight(newHeight) {}

    WindowResizeEvent::~WindowResizeEvent() {}

    QuitEvent::QuitEvent() {}

    QuitEvent::~QuitEvent() {}
}