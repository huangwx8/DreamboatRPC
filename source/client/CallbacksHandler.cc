// std c
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>

// linux
#include <sys/socket.h>

#include <client/CallbacksHandler.hh>

#include <common/RpcUtils.hh>
#include <common/RpcTypes.hh>
#include <common/Guid.hh>
#include <common/Defines.hh>
#include <common/Logger.hh>


CallbacksHandler::CallbacksHandler():
    CallidCallbackMapping(MAX_REQUESTS, nullptr)
{
    
}

CallbacksHandler::~CallbacksHandler()
{

}

static std::string logData(const void* data, size_t size) {
    const unsigned char* byteData = static_cast<const unsigned char*>(data);
    std::stringstream ss;  // Use stringstream to accumulate the log in a string

    // Log each byte in hexadecimal format
    for (size_t i = 0; i < size; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)byteData[i] << " ";
    }

    // Convert the stringstream to a std::string and return it
    return ss.str();
}

void CallbacksHandler::HandleReadEvent(int Fd)
{
    RpcResult res;
    // bzero receive buffer and receive
    bzero(&res, sizeof(RpcResult));
    int ret = recv(Fd, &res, sizeof(RpcResult), 0);
    if (ret == -1)
    {
        log_dev("CallbacksHandler::HandleReadEvent: receive failed.\n");
        return;
    }

    auto data = logData(&res, sizeof(RpcResult));

    printf("CallbacksHandler::HandleReadEvent fd=%d, seqno=%d, data=%s\n", Fd, res.seqno, data.c_str());

    CallidCallbackMapping[res.seqno]->Exec(res.return_buffer);

    delete CallidCallbackMapping[res.seqno];

    CallidCallbackMapping[res.seqno] = nullptr;

    Guid::RecycleGuid(res.seqno);
}
