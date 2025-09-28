#ifndef INCLUDE_SAVED_QUEUE_HPP_
#define INCLUDE_SAVED_QUEUE_HPP_

#include <list>

template <typename T>
class SavedQueue {

protected:
    std::list<T> elements_;
    typename decltype(elements_)::iterator head_, tail_;
    std::size_t queue_size_;

public:
    SavedQueue()
    :
        elements_(),
        head_(elements_.end()),
        tail_(elements_.end()),
        queue_size_(0)
    {
        
    }

    void add_element(const T& element)
    {
        auto pos = tail_;
        if (pos != elements_.end()) {
            pos++;
            elements_.insert(pos, element);
        } else {
            elements_.insert(pos, element);
            head_ = elements_.begin();
            tail_ = head_;
        }
    }

    void add_element(T&& element)
    {
        auto pos = tail_;
        if (pos != elements_.end()) {
            pos++;
            elements_.insert(pos, std::move(element));
        } else {
            elements_.insert(pos, std::move(element));
            head_ = elements_.begin();
            tail_ = head_;
        }
    }

    void push()
    {
        if (queue_size_ == elements_.size()) {
            add_element(T{});
        }

        if (queue_size_ != 0) {
            tail_++;
            if (tail_ == elements_.end()) {
                tail_ = elements_.begin();
            }
        }

        queue_size_++;
    }

    bool pop()
    {
        if (queue_size_ == 0) {
            return false;
        }

        if (queue_size_ > 1) {
            head_++;
            if (head_ == elements_.end()) {
                head_ = elements_.begin();
            }
        }

        queue_size_--;
        return true;
    }

    bool empty() const
    {
        return queue_size_ == 0;
    }

    std::size_t size() const
    {
        return queue_size_;
    }

    bool elements_empty() const
    {
        return elements_.empty();
    }

    std::size_t elements_size() const
    {
        return elements_.size();
    }

    T* front()
    {
        if (queue_size_ == 0) {
            return nullptr;
        }

        return &*head_;
    }

    T* back()
    {
        if (queue_size_ == 0) {
            return nullptr;
        }

        return &*tail_;
    }

};

#endif  // INCLUDE_SAVED_QUEUE_HPP_
