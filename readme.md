## 专题优化 - 频繁申请/释放内存

### 1. 开发一种开辟于栈空间的固定大小vector(fixed vector)

Note: 该方案风险太大，工作量也不小，暂时搁置。

重新实现一个fixed_vector，包含基本的stl vector的功能，参考本人之前的工作代码，一套成熟的渲染引擎中大量使用的fixed_vector<type, size>
附代码一览（见后）

### 2. 移植chromium中stack vector

重载std::allocator实现栈空间内存分配，在使用stl vector时候指定重新实现后的allocator(stack allocator)

1. 问题链接：https://stackoverflow.com/questions/354442/looking-for-c-stl-like-vector-class-but-using-stack-storage

2. 代码链接：https://chromium.googlesource.com/chromium/chromium/+/master/base/stack_container.h


附代码:

```c++
/**
 * A container which has some space allocated on the stack, thus not needing to
 * allocate memory unless an overflow occurs.
 */
template <typename T,
          size_t nodeCount,
          bool bEnableOverflow = true,
          typename OverflowAllocator = std::allocator<T> >
class fixed_vector {
 public:
    typedef OverflowAllocator allocator_type;
    typedef T value_type;
    typedef T* iterator;
    typedef const T* const_iterator;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;

    fixed_vector();
    explicit fixed_vector(const allocator_type& overflowAllocator);
    explicit fixed_vector(size_t n);
    fixed_vector(size_t n, const value_type& value);
    fixed_vector(const fixed_vector& other);
    template <typename InputIterator>
    fixed_vector(InputIterator first,
                 typename boost::disable_if<boost::is_integral<InputIterator>,
                                            InputIterator>::type last);
    ~fixed_vector();

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    const_iterator cbegin() const;
    const_iterator cend() const;

    reference back();
    const_reference back() const;
    pointer data();
    const_pointer data() const;
    size_t size() const;
    size_t capacity() const;
    bool empty() const;

    reference operator[](size_t i);
    const_reference operator[](size_t i) const;

    void resize(size_t n);
    void resize(size_t n, const T& value);
    ///@note  calls the default ctr
    reference push_back();
    void push_back(const T& value);
    ///@warning doesn't call the default ctr
    pointer push_back_uninitialized();
    template <typename A1>
    void emplace_back(const A1& a1);
    template <typename A1, typename A2>
    void emplace_back(const A1& a1, const A2& a2);
    template <typename A1, typename A2, typename A3>
    void emplace_back(const A1& a1, const A2& a2, const A3& a3);
    template <typename A1, typename A2, typename A3, typename A4>
    void emplace_back(const A1& a1, const A2& a2, const A3& a3, const A4& a4);

    iterator insert(const_iterator position, const value_type& value);
    template <typename InputIterator>
    void insert(const_iterator position,
                InputIterator first,
                InputIterator last);
    template <typename InputIterator>
    void append(InputIterator first, InputIterator last);

    void clear();
    void clear(bool freeOverflow);
    iterator erase(const_iterator position);
    iterator erase(const_iterator first, const_iterator last);
    // Same as erase, except it doesn't preserve order, but is faster because it
    // simply copies the last item in the vector over the erased position.
    iterator erase_unsorted(const_iterator position);
    // Erases from given position till the end.
    void erase_from(const_iterator position);

    fixed_vector& operator=(const fixed_vector& other);

 private:
    typedef
        typename tn::vector<T, OverflowAllocator>::iterator OverflowIterator;

    inline bool isOverflowed() const {
        return bEnableOverflow ? m_begin != m_data.begin() : false;
    }

    inline void reserve(size_t n) {
        ATLAS_ASSERT(!isOverflowed());
        ATLAS_ASSERT(n > nodeCount);

        m_overflow.get().reserve(n * 1.5);
        m_overflow.get().insert(m_overflow.get().end(), begin(), end());
        tn::detail::destruct(begin(), end());
        updateOverflow();
    }

    inline void updateOverflow() {
        m_begin = m_overflow.get().data();
        m_end = &m_overflow.get().back() + 1;
    }

    inline void updateOverflowEnd() {
        if (!m_overflow.get().empty())
            m_end = &m_overflow.get().back() + 1;
        else {
            m_end = m_overflow.get().data();
            ATLAS_ASSERT(m_begin == m_overflow.get().data());
        }
    }

    OverflowIterator from(const_iterator position) {
        return m_overflow.get().begin() + (position - begin());
    }

    iterator from(OverflowIterator position) {
        return begin() + (position - m_overflow.get().begin());
    }

 private:
    typedef tn::detail::OverflowVector<T, bEnableOverflow, OverflowAllocator>
        overflow_storage_type;
    typedef tn::detail::ArrayStorage<
        T,
        nodeCount,
        typename boost::has_trivial_destructor<T>::type>
        array_storage_type;

    array_storage_type m_data;
    T* m_begin;
    T* m_end;
    overflow_storage_type m_overflow;
};

```