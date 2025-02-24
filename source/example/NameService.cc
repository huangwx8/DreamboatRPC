#include <example/NameService.hh>

static int get_belonged_bucket(int k)
{
    if (k < 200000)
    {
        return 0;
    }
    else if (k < 400000)
    {
        return 1;
    }
    else if (k < 600000)
    {
        return 2;
    }
    else if (k < 800000)
    {
        return 3;
    }
    else 
    {
        return 4;
    }
}

std::string NameServiceBase::GetNameInternal(GetNameArgs args)
{
    int index = get_belonged_bucket(args.id());
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

int NameServiceBase::SetNameInternal(SetNameArgs args)
{
    int index = get_belonged_bucket(args.id());
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
