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

    CallidCallbackMapping[res.seqno]->Exec(res.return_buffer);

    delete CallidCallbackMapping[res.seqno];

    CallidCallbackMapping[res.seqno] = nullptr;

    Guid::RecycleGuid(res.seqno);
}
