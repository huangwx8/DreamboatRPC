# DearmboatRPC

## Prerequisites

This framework can only be run on Linux.

If you are using Windows or Macos, try WSL, Docker, or Cloud Development.

At first, install necessary tool chain.

```bash
apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    clang \
    gdb \
    git \
    protobuf-compiler \
    libprotobuf-dev \
    libprotoc-dev \
    vim \
    curl
```

## Setup

After tool chain is properly installed, you should be able to build this framework with CMake.

```bash
mkdir build
cd build
cmake ..
make
```

As a result, you should see two executable programs lying in build/ folder, Server and Client.

FYI, you can find the entries of these two program in source/example/Server.cc and source/example/Client.cc. You are allowed to modify these two files as you will to implement your own RPC applications.

## Overall Design

The module division of this system is based on trpc-cpp.

The structure definitions for request and response messages are as follows:

```cpp
struct RpcHeader
{
    unsigned short magic;
    unsigned short version;
    bool need_return;
    int seqno;
    int body_length;
    char servicename[MAX_RPC_NAME_SIZE];
};

struct RpcBody
{
    char parameters[MAX_RPC_PARAMS_SIZE];
};

struct RpcResult
{
    int seqno;
    int type; // 1 - int, 2 - float, 3 - string
    char return_buffer[MAX_RPC_RETURN_VALUE];
};
```

- `magic` is used by the server to quickly filter out data packets that don't belong to this RPC protocol.
- `servicename` is the name of the service used by the server to identify the request type and dispatch it to the corresponding routine.
- The request and response of a single call share the same `seqno`, which helps the client distribute return values to different RPC protocols.
- `need_return` indicates whether the server should return a function’s result.
- `parameters` contains the function parameters.
- `type` is used by the client to distinguish the return value type, and `return_buffer` contains the function's return value.

The Serialization layer implements serialization methods for reading and writing the `parameters` and `return_buffer` fields of the RPC.

The Runtime layer, based on libevent, implements a Reactor+Epoll ET (edge-triggered) IO multiplexing model with a thread pool. It defines an event handling base class `EventHandler`, making it easier to inherit and override event handling logic.

The Transport layer wraps Linux system calls for low-level socket communication, establishing a TCP channel between the server and client and passing received data to the Runtime layer.

The Common layer implements general features like the RPC call interface, global UID management, asynchronous logging, and timer management.

## Installation

To install the package, you can simply clone the repository and use `make` command to build it.

```bash
git clone https://github.com/huangwx8/DreamboatRPC.git
cd DreamboatRPC
make server
make client
```

## Usage

This section demonstrates how to call a simple server's "hello world" function to introduce the framework's usage.

To perform remote calls, you need an Rpc proxy class on the client side to invoke the RPC call, and you need a specific implementation on the server side to execute the task and return the function result.

### Writing the Service Base Class

Inherit `RpcServiceBase`, set the `ServiceName` to uniquely identify the RPC service, and add a function declaration for the RPC function with its parameters and return values.

```cpp
class EchoServiceBase : public RpcServiceBase
{
public:
    EchoServiceBase()
    {
        ServiceName = "Echo";
    }
    virtual ~EchoServiceBase() = default;
    virtual std::string Echo(std::string) = 0;
};
```

### Writing the Service Implementation

Inherit the `EchoServiceBase` class and implement the specific RPC function on the server. One limitation is that you need to implement the `Handle` function, which is the unified entry point for RPC processing.

Then, in the server's `main`, start the server and register the service. The server will now be able to respond to incoming RPC requests.

```cpp
class EchoServiceImpl : public EchoServiceBase
{
public:
    EchoServiceImpl() = default;
    virtual ~EchoServiceImpl() = default;
    virtual RpcResult Handle(const RpcMessage& Context) override;
    virtual std::string Echo(std::string data) override;
};

RpcResult EchoServiceImpl::Handle(const RpcMessage& Context)
{
    SERVER_EXEC_RPC_OneParam(Echo, string);
}

std::string EchoServiceImpl::Echo(std::string data)
{
    std::string echo_data = "Server says: " + data;
    std::cout << echo_data << std::endl;
    return echo_data;
}

int main(int argc, char* argv[])
{
    RpcServer ServerStub({"127.0.0.1", 8888, "server.log"}); // Get an RPC Server object
    EchoServiceImpl EchoImplementation; // Construct an Echo service implementation
    ServerStub.RegisterService(&EchoImplementation); // Register Echo service
    ServerStub.Main(argc, argv); // Run the RPC Server, waiting for and handling requests
}
```

