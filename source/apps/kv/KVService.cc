#include <apps/kv/KVService.hh>

static int getAllottedBucket(int k)
{
    return (k / 2000) % 5;
}

GetValueRsp KVServiceBase::GetValueInternal(GetValueReq req)
{
    int index = getAllottedBucket(req.id());
    GetValueRsp rsp;

    dict_locks[index].lock();
    auto it = dicts[index].find(req.id());
    if (it != dicts[index].end()) {
        rsp.set_name(it->second);
    }
    else {
        rsp.set_name("not found");
    }
    dict_locks[index].unlock();

    return rsp;
}

SetValueRsp KVServiceBase::SetValueInternal(SetValueReq req)
{
    int index = getAllottedBucket(req.id());
    SetValueRsp rsp;

    dict_locks[index].lock();
    auto it = dicts[index].find(req.id());
    if (it != dicts[index].end()) {
        it->second = req.name();
        rsp.set_code(1);
    }
    else {
        dicts[index][req.id()] = req.name();
        rsp.set_code(2);
    }
    dict_locks[index].unlock();

    return rsp;
}
