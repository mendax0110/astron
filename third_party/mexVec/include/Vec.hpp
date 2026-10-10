#pragma once

#include <algorithm>
#include <bit>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <new>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace mex
{
    /**
     * @brief High-performance dynamic array container
     *
     * @tparam T Element type
     * @tparam SmallSize Number of elements to store inline (SBO optimization)
     * @tparam GrowthFactor Growth factor numerator (denominator is 10)
     *
     * Features:
     * - Small Buffer Optimization: stores up to SmallSize elements inline
     * - Optimized for trivially copyable types using memcpy
     * - Configurable growth factor (default 1.5x instead of 2x for better memory usage)
     * - Exception-safe operations with strong guarantee
     * - Full iterator and ranges support
     */
    template<typename T, std::size_t SmallSize = 8, std::size_t GrowthFactor = 15>
    class Vec
    {
        static_assert(GrowthFactor > 10, "Growth factor must be > 1.0 (represented as GrowthFactor/10)");

    public:
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;
        using iterator = T*;
        using const_iterator = const T*;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        /**
         * @brief Default constructor
         *
         * Constructs an empty vector with inline storage ready.
         */
        constexpr Vec() noexcept;

        /**
         * @brief Constructs vector with count default-inserted elements
         *
         * @param count Number of elements to create
         */
        constexpr explicit Vec(size_type count);

        /**
         * @brief Constructs vector with count copies of value
         *
         * @param count Number of elements to create
         * @param value Value to copy
         */
        constexpr Vec(size_type count, const T& value);

        /**
         * @brief Range constructor
         *
         * @tparam InputIt Iterator type
         * @param first Beginning of range
         * @param last End of range
         */
        template<std::input_iterator InputIt>
        constexpr Vec(InputIt first, InputIt last);

        /**
         * @brief Copy constructor
         *
         * @param other Vector to copy from
         */
        constexpr Vec(const Vec& other);

        /**
         * @brief Move constructor
         *
         * @param other Vector to move from
         */
        constexpr Vec(Vec&& other) noexcept;

        /**
         * @brief Initializer list constructor
         *
         * @param init Initializer list
         */
        constexpr Vec(std::initializer_list<T> init);

        /**
         * @brief Destructor
         */
        constexpr ~Vec();

        /**
         * @brief Copy assignment operator
         *
         * @param other Vector to copy from
         * @return Reference to this
         */
        constexpr Vec& operator=(const Vec& other);

        /**
         * @brief Move assignment operator
         *
         * @param other Vector to move from
         * @return Reference to this
         */
        constexpr Vec& operator=(Vec&& other) noexcept;

        /**
         * @brief Initializer list assignment
         *
         * @param init Initializer list
         * @return Reference to this
         */
        constexpr Vec& operator=(std::initializer_list<T> init);

        /**
         * @brief Assigns count copies of value
         *
         * @param count Number of elements
         * @param value Value to copy
         */
        constexpr void assign(size_type count, const T& value);

        /**
         * @brief Assigns range to vector
         *
         * @tparam InputIt Iterator type
         * @param first Beginning of range
         * @param last End of range
         */
        template<std::input_iterator InputIt>
        constexpr void assign(InputIt first, InputIt last);

        /**
         * @brief Assigns initializer list to vector
         *
         * @param init Initializer list
         */
        constexpr void assign(std::initializer_list<T> init);

        /**
         * @brief Access element with bounds checking
         *
         * @param pos Position of element
         * @return Reference to element
         * @throws std::out_of_range if pos >= size()
         */
        constexpr reference at(size_type pos);

        /**
         * @brief Access element with bounds checking (const)
         *
         * @param pos Position of element
         * @return Const reference to element
         * @throws std::out_of_range if pos >= size()
         */
        constexpr const_reference at(size_type pos) const;

        /**
         * @brief Access element without bounds checking
         *
         * @param pos Position of element
         * @return Reference to element
         */
        constexpr reference operator[](size_type pos) noexcept;

        /**
         * @brief Access element without bounds checking (const)
         *
         * @param pos Position of element
         * @return Const reference to element
         */
        constexpr const_reference operator[](size_type pos) const noexcept;

        /**
         * @brief Access first element
         *
         * @return Reference to first element
         */
        constexpr reference front() noexcept;

        /**
         * @brief Access first element (const)
         *
         * @return Const reference to first element
         */
        constexpr const_reference front() const noexcept;

        /**
         * @brief Access last element
         *
         * @return Reference to last element
         */
        constexpr reference back() noexcept;

        /**
         * @brief Access last element (const)
         *
         * @return Const reference to last element
         */
        constexpr const_reference back() const noexcept;

        /**
         * @brief Direct access to underlying array
         *
         * @return Pointer to underlying data
         */
        constexpr pointer data() noexcept;

        /**
         * @brief Direct access to underlying array (const)
         *
         * @return Const pointer to underlying data
         */
        constexpr const_pointer data() const noexcept;

        // Iterators

        constexpr iterator begin() noexcept;
        constexpr const_iterator begin() const noexcept;
        constexpr const_iterator cbegin() const noexcept;

        constexpr iterator end() noexcept;
        constexpr const_iterator end() const noexcept;
        constexpr const_iterator cend() const noexcept;

        constexpr reverse_iterator rbegin() noexcept;
        constexpr const_reverse_iterator rbegin() const noexcept;
        constexpr const_reverse_iterator crbegin() const noexcept;

        constexpr reverse_iterator rend() noexcept;
        constexpr const_reverse_iterator rend() const noexcept;
        constexpr const_reverse_iterator crend() const noexcept;


        /**
         * @brief Check if vector is empty
         *
         * @return true if size() == 0
         */
        [[nodiscard]] constexpr bool empty() const noexcept;

        /**
         * @brief Get number of elements
         *
         * @return Number of elements in vector
         */
        [[nodiscard]] constexpr size_type size() const noexcept;

        /**
         * @brief Get maximum possible number of elements
         *
         * @return Maximum size
         */
        [[nodiscard]] constexpr size_type max_size() const noexcept;

        /**
         * @brief Reserve storage for at least new_cap elements
         *
         * @param new_cap Minimum capacity to reserve
         */
        constexpr void reserve(size_type new_cap);

        /**
         * @brief Get current storage capacity
         *
         * @return Number of elements that can be stored without reallocation
         */
        [[nodiscard]] constexpr size_type capacity() const noexcept;

        /**
         * @brief Reduce capacity to fit size
         *
         * Non-binding request to reduce capacity() to size().
         */
        constexpr void shrink_to_fit();

        /**
         * @brief Check if using inline storage (SBO)
         *
         * @return true if currently using small buffer optimization
         */
        [[nodiscard]] constexpr bool is_small() const noexcept;

        /**
         * @brief Clear all elements
         */
        constexpr void clear() noexcept;

        /**
         * @brief Insert element at position
         *
         * @param pos Iterator to insert before
         * @param value Value to insert
         * @return Iterator to inserted element
         */
        constexpr iterator insert(const_iterator pos, const T& value);

        /**
         * @brief Insert element at position (move)
         *
         * @param pos Iterator to insert before
         * @param value Value to insert
         * @return Iterator to inserted element
         */
        constexpr iterator insert(const_iterator pos, T&& value);

        /**
         * @brief Insert count copies of value at position
         *
         * @param pos Iterator to insert before
         * @param count Number of elements to insert
         * @param value Value to insert
         * @return Iterator to first inserted element
         */
        constexpr iterator insert(const_iterator pos, size_type count, const T& value);

        /**
         * @brief Insert range at position
         *
         * @tparam InputIt Iterator type
         * @param pos Iterator to insert before
         * @param first Beginning of range
         * @param last End of range
         * @return Iterator to first inserted element
         */
        template<std::input_iterator InputIt>
        constexpr iterator insert(const_iterator pos, InputIt first, InputIt last);

        /**
         * @brief Insert initializer list at position
         *
         * @param pos Iterator to insert before
         * @param init Initializer list
         * @return Iterator to first inserted element
         */
        constexpr iterator insert(const_iterator pos, std::initializer_list<T> init);

        /**
         * @brief Construct element in-place at position
         *
         * @tparam Args Argument types
         * @param pos Iterator to insert before
         * @param args Arguments to forward to constructor
         * @return Iterator to inserted element
         */
        template<typename... Args>
        constexpr iterator emplace(const_iterator pos, Args&&... args);

        /**
         * @brief Erase element at position
         *
         * @param pos Iterator to element to erase
         * @return Iterator following the erased element
         */
        constexpr iterator erase(const_iterator pos);

        /**
         * @brief Erase range of elements
         *
         * @param first Beginning of range to erase
         * @param last End of range to erase
         * @return Iterator following the last erased element
         */
        constexpr iterator erase(const_iterator first, const_iterator last);

        /**
         * @brief Add element to end
         *
         * @param value Value to add
         */
        constexpr void push_back(const T& value);

        /**
         * @brief Add element to end (move)
         *
         * @param value Value to add
         */
        constexpr void push_back(T&& value);

        /**
         * @brief Construct element in-place at end
         *
         * @tparam Args Argument types
         * @param args Arguments to forward to constructor
         * @return Reference to the new element
         */
        template<typename... Args>
        constexpr reference emplace_back(Args&&... args);

        /**
         * @brief Remove last element
         */
        constexpr void pop_back() noexcept;

        /**
         * @brief Resize to contain count elements
         *
         * @param count New size
         */
        constexpr void resize(size_type count);

        /**
         * @brief Resize to contain count elements with value
         *
         * @param count New size
         * @param value Value for new elements
         */
        constexpr void resize(size_type count, const T& value);

        /**
         * @brief Swap contents with another vector
         *
         * @param other Vector to swap with
         */
        constexpr void swap(Vec& other) noexcept;

        /**
         * @brief Three-way comparison operator
         * @param other The vector to compare with
         * @return a lexicographical comparison result
         */
        constexpr auto operator<=>(const Vec& other) const requires std::three_way_comparable<T>
        {
            return std::lexicographical_compare_three_way(begin(), end(), other.begin(), other.end());
        }

        /**
         * @brief Equality comparison operator
         * @param other The vector to compare with
         * @return true if vectors are equal, false otherwise
         */
        constexpr bool operator==(const Vec& other) const requires std::equality_comparable<T>
        {
            return size() == other.size() && std::equal(begin(), end(), other.begin());
        }

    private:

        /**
         * @brief Internal storage structure
         *
         * Uses a union-like approach to store either inline data or heap data.
         * The small buffer is always present, but we track whether we're using it
         * via the data_ pointer and capacity_.
         */
        struct Storage
        {
            alignas(T) std::byte small_buffer_[sizeof(T) * SmallSize];
            pointer data_;
            size_type size_;
            size_type capacity_;

            /**
             * @brief Default constructor initializes to use small buffer
             */
            constexpr Storage() noexcept
                    : data_(reinterpret_cast<pointer>(small_buffer_))
                    , size_(0)
                    , capacity_(SmallSize)
            {
            }
        };

        Storage storage_;

        /**
         * @brief Get pointer to small buffer storage
         */
        constexpr pointer small_buffer_ptr() noexcept
        {
            return reinterpret_cast<pointer>(storage_.small_buffer_);
        }

        /**
         * @brief Get const pointer to small buffer storage
         */
        constexpr const_pointer small_buffer_ptr() const noexcept
        {
            return reinterpret_cast<const_pointer>(storage_.small_buffer_);
        }

        /**
         * @brief Calculate growth for new capacity
         */
        [[nodiscard]] constexpr size_type calculate_growth(size_type min_capacity) const noexcept
        {
            const size_type current = capacity();
            const size_type max = max_size();

            if (current > max - current / 2)
            {
                return max;
            }

            const size_type geometric = current + (current * GrowthFactor) / 10;

            if (geometric < min_capacity)
            {
                return min_capacity;
            }

            return geometric;
        }

        /**
         * @brief Allocate new storage
         */
        constexpr pointer allocate(size_type n)
        {
            if (n == 0)
            {
                return nullptr;
            }

            return std::allocator<T>().allocate(n);
        }

        /**
         * @brief Deallocate storage
         */
        constexpr void deallocate(pointer p, size_type n) noexcept
        {
            if (p && !is_small_ptr(p))
            {
                std::allocator<T>().deallocate(p, n);
            }
        }

        /**
         * @brief Check if pointer points to small buffer
         */
        constexpr bool is_small_ptr(const_pointer p) const noexcept
        {
            return p == small_buffer_ptr();
        }

        /**
         * @brief Destroy range of elements
         */
        constexpr void destroy_range(pointer first, pointer last) noexcept
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                for (; first != last; ++first)
                {
                    std::destroy_at(first);
                }
            }
        }

        /**
         * @brief Uninitialized copy with optimization for trivial types
         */
        constexpr void uninitialized_copy_impl(const_pointer first, const_pointer last, pointer dest)
        {
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                if (!std::is_constant_evaluated())
                {
                    std::memcpy(dest, first, (last - first) * sizeof(T));
                    return;
                }
            }
            std::uninitialized_copy(first, last, dest);
        }

        /**
         * @brief Uninitialized move with optimization for trivial types
         */
        constexpr void uninitialized_move_impl(pointer first, pointer last, pointer dest)
        {
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                if (!std::is_constant_evaluated())
                {
                    std::memmove(dest, first, (last - first) * sizeof(T));
                    return;
                }
            }
            std::uninitialized_move(first, last, dest);
        }

        /**
         * @brief Reallocate storage and move elements
         */
        constexpr void reallocate(size_type new_capacity)
        {
            pointer new_data = (new_capacity <= SmallSize) ? small_buffer_ptr() : allocate(new_capacity);

            try
            {
                uninitialized_move_impl(storage_.data_, storage_.data_ + storage_.size_, new_data);
            }
            catch (...)
            {
                deallocate(new_data, new_capacity);
                throw;
            }

            destroy_range(storage_.data_, storage_.data_ + storage_.size_);
            deallocate(storage_.data_, storage_.capacity_);

            storage_.data_ = new_data;
            storage_.capacity_ = new_capacity;
        }

        /**
         * @brief Ensure capacity for at least n elements
         */
        constexpr void ensure_capacity(size_type n)
        {
            if (n > capacity())
            {
                reallocate(calculate_growth(n));
            }
        }

        /**
         * @brief Insert gap at position
         */
        constexpr pointer make_gap(const_iterator pos, size_type count)
        {
            const size_type offset = pos - begin();
            const size_type old_size = size();

            ensure_capacity(old_size + count);

            pointer p = storage_.data_ + offset;
            pointer old_end = storage_.data_ + old_size;

            if (p != old_end)
            {
                if constexpr (std::is_trivially_copyable_v<T>)
                {
                    if (!std::is_constant_evaluated())
                    {
                        std::memmove(p + count, p, (old_end - p) * sizeof(T));
                        storage_.size_ += count;
                        return p;
                    }
                }

                pointer new_end = old_end + count;
                pointer move_src = old_end;
                pointer move_dst = new_end;

                const size_type elems_after = old_end - p;
                const size_type elems_to_construct = std::min(count, elems_after);
                const size_type elems_to_move = elems_after - elems_to_construct;

                move_src -= elems_to_construct;
                move_dst -= elems_to_construct;
                std::uninitialized_move(move_src, old_end, move_dst);

                if (elems_to_move > 0)
                {
                    move_src = p;
                    move_dst = p + count;
                    std::move_backward(move_src, move_src + elems_to_move, move_dst + elems_to_move);
                }
            }

            storage_.size_ += count;
            return p;
        }
    };

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr Vec<T, SmallSize, GrowthFactor>::Vec() noexcept
            : storage_()
    {
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr Vec<T, SmallSize, GrowthFactor>::Vec(size_type count)
            : storage_()
    {
        resize(count);
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr Vec<T, SmallSize, GrowthFactor>::Vec(size_type count, const T& value)
            : storage_()
    {
        assign(count, value);
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    template<std::input_iterator InputIt>
    constexpr Vec<T, SmallSize, GrowthFactor>::Vec(InputIt first, InputIt last)
            : storage_()
    {
        assign(first, last);
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr Vec<T, SmallSize, GrowthFactor>::Vec(const Vec& other)
            : storage_()
    {
        reserve(other.size());
        uninitialized_copy_impl(other.begin(), other.end(), storage_.data_);
        storage_.size_ = other.size();
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr Vec<T, SmallSize, GrowthFactor>::Vec(Vec&& other) noexcept
            : storage_()
    {
        if (other.is_small())
        {
            // Move elements from small buffer
            uninitialized_move_impl(other.begin(), other.end(), storage_.data_);
            storage_.size_ = other.size();
            other.clear();
        }
        else
        {
            // Steal heap allocation
            storage_.data_ = other.storage_.data_;
            storage_.size_ = other.storage_.size_;
            storage_.capacity_ = other.storage_.capacity_;

            other.storage_.data_ = other.small_buffer_ptr();
            other.storage_.size_ = 0;
            other.storage_.capacity_ = SmallSize;
        }
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr Vec<T, SmallSize, GrowthFactor>::Vec(std::initializer_list<T> init)
            : storage_()
    {
        assign(init);
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr Vec<T, SmallSize, GrowthFactor>::~Vec()
    {
        clear();
        deallocate(storage_.data_, storage_.capacity_);
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr Vec<T, SmallSize, GrowthFactor>&
    Vec<T, SmallSize, GrowthFactor>::operator=(const Vec& other)
    {
        if (this != &other)
        {
            assign(other.begin(), other.end());
        }
        return *this;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr Vec<T, SmallSize, GrowthFactor>&
    Vec<T, SmallSize, GrowthFactor>::operator=(Vec&& other) noexcept
    {
        if (this != &other)
        {
            clear();
            deallocate(storage_.data_, storage_.capacity_);

            if (other.is_small())
            {
                storage_.data_ = small_buffer_ptr();
                storage_.capacity_ = SmallSize;
                uninitialized_move_impl(other.begin(), other.end(), storage_.data_);
                storage_.size_ = other.size();
                other.clear();
            }
            else
            {
                storage_.data_ = other.storage_.data_;
                storage_.size_ = other.storage_.size_;
                storage_.capacity_ = other.storage_.capacity_;

                other.storage_.data_ = other.small_buffer_ptr();
                other.storage_.size_ = 0;
                other.storage_.capacity_ = SmallSize;
            }
        }
        return *this;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr Vec<T, SmallSize, GrowthFactor>&
    Vec<T, SmallSize, GrowthFactor>::operator=(std::initializer_list<T> init)
    {
        assign(init);
        return *this;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr void Vec<T, SmallSize, GrowthFactor>::assign(size_type count, const T& value)
    {
        clear();
        reserve(count);
        std::uninitialized_fill_n(storage_.data_, count, value);
        storage_.size_ = count;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    template<std::input_iterator InputIt>
    constexpr void Vec<T, SmallSize, GrowthFactor>::assign(InputIt first, InputIt last)
    {
        clear();

        if constexpr (std::random_access_iterator<InputIt>)
        {
            const size_type count = std::distance(first, last);
            reserve(count);
            std::uninitialized_copy(first, last, storage_.data_);
            storage_.size_ = count;
        }
        else
        {
            for (; first != last; ++first)
            {
                push_back(*first);
            }
        }
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr void Vec<T, SmallSize, GrowthFactor>::assign(std::initializer_list<T> init)
    {
        assign(init.begin(), init.end());
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::reference
    Vec<T, SmallSize, GrowthFactor>::at(size_type pos)
    {
        if (pos >= size())
        {
            throw std::out_of_range("mex::Vec::at");
        }
        return storage_.data_[pos];
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::const_reference
    Vec<T, SmallSize, GrowthFactor>::at(size_type pos) const
    {
        if (pos >= size())
        {
            throw std::out_of_range("mex::Vec::at");
        }
        return storage_.data_[pos];
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::reference
    Vec<T, SmallSize, GrowthFactor>::operator[](size_type pos) noexcept
    {
        return storage_.data_[pos];
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::const_reference
    Vec<T, SmallSize, GrowthFactor>::operator[](size_type pos) const noexcept
    {
        return storage_.data_[pos];
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::reference
    Vec<T, SmallSize, GrowthFactor>::front() noexcept
    {
        return storage_.data_[0];
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::const_reference
    Vec<T, SmallSize, GrowthFactor>::front() const noexcept
    {
        return storage_.data_[0];
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::reference
    Vec<T, SmallSize, GrowthFactor>::back() noexcept
    {
        return storage_.data_[storage_.size_ - 1];
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::const_reference
    Vec<T, SmallSize, GrowthFactor>::back() const noexcept
    {
        return storage_.data_[storage_.size_ - 1];
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::pointer
    Vec<T, SmallSize, GrowthFactor>::data() noexcept
    {
        return storage_.data_;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::const_pointer
    Vec<T, SmallSize, GrowthFactor>::data() const noexcept
    {
        return storage_.data_;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::iterator
    Vec<T, SmallSize, GrowthFactor>::begin() noexcept
    {
        return storage_.data_;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::const_iterator
    Vec<T, SmallSize, GrowthFactor>::begin() const noexcept
    {
        return storage_.data_;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::const_iterator
    Vec<T, SmallSize, GrowthFactor>::cbegin() const noexcept
    {
        return storage_.data_;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::iterator
    Vec<T, SmallSize, GrowthFactor>::end() noexcept
    {
        return storage_.data_ + storage_.size_;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::const_iterator
    Vec<T, SmallSize, GrowthFactor>::end() const noexcept
    {
        return storage_.data_ + storage_.size_;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::const_iterator
    Vec<T, SmallSize, GrowthFactor>::cend() const noexcept
    {
        return storage_.data_ + storage_.size_;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::reverse_iterator
    Vec<T, SmallSize, GrowthFactor>::rbegin() noexcept
    {
        return reverse_iterator(end());
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::const_reverse_iterator
    Vec<T, SmallSize, GrowthFactor>::rbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::const_reverse_iterator
    Vec<T, SmallSize, GrowthFactor>::crbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::reverse_iterator
    Vec<T, SmallSize, GrowthFactor>::rend() noexcept
    {
        return reverse_iterator(begin());
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::const_reverse_iterator
    Vec<T, SmallSize, GrowthFactor>::rend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::const_reverse_iterator
    Vec<T, SmallSize, GrowthFactor>::crend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr bool Vec<T, SmallSize, GrowthFactor>::empty() const noexcept
    {
        return storage_.size_ == 0;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::size_type
    Vec<T, SmallSize, GrowthFactor>::size() const noexcept
    {
        return storage_.size_;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::size_type
    Vec<T, SmallSize, GrowthFactor>::max_size() const noexcept
    {
        return std::allocator_traits<std::allocator<T>>::max_size(std::allocator<T>{});
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr void Vec<T, SmallSize, GrowthFactor>::reserve(size_type new_cap)
    {
        if (new_cap > capacity())
        {
            reallocate(new_cap);
        }
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::size_type
    Vec<T, SmallSize, GrowthFactor>::capacity() const noexcept
    {
        return storage_.capacity_;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr void Vec<T, SmallSize, GrowthFactor>::shrink_to_fit()
    {
        if (storage_.size_ < storage_.capacity_)
        {
            if (storage_.size_ <= SmallSize && !is_small())
            {
                pointer old_data = storage_.data_;
                size_type old_capacity = storage_.capacity_;

                storage_.data_ = small_buffer_ptr();
                storage_.capacity_ = SmallSize;

                uninitialized_move_impl(old_data, old_data + storage_.size_, storage_.data_);

                destroy_range(old_data, old_data + storage_.size_);
                deallocate(old_data, old_capacity);
            }
            else if (storage_.size_ > SmallSize)
            {
                reallocate(storage_.size_);
            }
        }
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr bool Vec<T, SmallSize, GrowthFactor>::is_small() const noexcept
    {
        return is_small_ptr(storage_.data_);
    }

// Modifiers

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr void Vec<T, SmallSize, GrowthFactor>::clear() noexcept
    {
        destroy_range(storage_.data_, storage_.data_ + storage_.size_);
        storage_.size_ = 0;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::iterator
    Vec<T, SmallSize, GrowthFactor>::insert(const_iterator pos, const T& value)
    {
        pointer p = make_gap(pos, 1);
        std::construct_at(p, value);
        return p;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::iterator
    Vec<T, SmallSize, GrowthFactor>::insert(const_iterator pos, T&& value)
    {
        pointer p = make_gap(pos, 1);
        std::construct_at(p, std::move(value));
        return p;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::iterator
    Vec<T, SmallSize, GrowthFactor>::insert(const_iterator pos, size_type count, const T& value)
    {
        if (count == 0)
        {
            return begin() + (pos - begin());
        }

        pointer p = make_gap(pos, count);
        std::uninitialized_fill_n(p, count, value);
        return p;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    template<std::input_iterator InputIt>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::iterator
    Vec<T, SmallSize, GrowthFactor>::insert(const_iterator pos, InputIt first, InputIt last)
    {
        const size_type offset = pos - begin();

        if constexpr (std::random_access_iterator<InputIt>)
        {
            const size_type count = std::distance(first, last);
            if (count == 0)
            {
                return begin() + offset;
            }

            pointer p = make_gap(pos, count);
            std::uninitialized_copy(first, last, p);
            return p;
        }
        else
        {
            Vec temp(first, last);
            return insert(begin() + offset, temp.begin(), temp.end());
        }
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::iterator
    Vec<T, SmallSize, GrowthFactor>::insert(const_iterator pos, std::initializer_list<T> init)
    {
        return insert(pos, init.begin(), init.end());
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    template<typename... Args>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::iterator
    Vec<T, SmallSize, GrowthFactor>::emplace(const_iterator pos, Args&&... args)
    {
        pointer p = make_gap(pos, 1);
        std::construct_at(p, std::forward<Args>(args)...);
        return p;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::iterator
    Vec<T, SmallSize, GrowthFactor>::erase(const_iterator pos)
    {
        return erase(pos, pos + 1);
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::iterator
    Vec<T, SmallSize, GrowthFactor>::erase(const_iterator first, const_iterator last)
    {
        const size_type offset = first - begin();
        const size_type count = last - first;

        if (count == 0)
        {
            return begin() + offset;
        }

        pointer p = storage_.data_ + offset;
        pointer range_end = p + count;
        pointer vec_end = storage_.data_ + storage_.size_;

        destroy_range(p, range_end);

        if (range_end != vec_end)
        {
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                if (!std::is_constant_evaluated())
                {
                    std::memmove(p, range_end, (vec_end - range_end) * sizeof(T));
                }
                else
                {
                    std::move(range_end, vec_end, p);
                }
            }
            else
            {
                std::move(range_end, vec_end, p);
            }
        }

        storage_.size_ -= count;
        return p;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr void Vec<T, SmallSize, GrowthFactor>::push_back(const T& value)
    {
        emplace_back(value);
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr void Vec<T, SmallSize, GrowthFactor>::push_back(T&& value)
    {
        emplace_back(std::move(value));
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    template<typename... Args>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::reference
    Vec<T, SmallSize, GrowthFactor>::emplace_back(Args&&... args)
    {
        if (storage_.size_ == storage_.capacity_)
        {
            ensure_capacity(storage_.size_ + 1);
        }

        pointer p = storage_.data_ + storage_.size_;
        std::construct_at(p, std::forward<Args>(args)...);
        ++storage_.size_;

        return *p;
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr void Vec<T, SmallSize, GrowthFactor>::pop_back() noexcept
    {
        --storage_.size_;
        std::destroy_at(storage_.data_ + storage_.size_);
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr void Vec<T, SmallSize, GrowthFactor>::resize(size_type count)
    {
        if (count < storage_.size_)
        {
            destroy_range(storage_.data_ + count, storage_.data_ + storage_.size_);
            storage_.size_ = count;
        }
        else if (count > storage_.size_)
        {
            reserve(count);
            std::uninitialized_value_construct(storage_.data_ + storage_.size_, storage_.data_ + count);
            storage_.size_ = count;
        }
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr void Vec<T, SmallSize, GrowthFactor>::resize(size_type count, const T& value)
    {
        if (count < storage_.size_)
        {
            destroy_range(storage_.data_ + count, storage_.data_ + storage_.size_);
            storage_.size_ = count;
        }
        else if (count > storage_.size_)
        {
            reserve(count);
            std::uninitialized_fill(storage_.data_ + storage_.size_,
                                    storage_.data_ + count, value);
            storage_.size_ = count;
        }
    }

    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr void Vec<T, SmallSize, GrowthFactor>::swap(Vec& other) noexcept
    {
        if (is_small() && other.is_small())
        {
            const size_type min_size = std::min(storage_.size_, other.storage_.size_);
            const size_type max_size = std::max(storage_.size_, other.storage_.size_);

            std::swap_ranges(storage_.data_, storage_.data_ + min_size, other.storage_.data_);

            if (storage_.size_ < other.storage_.size_)
            {
                uninitialized_move_impl(other.storage_.data_ + min_size, other.storage_.data_ + max_size, storage_.data_ + min_size);
                destroy_range(other.storage_.data_ + min_size, other.storage_.data_ + max_size);
            }
            else if (storage_.size_ > other.storage_.size_)
            {
                uninitialized_move_impl(storage_.data_ + min_size, storage_.data_ + max_size, other.storage_.data_ + min_size);
                destroy_range(storage_.data_ + min_size, storage_.data_ + max_size);
            }

            std::swap(storage_.size_, other.storage_.size_);
        }
        else if (!is_small() && !other.is_small())
        {
            std::swap(storage_.data_, other.storage_.data_);
            std::swap(storage_.size_, other.storage_.size_);
            std::swap(storage_.capacity_, other.storage_.capacity_);
        }
        else
        {
            Vec temp(std::move(*this));
            *this = std::move(other);
            other = std::move(temp);
        }
    }

    /**
     * @brief Swap two vectors
     *
     * @tparam T Element type
     * @tparam SmallSize Small buffer size
     * @tparam GrowthFactor Growth factor
     * @param lhs First vector
     * @param rhs Second vector
     */
    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr void swap(Vec<T, SmallSize, GrowthFactor>& lhs,
                        Vec<T, SmallSize, GrowthFactor>& rhs) noexcept
    {
        lhs.swap(rhs);
    }

    /**
     * @brief Erase elements equal to value
     *
     * @tparam T Element type
     * @tparam SmallSize Small buffer size
     * @tparam GrowthFactor Growth factor
     * @param vec Vector to erase from
     * @param value Value to erase
     * @return Number of elements erased
     */
    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::size_type
    erase(Vec<T, SmallSize, GrowthFactor>& vec, const T& value)
    {
        auto it = std::remove(vec.begin(), vec.end(), value);
        auto count = std::distance(it, vec.end());
        vec.erase(it, vec.end());
        return count;
    }

    /**
     * @brief Erase elements satisfying predicate
     *
     * @tparam T Element type
     * @tparam SmallSize Small buffer size
     * @tparam GrowthFactor Growth factor
     * @tparam Pred Predicate type
     * @param vec Vector to erase from
     * @param pred Predicate
     * @return Number of elements erased
     */
    template<typename T, std::size_t SmallSize, std::size_t GrowthFactor, typename Pred>
    constexpr typename Vec<T, SmallSize, GrowthFactor>::size_type
    erase_if(Vec<T, SmallSize, GrowthFactor>& vec, Pred pred)
    {
        auto it = std::remove_if(vec.begin(), vec.end(), pred);
        auto count = std::distance(it, vec.end());
        vec.erase(it, vec.end());
        return count;
    }

} // namespace mex

// Range support
template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
inline constexpr bool std::ranges::enable_borrowed_range<mex::Vec<T, SmallSize, GrowthFactor>> = false;

template<typename T, std::size_t SmallSize, std::size_t GrowthFactor>
inline constexpr bool std::ranges::enable_view<mex::Vec<T, SmallSize, GrowthFactor>> = false;