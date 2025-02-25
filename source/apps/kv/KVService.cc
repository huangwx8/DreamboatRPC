#include <apps/kv/KVService.hh>

static int getAllottedBucket(int k)
{
    return (k / 200000) % 5;
}

std::string KVServiceBase::GetValueInternal(GetValueArgs args)
{
    int index = getAllottedBucket(args.id());
    std::string name;

    dict_locks[index].lock();
    auto it = dicts[index].find(args.id());
    if (it != dicts[index].end()) {
        name = it->second;
    }
    else {
        name = "not found";
    }
    dict_locks[index].unlock();

    return name;
}

int KVServiceBase::SetValueInternal(SetValueArgs args)
{
    int index = getAllottedBucket(args.id());
    int ret = 0;

    dict_locks[index].lock();
    auto it = dicts[index].find(args.id());
    if (it != dicts[index].end()) {
        it->second = args.name();
        ret = 1;
    }
    else {
        dicts[index][args.id()] = args.name();
        ret = 2;
    }
    dict_locks[index].unlock();

    return ret;
}
