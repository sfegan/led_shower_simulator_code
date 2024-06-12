#include <cstdlib>

#include <pico/stdlib.h>
#include <pico/multicore.h>

#include "flasher.hpp"
#include "event_dispatcher.hpp"
#include "set_charges.pio.h"

namespace {
    static BuildDate build_date(__DATE__,__TIME__);
}

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
    uint32_t x[128];

    // Choose which PIO instance to use (there are two instances)
    PIO pio = pio0;

    // Our assembled program needs to be loaded into this PIO's instruction
    // memory. This SDK function will find a location (offset) in the
    // instruction memory where there is enough space for our program. We need
    // to remember this location!
    uint offset = pio_add_program(pio, &set_charges_program);

    // Find a free state machine on our chosen PIO (erroring if there are
    // none). Configure it to run our program, and start it, using the
    // helper function we included in our .pio file.
    uint sm = pio_claim_unused_sm(pio, true);
    set_charges_program_init(pio, sm, offset, 0, 20);

    bool state = 0;
    lock();
    while(run_dispatcher_) {
        dispatcher_running_ = true;
        if(generator_ and generator_->isEnabled()) {
            uint32_t delay = generator_->nextEventDelay();
            uint32_t nx = generator_->nextEventPattern(x);
            unlock();
            gpio_put(PICO_DEFAULT_LED_PIN, state);
            state = 1 - state;
            pio_sm_put_blocking(pio, sm, x[0]);
            sleep_us(delay);
        } else {
            unlock();
            sleep_us(100);            
        }
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

void EventDispatcher::clear_event_generator()
{
    lock();
    generator_ = nullptr;
    unlock();
}

void EventDispatcher::register_event_generator(EventGenerator* generator)
{
    lock();
    generator_ = generator;
    unlock();
}
