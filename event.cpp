#include "event.h"

namespace event
{
    Event::~Event() {}

    WindowResizeEvent::WindowResizeEvent(int newWidth, int newHeight) : newWidth(newWidth), newHeight(newHeight) {}

    WindowResizeEvent::~WindowResizeEvent() {}

    QuitEvent::QuitEvent() {}

    QuitEvent::~QuitEvent() {}

    NextModelEvent::NextModelEvent() {}

    NextModelEvent::~NextModelEvent() {}

    NextCubemapEvent::NextCubemapEvent() {}

    NextCubemapEvent::~NextCubemapEvent() {}

    LightAzimuthChangeEvent::LightAzimuthChangeEvent(float delta) : delta(delta) {}

    LightAzimuthChangeEvent::~LightAzimuthChangeEvent() {}

    LightInclineChangeEvent::LightInclineChangeEvent(float delta) : delta(delta) {}

    LightInclineChangeEvent::~LightInclineChangeEvent() {}

    LuminanceChangeEvent::LuminanceChangeEvent(float delta) : delta(delta) {}

    LuminanceChangeEvent::~LuminanceChangeEvent() {}

    ScaleChangeEvent::ScaleChangeEvent(float delta) : delta(delta) {}

    ScaleChangeEvent::~ScaleChangeEvent() {}

    CameraSpeedChangeEvent::CameraSpeedChangeEvent(float factor) : factor(factor) {}

    CameraSpeedChangeEvent::~CameraSpeedChangeEvent() {}
}