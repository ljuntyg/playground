#pragma once

namespace event
{
    class Event
    {
    public:
        virtual ~Event();
    };

    class WindowResizeEvent : public Event
    {
    public:
        WindowResizeEvent(int newWidth, int newHeight);
        ~WindowResizeEvent();

        int newWidth;
        int newHeight;
    };

    class QuitEvent : public Event
    {
    public:
        QuitEvent();
        ~QuitEvent();
    };

    class NextModelEvent : public Event
    {
    public:
        NextModelEvent();
        ~NextModelEvent();
    };  

    class NextCubemapEvent : public Event
    {
    public:
        NextCubemapEvent();
        ~NextCubemapEvent();
    };

    class LightAzimuthChangeEvent : public Event
    {
    public:
        LightAzimuthChangeEvent(float delta);
        ~LightAzimuthChangeEvent();

        float delta;
    };

    class LightInclineChangeEvent : public Event
    {
    public:
        LightInclineChangeEvent(float delta);
        ~LightInclineChangeEvent();

        float delta;
    };

    class LuminanceChangeEvent : public Event
    {
    public:
        LuminanceChangeEvent(float delta);
        ~LuminanceChangeEvent();

        float delta;
    };

    class ScaleChangeEvent : public Event
    {
    public:
        ScaleChangeEvent(float delta);
        ~ScaleChangeEvent();

        float delta;
    };

    class CameraSpeedChangeEvent : public Event
    {
    public:
        CameraSpeedChangeEvent(float factor);
        ~CameraSpeedChangeEvent();

        float factor;
    };
}