#pragma once

#include <string>
#include <vector>
#include <functional>

#include <runtime/handlemodel/EventHandler.hh>

class CallbackFunctionBase
{
public:
    virtual ~CallbackFunctionBase() {};
    virtual void Exec(const char* data) = 0;
};

template<typename T>
class CallbackFunction : public CallbackFunctionBase
{
public:
    using CallbackType = std::function<void(T)>;

    CallbackFunction(CallbackType callback)
        : callback_(callback) {}

    void Exec(const char* data) 
    {
        T x;
        ParseProtoStruct(data, x);
        callback_(x);
    }

private:

    CallbackType callback_;  // Store the callback function
};

class CallbacksHandler: public EventHandler
{
public:
    CallbacksHandler();

    ~CallbacksHandler();

    /**
     * When READ event triggered at a connected fd
     * it should read rpc result and execute corresponding callback function
     */
    virtual void HandleReadEvent(int Fd) override;

    template<typename T>
    void Register(int seqno, std::function<void(T)> func) {
        CallidCallbackMapping[seqno] = new CallbackFunction<T>(func);
    }

private:
    std::vector<CallbackFunctionBase*> CallidCallbackMapping;
};
