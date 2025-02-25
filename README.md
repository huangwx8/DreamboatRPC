# DearmboatRPC

## Overview

DreamboatRPC framework is designed to provide a high-performance, scalable solution for distributed systems.

It integrates Protobuf for efficient data serialization, adopts a reactor thread model, which efficiently handles multiple concurrent connections using minimal threads, ensuring optimal resource utilization and supporting high concurrency with non-blocking I/O. 

Additionally, it incorporates an external name service, enabling dynamic service discovery and load balancing for distributed environments.

### Architecture

![arch](./imgs/RPCFlow.drawio.png)

### Packet Layout

![packet_layout](./imgs/RPCPacketLayout.drawio.png)

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

struct RpcResultHeader
{
    int seqno;
    int body_length;
};

struct RpcBody
{
    char parameters[MAX_RPC_PARAMS_SIZE];
};
```

- `magic` is used by the server to quickly filter out data packets that don't belong to this RPC protocol.
- `need_return` indicates whether the server should return a function’s result.
- `seqno` represents the position of this RPC is at in sequence. The request and response of a single call share the same `seqno`, which helps the client distribute return values to different RPC protocols.
- `body_length` the actual length in bytes of the body part.
- `servicename` is the name of the service used by the server to identify the request type and dispatch it to the corresponding routine.
- `parameters` contains the function parameters.

The Serialization layer will fill the `parameters` buffer of RpcBody.


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

To install the package, you can simply clone the repository and use `cmake` command to build it.

```bash
git clone https://github.com/huangwx8/DreamboatRPC.git
cd DreamboatRPC

mkdir build
cd build

cmake ..
make
```

As a result, you should see two executable programs lying in build/ folder, Server and Client.

FYI, you can find the entries of these two program in source/apps/Server.cc and source/apps/Client.cc. You are allowed to modify these two files as you will to implement your own RPC applications.

## Usage

This section demonstrates how to implement a hello service to introduce the framework's usage.

To perform remote calls, you need an Rpc proxy class on the client side to invoke the RPC call, and you need a specific implementation on the server side to execute the task and return the function result as well.

### Writing the .proto

The first thing you need to do is to define the arguments of your service, in Dreamboat, we outsource this definition to `protobuf`.

For our hello service, the only argument we need is a `string`, which is the username of client.

```proto
message HelloArgs {
  string username = 1;
}
```

By execute `cmake ..` command, `protoc` is automatically called before compile, which will generate reflection code according to this proto file.

In reflection code, you can see a c++ struct named `HelloArgs` produced by protoc, located in `*.pb.h`.

```cpp
class HelloArgs : public ::google::protobuf::Message
{
    <collapsed>
private:
  ::google::protobuf::internal::ArenaStringPtr username_;
}
```

### Writing the Service Base Class

Inherit `RpcServiceBase`, set the `ServiceName` to uniquely identify the RPC service, and add a function declaration for the RPC function with its parameters and return values.

```cpp
class HelloServiceBase : public RpcServiceBase
{
public:
    HelloServiceBase()
    {
        ServiceName = "Hello";
    }
    virtual ~HelloServiceBase() = default;
    virtual int Hello(HelloArgs args) = 0;
};
```

### Writing the Service Implementation

Inherit the `HelloServiceBase` class and implement the specific RPC function on the server. One limitation is that you need to implement the `Handle` function, which is the unified entry point for RPC processing.

Then, in the server's `main`, start the server and register the service. The server will now be able to respond to incoming RPC requests.

```cpp
class HelloServiceImpl : public HelloServiceBase
{
public:
    HelloServiceImpl() = default;
    virtual ~HelloServiceImpl() = default;
    virtual RpcResult Handle(const RpcMessage& Context) override;
    virtual int Hello(HelloArgs args) override;
};

RpcResult HelloServiceImpl::Handle(const RpcMessage& Context)
{
    HandleRPC(Hello, HelloArgs);
}

int HelloServiceImpl::Hello(HelloArgs args)
{
    std::string log_msg = "Hello, " + args.username();
    std::cout << log_msg << std::endl;
    return 0;
}

int main(int argc, char* argv[])
{
    RpcServer ServerStub({"127.0.0.1", 8888, "server.log"}); // Get an RPC Server object
    HelloServiceImpl HelloImplementation; // Construct an Hello service implementation
    ServerStub.RegisterService(&HelloImplementation); // Register Hello service
    ServerStub.Main(argc, argv); // Run the RPC Server, waiting for and handling requests
}
```

### Implementing a One-Way Call

Inherit `HelloServiceBase` and implement the RPC proxy on the client side. You only need to add a macro inside the client-side proxy function.

Start the RPC client, call the client method to get the service proxy object, and then you will see the server print "Hello, Alice".

```cpp
class HelloServiceProxy : public HelloServiceBase
{
public:
    HelloServiceProxy() = default;
    virtual ~HelloServiceProxy() = default;
    virtual int Hello(HelloArgs args) override;
};

