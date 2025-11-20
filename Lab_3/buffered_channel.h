#ifndef BUFFERED_CHANNEL_H_
#define BUFFERED_CHANNEL_H_

#include <queue>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <utility>

template<class T>
class BufferedChannel {
 public:
    explicit BufferedChannel(int size) : capacity_(size), is_closed_(false) {}

    void Send(T value) {
        std::unique_lock<std::mutex> lock(mutex_);
        not_full_.wait(lock, [this]() {
            return queue_.size() < capacity_ || is_closed_;
        });
        if (is_closed_) {
            throw std::runtime_error("Channel is closed");
        }
        queue_.push(std::move(value));
        not_empty_.notify_one();
    }

    std::pair<T, bool> Recv() {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_.wait(lock, [this]() {
            return !queue_.empty() || is_closed_;
        });
        if (queue_.empty() && is_closed_) {
            return {T(), false};
        }
        T val = std::move(queue_.front());
        queue_.pop();
        not_full_.notify_one();
        return {std::move(val), true};
    }

    void Close() {
        std::unique_lock<std::mutex> lock(mutex_);
        is_closed_ = true;
        not_full_.notify_all();
        not_empty_.notify_all();
    }

 private:
    size_t capacity_;
    std::queue<T> queue_;
    bool is_closed_;
    std::mutex mutex_;
    std::condition_variable not_full_;
    std::condition_variable not_empty_;
};

#endif // BUFFERED_CHANNEL_H_