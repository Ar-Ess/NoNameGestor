#pragma once

#include "Framework/Data/Vector.h"
#include "Framework/Engine/ID.h"

#include <functional>

//TODO: *(void**)&func is not the best way to identify and can lead to problems. Find a better solution.
//TODO: What happens if the function I want to bind is from a static class? Is the only option [](){StaticClass::Function();}?? This seems off.

/*
Multicast delegate similar to C#. It supports:
-> Global Functions: action += &GlobalFunc;
-> Local  Functions: action += Action<>::Bind(this, &Class::LocalFunc);
-> Lambda Functions: action += Action<>::Bind([](){});
Invoke action: action.Invoke();
Action Removal:
-> Global Functions: action -= &GlobalFunc;
-> Local  Functions: action -= Action<>::Bind(this, &Class::LocalFunc);
-> Lambda Functions: action -= l; (Listener l = Action<>::Bind(lambda))
*/
template<class ...Ts>
class Action
{
public:

    using Func = std::function<void(Ts...)>;

    //Internal listener representation.
    //Identity:
    //- Global functions  -> function pointer
    //- Member functions  -> object + function pointer
    //- Lambdas           -> generated ID
    struct Listener
    {
        Func func;
        void* object = nullptr;
        void* funcPtr = nullptr;
        ID id = ID::Empty;

        bool operator==(const Listener& other) const
        {
            if (id.IsEmpty() && other.id.IsEmpty())
            {
                return object == other.object &&
                    funcPtr == other.funcPtr;
            }

            return id == other.id;
        }

        template<typename R>
        Listener(R(*func)(Ts...)) :
            object(nullptr)
        {
            funcPtr = reinterpret_cast<void*>(func);

            this->func = [func](Ts... args)
                {
                    func(args...);
                };
        }

        Listener() = default;
    };

    //Creates a listener from a member function.
    //action += Action<>::Bind(this, &Class::LocalFunc);
    //action -= Action<>::Bind(this, &Class::LocalFunc);
    template<typename T, typename R>
    static Listener Bind(T* obj, R(T::* func)(Ts...))
    {
        Listener l{};

        l.object = obj;
        l.funcPtr = *(void**)&func;

        l.func = [obj, func](Ts... args)
            {
                (obj->*func)(args...);
            };

        return l;
    }

    //Creates a listener from a const member function.
    //action += Action<>::Bind(this, &Class::LocalFunc);
    //action -= Action<>::Bind(this, &Class::LocalFunc);
    template<typename T, typename R>
    static Listener Bind(const T* obj, R(T::* func)(Ts...) const)
    {
        Listener l{};

        l.object = (void*)obj;
        l.funcPtr = *(void**)&func;

        l.func = [obj, func](Ts... args)
            {
                (obj->*func)(args...);
            };

        return l;
    }

    //Creates a listener from a lambda/functor.
    //action = Action<>::Bind([](){});
    //or, to be able to remove it:
    //auto lambda = Action<>::Bind([](){});
    //action += lambda;
    //action -= lambda;
    //This does NOT work:
    //action -= Action<>::Bind([](){});
    template<typename Callable>
    static Listener Bind(Callable&& f)
    {
        Listener l{};

        l.id = ID::New();
        l.object = nullptr;
        l.funcPtr = nullptr;

        l.func = std::forward<Callable>(f);

        return l;
    }

    void Invoke(Ts... arguments)
    {
        listeners.Iterate(
            [&](const Listener& l)
            {
                l.func(arguments...);
                return true;
            }
        );
    }

    //Subscribes a listener.
    //- Global Functions: action += &GlobalFunc;
    //- Local  Functions: action += Action<>::Bind(this, &Class::LocalFunc);
    //- Lambda Functions: action = Action<>::Bind([](){});
    void operator+=(const Listener& l)
    {
        listeners.PushBack(l);
    }

    //Unsubscribes a listener.
    //- Global Functions: action -= &GlobalFunc;
    //- Local  Functions: action -= Action<>::Bind(this, &Class::LocalFunc); (or storing the Listener)
    //- Lambda Functions: Listener l = Action<>::Bind([](){}); action += l; action -= l; (Only way) 
    void operator-=(const Listener& l)
    {
        listeners.Remove(l);
    }

    //Returns true if the listener is subscribed.
    //- Global Functions: action.Exists(&GlobalFunc);
    //- Local  Functions: action.Exists(Action<>::Bind(this, &Class::LocalFunc)); (or storing the Listener)
    //- Lambda Functions: Listener l = Action<>::Bind([this, &Class::LocalFunc]](){}); action.Exists(l);
    bool Exists(const Listener& l)
    {
        return ListenerExists(l);
    }

    //Returns the number of subscribed listeners.
    unsigned int Count() const
    {
        return listeners.Size();
    }

    //Returns true if no listeners are subscribed.
    void IsEmpty() const
    {
        return listeners.IsEmpty();
    }

    //Removes all listeners.
    void Clear()
    {
        listeners.Clear();
    }

private:

    bool ListenerExists(const Listener& l)
    {
        return listeners.Contains(l);
    }

private:

    Vector<Listener> listeners;

};