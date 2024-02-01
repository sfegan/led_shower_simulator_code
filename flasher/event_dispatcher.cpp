#include "event_dispatcher.hpp"

EventDispatcher::EventDispatcher()
{
    mutex_init(&mutex_);
}