int HelloServiceProxy::Hello(HelloArgs args)
{
    CallRPC(args);
    return {};
}

int main(int argc, char* argv[])
{
    auto&& ClientStub = RpcClient::GetRpcClient({ "127.0.0.1", 8888, "client.log" }); // Get an RPC Client object
    auto HelloPtr = ClientStub->GetProxy<HelloServiceProxy>(); // Get an RPC Proxy object
    HelloArgs args;
    args.set_username("Alice");
    HelloPtr->Hello(args); // Call remote service
    return 0;
}
```

### Using Callbacks

In the above apps, the client calls a remote function but does not receive a return value. To write a client program that receives and handles the return value, modify the client code slightly.

```cpp
class AsyncHelloServiceProxy : public HelloServiceBase
{
public:
    AsyncHelloServiceProxy() = default;
    virtual ~AsyncHelloServiceProxy() = default;
    virtual int Hello(HelloArgs args) override;
private:
    static void HelloCallback(int return_value);
};

int AsyncHelloServiceProxy::Hello(HelloArgs args)
{
    CallRPCAsync(&AsyncHelloServiceProxy::HelloCallback, data);
    return {};
}

void AsyncHelloServiceProxy::HelloCallback(int return_value)
{
    std::cout << "Server returned " << return_value << std::endl;
}

int main(int argc, char* argv[])
{
    auto&& ClientStub = RpcClient::GetRpcClient({ "127.0.0.1", 8888, "client.log" }); // Get an RPC Client object
    auto HelloPtr = ClientStub->GetProxy<AsyncHelloServiceProxy>(); // Get an RPC Proxy object
    HelloArgs args;
    args.set_username("Alice");
    HelloPtr->Hello(args); // Call remote service and wait for response
    sleep(1); // Grace period
    return 0;
}
```

In this apps, we added callback handling code to the proxy class to process the return value received from the server. Running the client will display "Received Server says: hello world!" on the client.

### Synchronous Call

Synchronous calls are rarely used since they block the execution of the thread and may impact performance. However, if you prefer a more concise syntax where the proxy’s return value is the server’s return value, it can be implemented easily. You may need additional tools to block the current thread, such as pipes or futures.

Here is an apps of using a `future` to implement a synchronous call.

```cpp
class SyncHelloServiceProxy : public HelloServiceBase
{
public:
    SyncHelloServiceProxy() = default;
    virtual ~SyncHelloServiceProxy() = default;
    virtual int Hello(HelloArgs args) override;
};

int SyncHelloServiceProxy::Hello(HelloArgs args)
{
    std::promise<int> p;
    std::future<int> f = p.get_future();
    std::function<void(int)> cb = [&p](int ret) {
        p.set_value(ret);
    };
    CallRPCAsync(cb, args);
    return f.get();
}

int main(int argc, char* argv[])
{
    auto&& ClientStub = RpcClient::GetRpcClient({ "127.0.0.1", 8888, "client.log" }); // Get an RPC Client object
    auto HelloPtr = ClientStub->GetProxy<AsyncHelloServiceProxy>(); // Get an RPC Proxy object
    HelloArgs args;
    args.set_username("Alice");
    int return_value = HelloPtr->Hello(args); // Call remote service and wait for response
    std::cout << "Server returned " << return_value << std::endl;
    return 0;
}
```

Running this client will print "Server says: hello world!" on the main thread, instead of the asynchronous thread through a callback.

## Layer Design

The layer division of this system is similar to trpc-cpp.

### 1. **Serialization Layer**

The Serialization Layer is used to convert complex data types (such as objects or structs) into a byte stream that can be transmitted over the network.

### 2. **Common Layer**

The Common layer implements general features like the RPC call interface, global UID management, asynchronous logging, and timer management.

### 3. **Runtime Module**

The **Runtime Module** provides the execution environment for the RPC framework.

#### 1. **Event Handler Abstraction**

The event handler abstraction enables the runtime to manage different events, such as incoming connect() request or RPC packets, without directly coupling the application logic to event-driven mechanisms.

#### 2. **Thread Pool**

The **Thread Pool** ensures that the framework can handle multiple concurrent RPC requests in a scalable and efficient manner.

#### 3. **Reactor Model**

The **Reactor Model** enables asynchronous, event-driven handling of I/O operations.

### 4. **Transport layer**

The Transport layer wraps Linux system calls for low-level socket communication, establishing a TCP channel between the server and client and passing received data to the Runtime layer.

### 5. **Client Layer**

Send requests for remote procedure calls (RPC) to the server and waiting return values.

### 5. **Server Layer**

Map client requests to specific functions or methods on the server. Marshall execution of the requested remote procedure and sending the result back to the client.As well as error handling and sending appropriate failure responses.
