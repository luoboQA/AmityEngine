#pragma once

#include <vector>
#include <functional>

namespace Core {

template<typename... Args>
class Event
{
public:
    using Callback = std::function<void(Args...)>; // callback is no return with Args params

    void Connect(Callback callback)
    {
        // create connection for this dude
        m_callbacks.push_back(callback);
    }

    void fire(Args... args)
    {
        for (const auto& callback : m_callbacks)
        {
            callback(args...);
        }
    }

private:
    std::vector<Callback> m_callbacks;
};

} // namespace Core