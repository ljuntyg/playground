#include "pub_sub.h"

Publisher::~Publisher() {}

void Publisher::subscribe(Subscriber* subscriber)
{
    subscribers.push_back(subscriber);
}

void Publisher::unsubscribe(Subscriber* subscriber)
{
    subscribers.remove(subscriber);
}

void Publisher::publish(const event::Event* event)
{
    for (auto subscriber : subscribers)
    {
        subscriber->notify(event);
    }
}

Subscriber::~Subscriber() {}
