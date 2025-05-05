#include <string.h>
#include <thread>

#include <client/RpcClient.hh>

#include <common/Logger.hh>

void EventHandlerRouter::HandleEvent(int Fd, EventType Type)
{
    switch (Type)
    {
    case CLOSE_EVENT:
        Closehdl->HandleCloseEvent(Fd);
        break;
    case READ_EVENT:
        Readhdl->HandleReadEvent(Fd);
        break;
    case WRITE_EVENT:
        Writehdl->HandleWriteEvent(Fd);
        break;
    default:
        break;
    }
}

RpcClient::RpcClient(Options options):
    _options(options),
    _transport(),
    _router(),
    _callback_handler(),
    _initialized(false),
    m_stop(false)
{
}

RpcClient::~RpcClient()
{
    // Stop main()
    m_stop = true;
    // we cannot start destructor procedure before main() returns
    std::unique_lock<std::mutex> lock(m);
}

void RpcClient::Initialize()
{
    // 建立与服务器的连接
    _transport.Connect(_options.svr_ip, _options.svr_port);

    // 由Transport处理CLOSE事件
    _router.Closehdl = &_transport;
    // 由Transport处理WRITE事件
    _router.Writehdl = &_transport;
    // 由Callbackshdl处理READ事件
    _router.Readhdl = &_callback_handler;

    _initialized = true;
}

int RpcClient::Main(int argc, char* argv[])
{
    std::unique_lock<std::mutex> lock(m);

    // 0.1s
    int timeout = 100;

    while (!m_stop)
    {
        // wait, but not block, or this loop may dead
        auto&& tasks = _transport.GetPoller().Dispatch(timeout, _router);
        // process
        for (auto&& task : tasks)
        {
            task();
        }
    }

    return 0;
}

void RpcClient::SendRequest(const RpcMessage& Message)
{
    // if not _initialized, interrupt
    if (!_initialized)
    {
        log_err("RpcClient::SendRequest Not _initialized!\n");
        return;
    }
    // 新请求入队
    _transport.Push(Message);
}

std::shared_ptr<RpcClient> RpcClient::GetRpcClient(Options options)
{
    std::shared_ptr<RpcClient> sp = std::make_shared<RpcClient>(options);
    // 初始化Rpc客户端
    sp->Initialize();
    // 启动Rpc客户端
    std::thread([&]() {
        sp->Main(1, nullptr);
    }).detach();
    return sp; 
}
