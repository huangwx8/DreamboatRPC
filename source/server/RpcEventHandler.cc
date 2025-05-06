// std c/c++
#include <assert.h>
#include <stdio.h>
#include <string.h>

// unix
#include <sys/socket.h>
#include <iostream>
#include <sstream>
#include <iomanip>

// inner
#include <server/RpcEventHandler.hh>
#include <common/Defines.hh> 
#include <common/Logger.hh>
#include <common/RpcUtils.hh>
#include <common/RpcTypes.hh>
#include <common/RpcServiceBase.hh>

RpcResult ReturnValuePipe::Read(int fd) 
{
    assert(!_return_values[fd].empty());
    // 从fd对应的等待队列中拿一个元素
    RpcResult result = _return_values[fd].front();
    _return_values[fd].pop();
    return result;
}

void ReturnValuePipe::Write(int fd, RpcResult result) 
{
    _return_values[fd].push(result);
}

RpcRequestHandler::RpcRequestHandler(ReturnValuePipe* pipe, FileDescriptorEventDelegate* delegate): _pipe(pipe), _finished(delegate) {}

void RpcRequestHandler::HandleReadEvent(int connfd)
{
    RpcMessage Message;
    RpcResult Result;
    int recv_bytes;

    recv_bytes = recv(connfd, &Message.header, sizeof(RpcHeader), 0); // read header
    if (recv_bytes < 0)
    {
        // bad read
        if (errno != EAGAIN)
        {
            log_err("RpcRequestHandler::HandleReadEvent: Bad recv\n");
            exit(1);
        }
        return;
    }
    else if (recv_bytes != sizeof(RpcHeader))
    {
        log_err("RpcRequestHandler::HandleReadEvent: Bad packet\n");
        return;
    }
    else 
    {
        recv_bytes = recv(connfd, &Message.body, Message.header.body_length, 0); // read body
        if (recv_bytes != Message.header.body_length) 
        {
            log_err("RpcRequestHandler::HandleReadEvent: Bad packet\n");
            return;
        }

        log_dev("RpcRequestHandler::HandleReadEvent: get rpc request from %d\n", connfd);
        if (Message.header.magic != RPC_MAGIC_NUMBER) {
            log_dev("RpcRequestHandler::HandleReadEvent: magic number does not match, [%d] excepted but [%d] received\n",
                RPC_MAGIC_NUMBER, Message.header.magic);
            return;
        }
        if (Message.header.version != RPC_VERSION) {
            log_dev("RpcRequestHandler::HandleReadEvent: version does not match, [%d] excepted but [%d] received\n",
                RPC_VERSION, Message.header.version);
            return;
        }
        std::string servicename = Message.header.servicename;
        if (_dictionary.find(servicename) != _dictionary.end())
        {
            RpcServiceBase* Service = _dictionary[servicename];
            Result = Service->Handle(Message);
        }
        else 
        {
            log_dev("RpcRequestHandler::HandleReadEvent: Warning! Service [%s] not found\n", Message.header.servicename);
            return;
        }
        
        Result.seqno = Message.header.seqno;

        // 如果报文中明确标识了需要服务端发回返回值，则需要把这个Result入队，并激活OUT事件
        if (Message.header.need_return) {
            _pipe->Write(connfd, Result);
        }
        _finished->FileDescriptorEventDone(connfd);
    }
}

void RpcRequestHandler::AddProxy(RpcServiceBase* Service)
{
    std::string ServiceName = Service->GetServiceName();
    if (_dictionary.find(ServiceName) == _dictionary.end())
    {
        _dictionary.insert(std::make_pair(ServiceName, Service));
    }
}

RpcResultSender::RpcResultSender(ReturnValuePipe* pipe, FileDescriptorEventDelegate* delegate): _pipe(pipe), _finished(delegate) {}

void RpcResultSender::HandleWriteEvent(int connfd)
{
    RpcResult Result = _pipe->Read(connfd);
    int ret = send(connfd, &Result, sizeof(RpcResult), 0);
    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            log_dev("Socket buffer full, try again later\n");
        } else {
            log_err("Send failed, errno = %d\n", errno);
            assert(false);
        }
    }

    _finished->FileDescriptorEventDone(connfd);
}
