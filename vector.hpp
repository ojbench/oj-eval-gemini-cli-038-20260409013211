#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <string>
#include <memory>
#include <type_traits>
#include <utility>
#include <stdexcept>

namespace sjtu {

class exception {
protected:
    const std::string variant = "";
    std::string detail = "";
public:
    exception() {}
    exception(const exception &ec) : variant(ec.variant), detail(ec.detail) {}
    virtual std::string what() {
        return variant + " " + detail;
    }
};

class index_out_of_bound : public exception {};
class runtime_error : public exception {};
class invalid_iterator : public exception {};
class container_is_empty : public exception {};

template<typename T>
class vector {
private:
    T* data_;
    size_t size_;
    size_t capacity_;

    void reallocate(size_t new_capacity) {
        if constexpr (std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>) {
            T* new_data = reinterpret_cast<T*>(std::realloc(data_, new_capacity * sizeof(T)));
            if (!new_data && new_capacity > 0) throw std::bad_alloc();
            data_ = new_data;
        } else {
            T* new_data = reinterpret_cast<T*>(operator new(new_capacity * sizeof(T)));
            for (size_t i = 0; i < size_; ++i) {
                new (new_data + i) T(std::move(data_[i]));
                data_[i].~T();
            }
            if (data_) {
                operator delete(data_);
            }
            data_ = new_data;
        }
        capacity_ = new_capacity;
    }

public:
    class const_iterator;
    class iterator {
    private:
        T* ptr_;
        vector<T>* vec_;
    public:
        iterator(T* ptr = nullptr, vector<T>* vec = nullptr) : ptr_(ptr), vec_(vec) {}
        
        iterator operator+(const int &n) const { return iterator(ptr_ + n, vec_); }
        iterator operator-(const int &n) const { return iterator(ptr_ - n, vec_); }
        int operator-(const iterator &rhs) const {
            if (vec_ != rhs.vec_) throw invalid_iterator();
            return ptr_ - rhs.ptr_;
        }
        iterator& operator+=(const int &n) { ptr_ += n; return *this; }
        iterator& operator-=(const int &n) { ptr_ -= n; return *this; }
        iterator operator++(int) { iterator tmp = *this; ptr_++; return tmp; }
        iterator& operator++() { ptr_++; return *this; }
        iterator operator--(int) { iterator tmp = *this; ptr_--; return tmp; }
        iterator& operator--() { ptr_--; return *this; }
        T& operator*() const { return *ptr_; }
        T* operator->() const { return ptr_; }
        bool operator==(const iterator &rhs) const { return ptr_ == rhs.ptr_; }
        bool operator==(const const_iterator &rhs) const { return ptr_ == rhs.ptr_; }
        bool operator!=(const iterator &rhs) const { return ptr_ != rhs.ptr_; }
        bool operator!=(const const_iterator &rhs) const { return ptr_ != rhs.ptr_; }
        friend class const_iterator;
    };

    class const_iterator {
    private:
        const T* ptr_;
        const vector<T>* vec_;
    public:
        const_iterator(const T* ptr = nullptr, const vector<T>* vec = nullptr) : ptr_(ptr), vec_(vec) {}
        const_iterator(const iterator& other) : ptr_(other.ptr_), vec_(other.vec_) {}
        
        const_iterator operator+(const int &n) const { return const_iterator(ptr_ + n, vec_); }
        const_iterator operator-(const int &n) const { return const_iterator(ptr_ - n, vec_); }
        int operator-(const const_iterator &rhs) const {
            if (vec_ != rhs.vec_) throw invalid_iterator();
            return ptr_ - rhs.ptr_;
        }
        const_iterator& operator+=(const int &n) { ptr_ += n; return *this; }
        const_iterator& operator-=(const int &n) { ptr_ -= n; return *this; }
        const_iterator operator++(int) { const_iterator tmp = *this; ptr_++; return tmp; }
        const_iterator& operator++() { ptr_++; return *this; }
        const_iterator operator--(int) { const_iterator tmp = *this; ptr_--; return tmp; }
        const_iterator& operator--() { ptr_--; return *this; }
        const T& operator*() const { return *ptr_; }
        const T* operator->() const { return ptr_; }
        bool operator==(const iterator &rhs) const { return ptr_ == rhs.ptr_; }
        bool operator==(const const_iterator &rhs) const { return ptr_ == rhs.ptr_; }
        bool operator!=(const iterator &rhs) const { return ptr_ != rhs.ptr_; }
        bool operator!=(const const_iterator &rhs) const { return ptr_ != rhs.ptr_; }
        friend class iterator;
    };

    vector() : data_(nullptr), size_(0), capacity_(0) {}
    
    vector(const vector &other) : size_(other.size_), capacity_(other.size_) {
        if (capacity_ > 0) {
            if constexpr (std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>) {
                data_ = reinterpret_cast<T*>(std::malloc(capacity_ * sizeof(T)));
                std::memcpy(data_, other.data_, size_ * sizeof(T));
            } else {
                data_ = reinterpret_cast<T*>(operator new(capacity_ * sizeof(T)));
                for (size_t i = 0; i < size_; ++i) {
                    new (data_ + i) T(other.data_[i]);
                }
            }
        } else {
            data_ = nullptr;
        }
    }
    
