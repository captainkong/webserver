#include "heaptimer.h"

using std::cout;
using std::endl;

HeapTimer::HeapTimer()
{
    heap_.reserve(128);
}

HeapTimer::~HeapTimer()
{
    heap_.clear();
    ref_.clear();
}

void HeapTimer::add(size_t id, size_t msTimeOut, timeOutCallBack _cb)
{
    // 单线程暂时不考虑线程安全问题
    if (ref_.find(id) == ref_.end())
    {
        // 新增
        size_t idx = heap_.size();
        heap_.emplace_back(timerNode{id, clocks::now() + MS(msTimeOut), _cb});
        ref_[id] = idx;
        adjUp_(idx);
    }
    else
    {
        // 更新节点
        size_t idx = ref_[id];
        heap_[idx].cb = _cb;
        heap_[idx].expire = clocks::now() + MS(msTimeOut);
        if (!adjDown_(idx, heap_.size() - 1))
            adjUp_(idx);
    }
    // cout << "add:" << id << ",idx=" << ref_[id] << endl;
}

void HeapTimer::update(size_t id, size_t msTimeOut)
{
    if (ref_.find(id) == ref_.end())
        return;
    int idx = ref_[id];
    // cout << "update:idx=" << idx << endl;
    heap_[idx].expire = clocks::now() + MS(msTimeOut);
    if (!adjDown_(idx, heap_.size() - 1))
        adjUp_(idx);
}

// 处理过期定时器
void HeapTimer::tick()
{
    // cout << "beg size:" << heap_.size() << endl;
    // display();
    while (heap_.size() > 0 && heap_[0].expire < clocks::now())
    {
        heap_[0].cb();
        del_(0);
        // display();
    }
    // cout << "end size:" << heap_.size() << endl;
}

int HeapTimer::getNextExpireTime()
{
    int ans = -1;
    tick();
    if (heap_.size() > 0)
    {
        ans = std::chrono::duration_cast<MS>(heap_[0].expire - clocks::now()).count();
        ans = std::max(ans, 0);
    }
    return ans;
}

void HeapTimer::adjUp_(size_t idx)
{
    while (idx > 0)
    {
        size_t fi = (idx - 1) / 2;
        if (heap_[idx] < heap_[fi])
        {
            swap_(idx, fi);
            idx = fi;
        }
        else
        {
            break;
        }
    }
}

bool HeapTimer::adjDown_(size_t idx, size_t lastIndex)
{
    size_t base = idx;
    while (base * 2 + 1 <= lastIndex)
    {
        size_t small = base, lson = base * 2 + 1, rson = base * 2 + 2;
        if (heap_[lson] < heap_[base])
        {
            small = lson;
        }
        if (rson <= lastIndex && heap_[rson] < heap_[small])
        {
            small = rson;
        }
        if (small == base)
            break;
        swap_(small, base);
        base = small;
    }
    return base != idx;
}

void HeapTimer::del_(size_t idx)
{
    // cout << "del " << idx << endl;
    // 将节点移动到最后,然后再删掉
    size_t last = heap_.size() - 1;
    size_t id = heap_[idx].id;

    if (idx != last)
    {
        swap_(idx, last);
        adjDown_(idx, last - 1);
    }

    heap_.pop_back();
    ref_.erase(id);
    // cout << "end of del:" << idx << ",len=" << heap_.size() << endl;
}

void HeapTimer::swap_(size_t left, size_t right)
{
    // cout << "beg  swap left:" << left << ",right=" << right << endl;
    // display();
    std::swap(heap_[left], heap_[right]);
    ref_[heap_[left].id] = left;
    ref_[heap_[right].id] = right;
    // cout << "end swap" << endl;
    // display();
}

void HeapTimer::display()
{
    // for (auto &n : heap_)
    // {
    //     cout << n.id << ",";
    // }
    int n = heap_.size();
    for (int i = 0; i < n; ++i)
    {
        cout << "i=" << i << ",id=" << heap_[i].id << ",check=" << ref_[heap_[i].id] << endl;
    }
    cout << endl;
}