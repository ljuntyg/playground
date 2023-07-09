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
}