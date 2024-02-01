#include <pico/stdlib.h>
#include <pico/multicore.h>

#include "event_dispatcher.hpp"

EventDispatcher::EventDispatcher()
{
    mutex_init(&mutex_);
}

void EventDispatcher::launch_dispatcher_thread()
{
    instance().run_dispatcher_loop();
}

void EventDispatcher::run_dispatcher_loop()
{
    bool state = 0;
    lock();
    while(run_dispatcher_) {
        unlock();
        dispatcher_running_ = true;
        gpio_put(PICO_DEFAULT_LED_PIN, state);
        state = 1 - state;
        sleep_ms(1000);
        lock();
    }
    dispatcher_running_ = false;
    unlock();
}

void EventDispatcher::start_dispatcher()
{
    run_dispatcher_ = true;
    multicore_launch_core1(&EventDispatcher::launch_dispatcher_thread);
}

void EventDispatcher::stop_dispatcher()
{
    lock();
    run_dispatcher_ = false;
    unlock();  
}

bool EventDispatcher::is_dispatcher_running()
{
    lock();
    bool running = dispatcher_running_;
    unlock();
    return running;
}
