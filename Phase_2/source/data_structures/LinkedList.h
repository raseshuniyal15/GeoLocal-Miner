#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <cstddef>
#include <stdexcept>
#include <utility>

namespace GeoLocalMiner {

template <typename T>
class LinkedList
{
public:

    struct Node
    {
        T data;
        Node* prev = nullptr;
        Node* next = nullptr;

        explicit Node(const T& value)
            : data(value)
        {
        }

        explicit Node(T&& value)
            : data(std::move(value))
        {
        }
    };


    class Iterator
    {
        Node* node;

    public:
        explicit Iterator(Node* p = nullptr)
            : node(p)
        {
        }

        T& operator*() { return node->data; }
        T* operator->() { return &node->data; }

        Iterator& operator++()
        {
            if (node)
                node = node->next;
            return *this;
        }

        Iterator& operator--()
        {
            if (node)
                node = node->prev;
            return *this;
        }

        bool operator==(const Iterator& other) const
        {
            return node == other.node;
        }

        bool operator!=(const Iterator& other) const
        {
            return node != other.node;
        }
    };


    class ConstIterator
    {
        const Node* node;

    public:
        explicit ConstIterator(const Node* p = nullptr)
            : node(p)
        {
        }

        const T& operator*() const { return node->data; }
        const T* operator->() const { return &node->data; }

        ConstIterator& operator++()
        {
            if (node)
                node = node->next;
            return *this;
        }

        ConstIterator& operator--()
        {
            if (node)
                node = node->prev;
            return *this;
        }

        bool operator==(const ConstIterator& other) const
        {
            return node == other.node;
        }

        bool operator!=(const ConstIterator& other) const
        {
            return node != other.node;
        }
    };


private:

    Node* head = nullptr;
    Node* tail = nullptr;
    size_t count = 0;


    Node* nodeAt(size_t index) const
    {
        if (index >= count)
            throw std::out_of_range("LinkedList index out of range");

        if (index < count / 2)
        {
            Node* node = head;

            for (size_t i = 0; i < index; ++i)
                node = node->next;

            return node;
        }

        Node* node = tail;

        for (size_t i = count - 1; i > index; --i)
            node = node->prev;

        return node;
    }


    void linkBack(Node* node)
    {
        if (!tail)
        {
            head = tail = node;
        }
        else
        {
            node->prev = tail;
            tail->next = node;
            tail = node;
        }

        ++count;
    }


    void linkFront(Node* node)
    {
        if (!head)
        {
            head = tail = node;
        }
        else
        {
            node->next = head;
            head->prev = node;
            head = node;
        }

        ++count;
    }


public:

    LinkedList() = default;


    ~LinkedList()
    {
        clear();
    }


    LinkedList(const LinkedList& other)
    {
        for (const auto& value : other)
            push_back(value);
    }


    LinkedList& operator=(const LinkedList& other)
    {
        if (this != &other)
        {
            clear();

            for (const auto& value : other)
                push_back(value);
        }

        return *this;
    }


    LinkedList(LinkedList&& other) noexcept
        : head(other.head),
          tail(other.tail),
          count(other.count)
    {
        other.head = nullptr;
        other.tail = nullptr;
        other.count = 0;
    }


    LinkedList& operator=(LinkedList&& other) noexcept
    {
        if (this != &other)
        {
            clear();

            head = other.head;
            tail = other.tail;
            count = other.count;

            other.head = nullptr;
            other.tail = nullptr;
            other.count = 0;
        }

        return *this;
    }


    size_t size() const
    {
        return count;
    }


    bool empty() const
    {
        return count == 0;
    }


    void push_back(const T& value)
    {
        linkBack(new Node(value));
    }


    void push_back(T&& value)
    {
        linkBack(new Node(std::move(value)));
    }


    void push_front(const T& value)
    {
        linkFront(new Node(value));
    }


    void push_front(T&& value)
    {
        linkFront(new Node(std::move(value)));
    }


    void pop_back()
    {
        if (!tail)
            throw std::underflow_error(
                "Cannot pop_back from empty list"
            );

        Node* old = tail;
        tail = tail->prev;

        if (tail)
            tail->next = nullptr;
        else
            head = nullptr;

        delete old;
        --count;
    }


    void pop_front()
    {
        if (!head)
            throw std::underflow_error(
                "Cannot pop_front from empty list"
            );

        Node* old = head;
        head = head->next;

        if (head)
            head->prev = nullptr;
        else
            tail = nullptr;

        delete old;
        --count;
    }


    void clear()
    {
        while (head)
        {
            Node* next = head->next;
            delete head;
            head = next;
        }

        tail = nullptr;
        count = 0;
    }


    T& get(size_t index)
    {
        return nodeAt(index)->data;
    }


    const T& get(size_t index) const
    {
        return nodeAt(index)->data;
    }


    T& operator[](size_t index)
    {
        return get(index);
    }


    const T& operator[](size_t index) const
    {
        return get(index);
    }


    void removeAt(size_t index)
    {
        Node* node = nodeAt(index);

        if (node == head)
        {
            pop_front();
            return;
        }

        if (node == tail)
        {
            pop_back();
            return;
        }

        node->prev->next = node->next;
        node->next->prev = node->prev;

        delete node;
        --count;
    }


    Iterator begin()
    {
        return Iterator(head);
    }

    Iterator end()
    {
        return Iterator();
    }

    ConstIterator begin() const
    {
        return ConstIterator(head);
    }

    ConstIterator end() const
    {
        return ConstIterator();
    }
};

}

#endif