### Implementing a One-Way Call

Inherit `EchoServiceBase` and implement the RPC proxy on the client side. You only need to add a macro inside the client-side proxy function.

Start the RPC client, call the client method to get the service proxy object, and then you will see the server print "Server says: hello world!".

```cpp
class EchoServiceProxy : public EchoServiceBase
{
public:
    EchoServiceProxy() = default;
    virtual ~EchoServiceProxy() = default;
    virtual std::string Echo(std::string data) override;
};

std::string EchoServiceProxy::Echo(std::string data)
{
    CLIENT_CALL_RPC_OneParam(data);
    return {};
}

int main(int argc, char* argv[])
{
    auto&& ClientStub = RpcClient::GetRpcClient({ "127.0.0.1", 8888, "client.log" }); // Get an RPC Client object
    auto EchoPtr = ClientStub->GetProxy<EchoServiceProxy>(); // Get an RPC Proxy object
    EchoPtr->Echo("hello world!"); // Initiate the call
    return 0;
}
```

### Using Callbacks

In the above example, the client calls a remote function but does not receive a return value. To write a client program that receives and handles the return value, modify the client code slightly.

```cpp
class AsyncEchoServiceProxy : public EchoServiceBase
{
public:
    AsyncEchoServiceProxy() = default;
    virtual ~AsyncEchoServiceProxy() = default;
    virtual std::string Echo(std::string data) override;
private:
    static void EchoCallback(std::string return_value);
};

std::string AsyncEchoServiceProxy::Echo(std::string data)
{
    CLIENT_CALL_RPC_OneParam_Asynchronously(&AsyncEchoServiceProxy::EchoCallback, data);
    return {};
}

void AsyncEchoServiceProxy::EchoCallback(std::string return_value)
{
    std::cout << "Received " << return_value << std::endl;
}

int main(int argc, char* argv[])
{
    auto&& ClientStub = RpcClient::GetRpcClient({ "127.0.0.1", 8888, "client.log" }); // Get an RPC Client object
    auto EchoPtr = ClientStub->GetProxy<AsyncEchoServiceProxy>(); // Get an RPC Proxy object
    EchoPtr->Echo("hello world!"); // Initiate the call
    sleep(1); // Wait for the return value
    return 0;
}
```

In this example, we added callback handling code to the proxy class to process the return value received from the server. Running the client will display "Received Server says: hello world!" on the client.

### Synchronous Call

Synchronous calls are rarely used since they block the execution of the thread and may impact performance. However, if you prefer a more concise syntax where the proxy’s return value is the server’s return value, it can be implemented easily. You may need additional tools to block the current thread, such as pipes or futures.

Here is an example of using a `future` to implement a synchronous call.

```cpp
class SyncEchoServiceProxy : public EchoServiceBase
{
public:
    SyncEchoServiceProxy() = default;
    virtual ~SyncEchoServiceProxy() = default;
    virtual std::string Echo(std::string data) override;
};

std::string SyncEchoServiceProxy::Echo(std::string data)
{
    std::promise<std::string> p;
    std::future<std::string> f = p.get_future();
    std::function<void(std::string)> cb = [&p](std::string s) {
        p.set_value(s);
    };
    CLIENT_CALL_RPC_OneParam_Asynchronously(cb, data);
    return f.get();
}

int main(int argc, char* argv[])
{
    auto&& ClientStub = RpcClient::GetRpcClient({ "127.0.0.1", 8888, "client.log" });
    auto EchoPtr = ClientStub->GetProxy<SyncEchoServiceProxy>();
    std::string ret = EchoPtr->Echo("hello world!");
    std::cout << ret << std::endl;
    return 0;
}
```

Running this client will print "Server says: hello world!" on the main thread, instead of the asynchronous thread through a callback.

### Expansion

The format of the server's `Handle` method is fixed, but it can be generated using a code generator. Future versions may consider adding auxiliary preprocessing tools for RPC.
