#pragma once

namespace cal {

    template <typename T> struct Action;

    template <typename R, typename... Args> 
    struct Action<R(Args...)>
    {
    private:
        using InstancePtr = void*;
        using InternalFunction = R(*)(InstancePtr, Args...);
        struct Stub
        {
            InstancePtr first;
            InternalFunction second;
        };

        template <R(*Function)(Args...)> static R FunctionStub(InstancePtr, Args... args)
        {
            return (Function)(args...);
        }

        template <typename C, R(C::* Function)(Args...)>
        static R ClassMethodStub(InstancePtr instance, Args... args)
        {
            return (static_cast<C*>(instance)->*Function)(args...);
        }

        template <typename C, R(C::* Function)(Args...) const>
        static R ClassMethodStub(InstancePtr instance, Args... args)
        {
            return (static_cast<C*>(instance)->*Function)(args...);
        }

    public:
        Action()
        {
            m_stub.first = nullptr;
            m_stub.second = nullptr;
        }

        template <typename T>
        Action(const T& obj)
        {
            m_stub.first = (InstancePtr)&obj;
            m_stub.second = [](InstancePtr inst, Args... args) -> R {
                const T& obj = *(const T*)inst;
                return obj(args...);
                };
        }

        bool isValid() { return m_stub.second != nullptr; }

        void bindRaw(InstancePtr obj, InternalFunction fn) {
            m_stub.first = obj;
            m_stub.second = fn;
        }

        template <R(*Function)(Args...)> void bind()
        {
            m_stub.first = nullptr;
            m_stub.second = &FunctionStub<Function>;
        }

        template <auto F, typename C> void bind(C* instance)
        {
            m_stub.first = instance;
            m_stub.second = &ClassMethodStub<C, F>;
        }

        R invoke(Args... args) const
        {
            ASSERT(m_stub.second != nullptr);
            return m_stub.second(m_stub.first, args...);
        }

        bool operator==(const Action<R(Args...)>& rhs)
        {
            return m_stub.first == rhs.m_stub.first && m_stub.second == rhs.m_stub.second;
        }

    private:
        Stub m_stub;
    };

    template <typename T> struct ToAction_T;
    template <typename R, typename C, typename... Args> struct ToAction_T<R(C::*)(Args...)> {
        using Type = Action<R(Args...)>;
    };

    template <typename T> using ToAction = typename ToAction_T<T>::Type;

    template <auto M, typename C>
    auto makeAction(C* inst) {
        ToAction<decltype(M)> res;
        res.template bind<M, C>(inst);
        return res;
    };

}