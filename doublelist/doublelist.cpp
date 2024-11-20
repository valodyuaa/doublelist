#include <initializer_list>
#include <iostream>
#include <memory>
#include <cassert>


template<class T>
struct Node 
{
    using LPtr = std::weak_ptr<Node>;
    using RPtr = std::shared_ptr<Node>;

    T data{};
    LPtr prev = {};
    RPtr next = {};

    Node(T data) : data{ std::move(data) } {}
};

template<class T>
struct List 
{
    using Item = Node<T>;
    using ItemPtr = std::shared_ptr<Item>;

    ItemPtr head{ nullptr };
    ItemPtr tail{ nullptr };
    std::size_t len{};

    List() 
    {
        head = std::make_shared<Item>(T());
        tail = std::make_shared<Item>(T());
        head->next = tail;
        tail->prev = head;
        len = 0;
    }

    List(const std::initializer_list<T>& items) : List() 
    {
        for (const auto& item : items) 
        {
            PushBack(item);
        }
    }

    List(const List& rhs) : List() 
    {
        ItemPtr current = rhs.head->next;
        while (current != rhs.tail) 
        {
            PushBack(current->data);
            current = current->next;
        }
    }

    List(List&& rhs) : List() 
    {
        Swap(rhs);
    }

    List& operator=(List rhs) 
    {
        if (this != &rhs) 
        {
            Swap(rhs);
        }
        return *this;
    }

    ~List()
    {
        ItemPtr current = head;
        while (current != tail) 
        {
            ItemPtr tmp = current;
            current = current->next;
            tmp->next = {};
            tmp->prev = {};
        }
    }

    bool operator==(const List& rhs) const noexcept 
    {
        if (this == &rhs) return true;
        if (len != rhs.len) return false;
        ItemPtr node1 = head->next;
        ItemPtr node2 = rhs.head->next;
        while (node1 != tail && node2 != rhs.tail) 
        {
            if (node1->data != node2->data) 
            {
                return false;
            }
            node1 = node1->next;
            node2 = node2->next;
        }
        return true;
    }

    void Swap(List& rhs) noexcept 
    {
        head = std::exchange(rhs.head, nullptr);
        tail = std::exchange(rhs.tail, nullptr);
        len = std::exchange(rhs.len, 0);
    }

    void PushFront(T data) 
    {
        PushFront(std::make_shared<Item>(std::move(data)));
    }

    void PushFront(const ItemPtr& node) 
    {
        node->prev = head;
        node->next = head->next;
        head->next->prev = node;
        head->next = node;
        len += 1;
    }

    void PushBack(T data)
    {
        PushBack(std::make_shared<Item>(std::move(data)));
    }

    void PushBack(const ItemPtr& node) 
    {
        node->prev = tail->prev;
        node->next = tail;
        tail->prev.lock()->next = node;
        tail->prev = node;
        len += 1;
    }

    void PushBack(const std::initializer_list<T>& items) 
    {
        for (const auto& item : items) 
        {
            PushBack(item);
        }
    }

    void Remove(ItemPtr node) 
    {
        if (!node) return;
        node->prev.lock()->next = node->next;
        node->next->prev = node->prev;
        node->prev = {};
        node->next = {};
        len -= 1;
    }

    void MoveToFront(ItemPtr node)
    {
        if (!node) return;
        Remove(node);
        PushFront(node);
    }

    void MoveToBack(ItemPtr node)
    {
        if (!node) return;
        Remove(node);
        PushBack(node);
    }

    ItemPtr Back() 
    {
        if (head->next == tail) throw std::runtime_error("List is empty");
        return tail->prev.lock();
    }

    ItemPtr Front() 
    {
        if (head->next == tail) throw std::runtime_error("List is empty");
        return head->next;
    }

    ItemPtr Search(const T& target) noexcept 
    {
        auto current = head->next;
        while (current != tail) 
        {
            if (current->data == target) 
            {
                return current;
            }
            current = current->next;
        }
        return {};
    }

    void Reverse() noexcept 
    {
        auto i = head->next;
        auto j = tail->prev.lock();
        while (i != tail && j != head && i != j && i->prev.lock() != j) 
        {
            std::swap(i->data, j->data);
            i = i->next;
            j = j->prev.lock();
        }
    }

    bool IsEmpty() const noexcept 
    {
        return head->next == tail;
    }

    std::size_t Size() const noexcept 
    {
        return len;
    }

    void Print() noexcept 
    {
        std::cout << "List(";
        ItemPtr current = head->next;
        while (current != tail) 
        {
            if (current != head->next) 
            {
                std::cout << " ";
            }
            std::cout << current->data;
            current = current->next;
        }
        std::cout << ")" << std::endl;
    }
};

int main() {
    auto list = List<int>({ 1,2 });
    list.PushBack(3);
    list.PushBack({ 4, 5 });
    list.PushFront(0);

    assert(list == List<int>({ 0,1,2,3,4,5 }));
    assert(list.Size() == 6);

    list.MoveToBack(list.Front());

    assert(list == List<int>({ 1,2,3,4,5,0 }));
    assert(list.Size() == 6);
    assert(list.Back()->data == 0);
    assert(list.Front()->data == 1);

    list.MoveToFront(list.Back());
    assert(list == List<int>({ 0,1,2,3,4,5 }));
    assert(list.Size() == 6);
    assert(list.Back()->data == 5);
    assert(list.Front()->data == 0);

    if (const auto& node = list.Search(2)) 
    {
        list.Remove(node);
        assert(list == List<int>({ 0,1,3,4,5 }));
    }

    list.Reverse();
    assert(list == List<int>({ 5,4,3,1,0 }));

    while (!list.IsEmpty()) 
    {
        list.Remove(list.Front());
    }
    assert(list == List<int>());
    assert(list.Size() == 0);

    {
        auto list = List<int>({ 1,2,3,4,5 });
        assert(list.Size() == 5);
    }

    {
        auto list = List<int>({ 1,2,3,4,5 });
        assert(list.Size() == 5);
        assert(list == List<int>({ 1,2,3,4,5 }));

        List<int> newList = list;
        list.Front()->data = -111;
        list.Back()->data = -111;
        assert(list == List<int>({ -111,2,3,4,-111 }));
        assert(newList == List<int>({ 1,2,3,4,5 }));
        assert(list.Front()->data == -111);
        assert(list.Back()->data == -111);
        assert(newList.Front()->data == 1);
        assert(newList.Back()->data == 5);
    }

    std::cout << "OK" << std::endl;
}