
/**
 * Copyright (c) Saikishore Gowrishankar 2021. All rights reserved.
 *
 * All owned trademarks belong to their respective owners.
 * Lawyers love tautologies
 *
 */

//Standard includes
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <random>

//Depedent includes
#include "sync_cout.h"
#include "semaphore.h"

Semaphore agent = 1, mutex = 1;

struct Signal
{
    bool condition{false};
    Semaphore s{0};
} tobacco_signal{}, paper_signal{}, match_signal{};

//Functor that models an Agent.
class Agent
{
public:
    Agent(Semaphore* r1, Semaphore* r2, int i) : id{i}, resource1{r1}, resource2{r2} {}
    void operator()() const
    {
        std::mt19937 gen{std::random_device{}()};
        std::uniform_int_distribution<> distrib(1, 200);

        for(int i = 0; i < 6; ++i)
        {

            //Sleep for random time between 1 to 200ms
            std::this_thread::sleep_for(std::chrono::milliseconds(distrib(gen)));
            agent.P();

            out::sync_cout{} << "\n\u001b[36mAgent " << id << " releasing two ingredients...\u001b[0m\n";

            resource1->V();
            resource2->V();
        }
    }
private:
    int id{};
    Semaphore* resource1 = nullptr,
             * resource2 = nullptr;
};

//Functor that models a Pusher
class Pusher
{
public:
    Pusher(Semaphore* src, Signal* src_sig, Signal* s1, Signal* s2, int i)
        : id{i}, source{src}, source_sig{src_sig}, signal1{s1}, signal2{s2} {}
    void operator()() const
    {

        for(int i = 0; i < 12; ++i)
        {
            source->P();

            mutex.P(); //Enter critical region

            if (auto& c = signal1->condition; c)
            {
                c = false;
                signal2->s.V();
                out::sync_cout{} << "\u001b[35mPusher " << id << " notifying smoker\u001b[0m\n";
            }
            else if(auto& c = signal2->condition; c)
            {
                c = false;
                signal1->s.V();
                out::sync_cout{} << "\u001b[35mPusher " << id << " notifying smoker\u001b[0m\n";
            }

            else source_sig->condition = true;

            mutex.V(); //Leave critical region

        }

    }
private:
    int id = 0;
    Semaphore* source = nullptr;
    Signal * source_sig = nullptr;
    Signal * signal1 = nullptr;
    Signal * signal2 = nullptr;
};

//Functor that models a Smoker
class Smoker
{
public:
    Smoker(Signal* src, int i) : id{i}, source{src} {}

    void operator()() const
    {
        std::mt19937 gen{std::random_device{}()};
        std::uniform_int_distribution<> distrib(1, 50);

        for(int i = 0; i < 3; ++i)
        {
            source->s.P();

            //Make cigarette
            out::sync_cout{} << "Smoker " << id << " making a cigarette\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(distrib(gen)));

            agent.V();

            //Smoke cigarrete
            out::sync_cout{} << "Smoker " << id << " smoking a cigarette\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(distrib(gen)));
        }

        out::sync_cout{} << "\u001b[31mSmoker " << id << " is finished.\n"
                         << "\t\u001b[33m\"Great, now I'm hungry!\"\u001b[0m\n";
    }

private:
    int id = 0;
    Signal* source = nullptr;
};

int main()
{
    std::vector<std::thread> threads;
    Semaphore tobacco = 0, paper = 0, match = 0; 

    out::sync_cout{} << "\n\n\u001b[32mLaunching threads...\u001b[0m\n\n";

    //Launch Agents
    threads.emplace_back(Agent{&tobacco, &paper, 1});
    threads.emplace_back(Agent{&tobacco, &match, 2});
    threads.emplace_back(Agent{&paper,   &match, 3});

    //Launch Pushers
    threads.emplace_back(Pusher{&tobacco, &tobacco_signal, &paper_signal,   &match_signal, 1});
    threads.emplace_back(Pusher{&paper,   &paper_signal, &tobacco_signal, &match_signal, 2});
    threads.emplace_back(Pusher{&match,   &match_signal, &tobacco_signal, &paper_signal, 3});

    //Launch Smokers
    for(int i = 0; i < 6;)
    {
        threads.emplace_back(Smoker{&tobacco_signal, ++i});
        threads.emplace_back(Smoker{&paper_signal, ++i});
        threads.emplace_back(Smoker{&match_signal, ++i});
    }

    //Join all threads
    for(auto&& t : threads) t.join();

    out::sync_cout{} << "\n\n\u001b[32mAll threads have successfully joined. Terminating.\u001b[0m\n\n";
}
