#pragma once

#include <list>
#include "event.h"

class Subscriber
{
public:
    virtual ~Subscriber();

    virtual void notify(const event::Event* event) = 0;
};

// TODO: Lifetime of subscribers?
class Publisher
{
public:
    virtual ~Publisher();

    void subscribe(Subscriber* subscriber);
    void unsubscribe(Subscriber* subscriber);
    void publish(const event::Event* event);

private:
    std::list<Subscriber*> subscribers;
};