    vector(vector &&other) noexcept : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }
    
    ~vector() {
        if (data_) {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                for (size_t i = 0; i < size_; ++i) {
                    data_[i].~T();
                }
                operator delete(data_);
            } else {
                std::free(data_);
            }
        }
    }
    
    vector &operator=(const vector &other) {
        if (this == &other) return *this;
        if (data_) {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                for (size_t i = 0; i < size_; ++i) {
                    data_[i].~T();
                }
                operator delete(data_);
            } else {
                std::free(data_);
            }
        }
        size_ = other.size_;
        capacity_ = other.size_;
        if (capacity_ > 0) {
            if constexpr (std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>) {
                data_ = reinterpret_cast<T*>(std::malloc(capacity_ * sizeof(T)));
                std::memcpy(data_, other.data_, size_ * sizeof(T));
            } else {
                data_ = reinterpret_cast<T*>(operator new(capacity_ * sizeof(T)));
                for (size_t i = 0; i < size_; ++i) {
                    new (data_ + i) T(other.data_[i]);
                }
            }
        } else {
            data_ = nullptr;
        }
        return *this;
    }
    
    vector &operator=(vector &&other) noexcept {
        if (this == &other) return *this;
        if (data_) {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                for (size_t i = 0; i < size_; ++i) {
                    data_[i].~T();
                }
                operator delete(data_);
            } else {
                std::free(data_);
            }
        }
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        return *this;
    }
    
    T &at(const size_t &pos) {
        if (pos >= size_) throw index_out_of_bound();
        return data_[pos];
    }
    
    const T &at(const size_t &pos) const {
        if (pos >= size_) throw index_out_of_bound();
        return data_[pos];
    }
    
    T &operator[](const size_t &pos) {
        return data_[pos];
    }
    
    const T &operator[](const size_t &pos) const {
        return data_[pos];
    }
    
    const T &front() const {
        if (size_ == 0) throw container_is_empty();
        return data_[0];
    }
    
    const T &back() const {
        if (size_ == 0) throw container_is_empty();
        return data_[size_ - 1];
    }
    
    iterator begin() { return iterator(data_, this); }
    const_iterator begin() const { return const_iterator(data_, this); }
    const_iterator cbegin() const { return const_iterator(data_, this); }
    
    iterator end() { return iterator(data_ + size_, this); }
    const_iterator end() const { return const_iterator(data_ + size_, this); }
    const_iterator cend() const { return const_iterator(data_ + size_, this); }
    
    bool empty() const { return size_ == 0; }
    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    
    void clear() {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (size_t i = 0; i < size_; ++i) {
                data_[i].~T();
            }
        }
        size_ = 0;
    }
    
    iterator insert(iterator pos, const T &value) {
        size_t idx = pos.ptr_ - data_;
        if (size_ == capacity_) {
            reallocate(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        if constexpr (std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>) {
            if (idx < size_) {
                std::memmove(data_ + idx + 1, data_ + idx, (size_ - idx) * sizeof(T));
            }
            new (data_ + idx) T(value);
        } else {
            if (idx < size_) {
                new (data_ + size_) T(std::move(data_[size_ - 1]));
                for (size_t i = size_ - 1; i > idx; --i) {
                    data_[i] = std::move(data_[i - 1]);
                }
                data_[idx] = value;
            } else {
                new (data_ + idx) T(value);
            }
        }
        size_++;
        return iterator(data_ + idx, this);
    }
    
    iterator insert(const size_t &ind, const T &value) {
        if (ind > size_) throw index_out_of_bound();
        return insert(iterator(data_ + ind, this), value);
    }
    
    iterator erase(iterator pos) {
        size_t idx = pos.ptr_ - data_;
        if constexpr (std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>) {
            data_[idx].~T();
            if (idx < size_ - 1) {
                std::memmove(data_ + idx, data_ + idx + 1, (size_ - idx - 1) * sizeof(T));
            }
        } else {
            for (size_t i = idx; i < size_ - 1; ++i) {
                data_[i] = std::move(data_[i + 1]);
            }
            data_[size_ - 1].~T();
        }
        size_--;
        return iterator(data_ + idx, this);
    }
    
    iterator erase(const size_t &ind) {
        if (ind >= size_) throw index_out_of_bound();
        return erase(iterator(data_ + ind, this));
    }
    
    void push_back(const T &value) {
        if (size_ == capacity_) {
            reallocate(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        new (data_ + size_) T(value);
        size_++;
    }
    
    void push_back(T &&value) {
        if (size_ == capacity_) {
            reallocate(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        new (data_ + size_) T(std::move(value));
        size_++;
    }
    
    void pop_back() {
        if (size_ == 0) throw container_is_empty();
        data_[size_ - 1].~T();
        size_--;
    }
};

}

#endif
