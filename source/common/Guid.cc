#include <common/Guid.hh>
#include <mutex>

#define MAX_GUID 1024
#define GET(i) ((size_t)i < Heap.size() ? Heap[i] : 0)

std::vector<int> Guid::Heap = std::vector<int>();
int Guid::Bound = 0;
std::mutex HeapMutex;  // Mutex to protect access to Heap and Bound

int Guid::GetGuid()
{
    std::lock_guard<std::mutex> lock(HeapMutex);

    auto Popitem = []() -> int 
    {
        int ret = Heap[0];
        std::swap(*(Heap.begin()), *(Heap.end()-1));
        Heap.pop_back();

        int nid = 0;
        // percolate down
        while (1) 
        {
            int nvalue = Heap[nid];
            int lid = (nid << 1) + 1, rid = lid + 1;
            int lvalue = GET(lid), rvalue = GET(rid);
            int nextnid = nid;
            if (lvalue && nvalue > lvalue)
            {
                nvalue = lvalue;
                std::swap(Heap[nid], Heap[lid]);
                nextnid = lid;
            }
            if (rvalue && nvalue > rvalue)
            {
                std::swap(Heap[nid], Heap[rid]);
                if (nextnid == nid)
                {
                    nextnid = rid;
                }
            }
            if (nextnid == nid)
            {
                break;
            }
            nid = nextnid;
        }
        return ret;
    };

    if (Heap.empty())
    {
        if (Bound >= MAX_GUID)
        {
            return -1;
        }
        else
        {
            return ++Bound;
        }
    }
    else 
    {
        return Popitem();
    }
}

void Guid::RecycleGuid(int id)
{
    std::lock_guard<std::mutex> lock(HeapMutex);
    
    auto Pushitem = [](int id)
    {
        Heap.push_back(id);
        
        int nid = Heap.size() - 1;
        // percolate up
        while (nid != 0) 
        {
            int rootid = (nid - 1) / 2;
            if (Heap[rootid] > Heap[nid])
            {
                std::swap(Heap[nid], Heap[rootid]);
                nid = rootid;
            }
            else 
            {
                break;
            }
        }
    };

    Pushitem(id);
}
