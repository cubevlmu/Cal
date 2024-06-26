#pragma once

#include "base/threading/Thread.hpp"

namespace cal {

    struct VMStack;
    struct VMOperator;

    class VMThread : public Thread
    {
    private:
        VMStack* m_calling_stack;
        VMStack* m_variable_stack;
        VMOperator* m_registers;
        i32 m_code_counter;

    public:
        virtual int run() override;
    };

}