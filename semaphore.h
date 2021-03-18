#pragma once
#include <semaphore.h>

class Semaphore
{
private:
    sem_t s{};
public:

    // RAII/RRID
    // https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization
    Semaphore(int value) { sem_init(&s, 0, value); }
    ~Semaphore()         { sem_destroy(&s);        }

    //Disallow copying
    Semaphore(Semaphore const&) = delete;
    Semaphore& operator=(Semaphore const&) = delete;

    int P()     { return sem_wait(&s);    }
    int try_P() { return sem_trywait(&s); }
    int V()     { return sem_post(&s);    }
};
