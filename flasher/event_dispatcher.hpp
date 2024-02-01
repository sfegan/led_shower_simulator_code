#pragma once

#include<pico/sync.h>

class EventDispatcher
{
public:
    void lock() { mutex_enter_blocking(&mutex_); }
    void unlock() { mutex_exit(&mutex_); };

    static EventDispatcher& instance() { 
        static EventDispatcher the_singleton;
        return the_singleton; 
    }
private:
    EventDispatcher();
    EventDispatcher(EventDispatcher&);
    EventDispatcher& operator=(EventDispatcher const&);
    
    mutex_t mutex_;
};