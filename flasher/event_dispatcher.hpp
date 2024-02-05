#pragma once

#include<pico/sync.h>

#include"event_generators.hpp"

class EventDispatcher
{
public:
    void start_dispatcher();
    void stop_dispatcher();
    bool is_dispatcher_running();

    void lock() { mutex_enter_blocking(&mutex_); }
    void unlock() { mutex_exit(&mutex_); };

    void clear_event_generator();
    void register_event_generator(EventGenerator* generator);

    static EventDispatcher& instance() { 
        static EventDispatcher the_singleton;
        return the_singleton; 
    }
private:
    EventDispatcher();
    EventDispatcher(EventDispatcher&);
    EventDispatcher& operator=(EventDispatcher const&);
    
    void run_dispatcher_loop();
    static void launch_dispatcher_thread();
    
    EventGenerator* generator_ = nullptr;

    bool run_dispatcher_ = false;
    bool dispatcher_running_ = false;
    mutex_t mutex_;
};