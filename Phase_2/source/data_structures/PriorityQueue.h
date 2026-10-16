#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <vector>
#include <functional>
#include <stdexcept>
#include <utility>
#include <cstddef>

namespace GeoLocalMiner {

template <typename T, typename Compare = std::less<T>>
class CustomPriorityQueue
{
private:

    std::vector<T> heap;
    Compare comp;


    void siftUp(size_t index)
    {
        while (index > 0)
        {
            size_t parent = (index - 1) / 2;

            if (!comp(heap[parent], heap[index]))
                break;

            std::swap(heap[parent], heap[index]);
            index = parent;
        }
    }


    void siftDown(size_t index)
    {
        const size_t n = heap.size();

        while (true)
        {
            size_t best = index;
            size_t left = 2 * index + 1;
            size_t right = left + 1;

            if (left < n &&
                comp(heap[best], heap[left]))
            {
                best = left;
            }

            if (right < n &&
                comp(heap[best], heap[right]))
            {
                best = right;
            }

            if (best == index)
                break;

            std::swap(heap[index], heap[best]);
            index = best;
        }
    }


public:

    CustomPriorityQueue() = default;

    explicit CustomPriorityQueue(
        const Compare& comparator
    )
        : comp(comparator)
    {
    }


    template <typename Iterator>
    CustomPriorityQueue(
        Iterator first,
        Iterator last,
        const Compare& comparator = Compare()
    )
        : heap(first, last),
          comp(comparator)
    {
        if (heap.size() < 2)
            return;

        for (size_t i = heap.size() / 2; i > 0; --i)
            siftDown(i - 1);
    }


    void push(const T& value)
    {
        heap.push_back(value);
        siftUp(heap.size() - 1);
    }


    void push(T&& value)
    {
        heap.push_back(std::move(value));
        siftUp(heap.size() - 1);
    }


    void pop()
    {
        if (heap.empty())
            throw std::underflow_error(
                "Cannot pop from empty PriorityQueue"
            );

        if (heap.size() == 1)
        {
            heap.pop_back();
            return;
        }

        heap[0] = std::move(heap.back());
        heap.pop_back();

        siftDown(0);
    }


    const T& top() const
    {
        if (heap.empty())
            throw std::underflow_error(
                "Cannot access empty PriorityQueue"
            );

        return heap.front();
    }


    bool empty() const
    {
        return heap.empty();
    }


    size_t size() const
    {
        return heap.size();
    }


    void clear()
    {
        heap.clear();
    }


    std::vector<T> extractTopN(size_t n) const
    {
        CustomPriorityQueue copy(*this);

        n = std::min(n, copy.size());

        std::vector<T> result;
        result.reserve(n);

        while (n--)
        {
            result.push_back(copy.top());
            copy.pop();
        }

        return result;
    }


    std::vector<T> extractAllSorted()
    {
        std::vector<T> result;
        result.reserve(heap.size());

        while (!heap.empty())
        {
            result.push_back(top());
            pop();
        }

        return result;
    }
};

}

#endif