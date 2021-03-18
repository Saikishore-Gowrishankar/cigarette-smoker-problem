#pragma once

#include <sstream>
#include <mutex>
#include <iostream>

namespace out
{
    class sync_cout : public std::ostringstream
    {
    public:
        sync_cout() = default;
        virtual ~sync_cout() {std::scoped_lock l(out); std::cout << this->str();}
    private:
        std::mutex out{};
    };
}
    