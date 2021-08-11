#pragma once

template<typename T>
class CircularIterator_Forward;

template<typename T>
class CircularIterator_Reverse;

template<typename T>
class SubBuffer;

template<typename T>
class SelectiveSubBuffer;

template <typename T>
class CircularBuffer {
public:
    friend class CircularIterator_Reverse<T>;
    friend class CircularIterator_Forward<T>;
    friend class SubBuffer<T>;
    friend class SelectiveSubBuffer<T>;

    explicit CircularBuffer(int bufferSize) :
        size(bufferSize),
        buffer(new T[bufferSize])
    {
        firstPointer = &buffer[0];
        lastPointer = &buffer[size - 1];
    }

    ~CircularBuffer() = default;

    inline void reset() {
        QMutexLocker locker(&mutex_lastIndex);
        lastIndex = 0;
        isFull = false;
    }

    inline bool isEmpty() {
        QMutexLocker locker(&mutex_lastIndex);
        return lastIndex == 0 && !isFull;
    }

    inline int count() {
        if (isFull)
            return size - 1; /// 1 element is not usable as it acts as the imaginary one.
        else {
            QMutexLocker locker(&mutex_lastIndex);
            return lastIndex;
        }
    }

    inline int putLast(const T& item) {
        QMutexLocker locker(&mutex_lastIndex);
        buffer[lastIndex] = item;
        int out = lastIndex;
        lastIndex++;
        if (lastIndex >= size) {
            lastIndex = 0;
            isFull = true;
        }
        return out;
    }

    /// Do not call on an empty buffer.
    inline int getLastValidIndex() {
        int lastIndex_copy;
        {
            QMutexLocker locker(&mutex_lastIndex);
            lastIndex_copy = lastIndex;
        }
        return prevIndex(lastIndex_copy);
    }

    /// Do not call on an empty buffer.
    inline T& getLastElement() {
        int lastValidIndex = getLastValidIndex();
        return buffer[lastValidIndex];
    }

    inline SubBuffer<T> subBuffer();
    inline SubBuffer<T> subBuffer(int start, int finish);
    inline SubBuffer<T> subBuffer(const std::pair<int, int>& indexes);
    inline SelectiveSubBuffer<T> subBuffer(const QVector<int>& indexes);

    inline T& operator[] (int index) { return buffer[index]; }
    inline const T& operator[] (int index) const { return buffer[index];}

    inline std::pair<int, int> range();
    inline std::pair<int, int> reverse_range();
    inline std::pair<CircularIterator_Forward<T>, CircularIterator_Forward<T>> iterators();
    inline std::pair<CircularIterator_Reverse<T>, CircularIterator_Reverse<T>> iterators_reverse();

private:
    inline int getIndex(T* item) {
        auto diff = item - buffer.get();
        if (diff >= 0 && diff < size) {
            return int(diff);
        }
        else {
            return -1;
        }
    }

    inline int count(int start, int finish) {
        if (finish >= start) {
            return finish - start;
        }
        else {
            return finish - start + size;
        }
    }

    inline int count(std::pair<int, int>& indexes) {
        return count(indexes.first, indexes.second);
    }

    inline int prevIndex(int index) const {
        index--;
        if (index < 0) {
            index = size - 1;
        }
        return index;
    }

    inline int nextIndex(int index) const {
        index++;
        if (index >= size)
            index = 0;
        return index;
    }

    inline T* next(T* p) {
        if (p < lastPointer) {
            return p + 1;
        }
        else {
            return firstPointer;
        }
    }

    inline T* prev(T* p) {
        if (p > firstPointer) {
            return p - 1;
        }
        else {
            return lastPointer;
        }
    }

    const int size = {};
    std::unique_ptr<T[]> buffer;
    T* lastPointer = nullptr;
    T* firstPointer = nullptr;
    int lastIndex = 0; /// the index to write the new data to (not the one that the last data was written to).
    bool isFull = false;
    QMutex mutex_lastIndex;
};

template<typename T>
class CircularIterator {
public:
    CircularIterator(CircularBuffer<T>* ref, T* pointer) :
        ref(ref),
        pointer(pointer)
    {}

    int getIndex() const { return ref->getIndex(pointer); }
    T* operator->() const { return pointer; }
    T& operator*() const { return *pointer; }
    friend bool operator==(const CircularIterator<T>& first, const CircularIterator<T>& second) {
        return first.pointer == second.pointer;
    }
    friend bool operator!=(const CircularIterator<T>& first, const CircularIterator<T>& second) {
        return first.pointer != second.pointer;
    }

protected:
    CircularBuffer<T>* ref;
    T* pointer;
};

template<typename T>
class CircularIterator_Forward : public CircularIterator<T> {
public:
    CircularIterator_Forward(CircularBuffer<T>* ref, T* pointer) :
        CircularIterator<T>(ref, pointer)
    {}
    using CircularIterator<T>::pointer;
    using CircularIterator<T>::ref;

    void operator++() { pointer = ref->next(pointer); }
    void operator++(int) { ++*this; }
    void operator--() { pointer = ref->prev(pointer); }
    void operator--(int) { --*this; }

    CircularIterator_Forward operator+(int c) const {
        CircularIterator_Forward out(*this);
        out += c;
        return out;
    }

    CircularIterator_Forward operator-(int c) const {
        CircularIterator_Forward out(*this);
        out -= c;
        return out;
    }

    void operator+=(int c) {
        pointer += c;
        while (pointer > ref->lastPointer)
            pointer -= ref->size;
    }

    void operator-=(int c) {
        pointer -= c;
        while (pointer < ref->firstPointer)
            pointer += ref->size;
    }
};

template<typename T>
class CircularIterator_Reverse : public CircularIterator<T> {
public:
    CircularIterator_Reverse(CircularBuffer<T>* ref, T* pointer) :
        CircularIterator<T>(ref, pointer)
    {}
    using CircularIterator<T>::pointer;
    using CircularIterator<T>::ref;

    void operator++() { pointer = ref->prev(pointer); }
    void operator++(int) { ++*this; }
    void operator--() { pointer = ref->next(pointer); }
    void operator--(int) { --*this; }

    CircularIterator_Reverse operator+(int c) const {
        CircularIterator_Reverse out(*this);
        out += c;
        return out;
    }

    CircularIterator_Reverse operator-(int c) const {
        CircularIterator_Reverse out(*this);
        out -= c;
        return out;
    }

    void operator+=(int c) {
        pointer -= c;
        while (pointer < ref->firstPointer)
            pointer += ref->size;
    }

    void operator-=(int c) {
        pointer += c;
        while (pointer > ref->lastPointer)
            pointer -= ref->size;
    }
};

template <typename T>
class SubBuffer {
public:
    explicit SubBuffer(CircularBuffer<T>* ref, const std::pair<int, int>& indexes) :
        ref(ref),
        indexes(indexes),
        beginIt(ref, &(*ref)[indexes.first]),
        endIt(ref, &(*ref)[indexes.second]),
        beginIt_reverse(ref, &(*ref)[ref->prevIndex(indexes.second)]),
        endIt_reverse(ref, &(*ref)[ref->prevIndex(indexes.first)])
    {}
    inline bool isEmpty() { return indexes.first != indexes.second; }
    inline int count() const {return ref->count(indexes);}
    inline CircularIterator_Forward<T> begin() { return beginIt; }
    inline const CircularIterator_Forward<T>& end() { return endIt; }
    inline CircularIterator_Reverse<T> rbegin() { return beginIt_reverse; }
    inline const CircularIterator_Reverse<T>& rend() { return endIt_reverse; }

    inline T& first() { return *beginIt; }
    inline T& last() { return *beginIt_reverse; }
    inline T& operator[](int round) { return *(beginIt + round); }

    inline std::pair<int, int> range() { return indexes; }

private:
    CircularBuffer<T>* ref;
    const std::pair<int, int> indexes;
    const CircularIterator_Forward<T> beginIt;
    const CircularIterator_Forward<T> endIt;
    const CircularIterator_Reverse<T> beginIt_reverse;
    const CircularIterator_Reverse<T> endIt_reverse;
};

template<typename T>
class SelectiveIterator {
public:
    SelectiveIterator(CircularBuffer<T>* ref, const int* indexesIt) :
        ref(ref),
        indexesIt(indexesIt)
    {}

    int getIndex() const { return *indexesIt; }
    T* operator->() const { return &(*ref)[*indexesIt]; }
    T& operator*() const { return (*ref)[*indexesIt]; }

    friend bool operator==(const SelectiveIterator<T>& first, const SelectiveIterator<T>& second) {
        return first.indexesIt == second.indexesIt;
    }
    friend bool operator!=(const SelectiveIterator<T>& first, const SelectiveIterator<T>& second) {
        return first.indexesIt != second.indexesIt;
    }

    void operator++() { ++indexesIt; }
    void operator++(int) { ++*this; }
    void operator--() { --indexesIt; }
    void operator--(int) { --*this; }
    void operator+=(int c) { indexesIt += c; }
    void operator-=(int c) { indexesIt -= c; }

    SelectiveIterator operator+(int c) const {
        SelectiveIterator out(*this);
        out += c;
        return out;
    }

    SelectiveIterator operator-(int c) const {
        SelectiveIterator out(*this);
        out -= c;
        return out;
    }

private:
    CircularBuffer<T>* ref;
    const int* indexesIt;
};

template<typename T>
class SelectiveSubBuffer {
public:
    explicit SelectiveSubBuffer(CircularBuffer<T>& ref, const QVector<int>& indexes) :
            ref(ref),
            indexes(indexes),
            endIt(&ref, indexes.end())
    {}
    inline const QVector<int>& getIndexes() const { return indexes; }
    inline bool isEmpty() const {return indexes.isEmpty();}
    inline int count() const  {return indexes.size();}
    inline T& first() {return ref[indexes.first()]; }
    inline T& last() {return ref[indexes.last()];}
    inline SelectiveIterator<T> begin() { return SelectiveIterator<T> (&ref, indexes.begin()); }
    inline const SelectiveIterator<T>& end() { return endIt;}
private:
    CircularBuffer<T>& ref;
    const QVector<int>& indexes;
    const SelectiveIterator<T> endIt;
};

template<typename T>
SubBuffer<T> CircularBuffer<T>::subBuffer()
{
    return subBuffer(range());
}

template<typename T>
SubBuffer<T> CircularBuffer<T>::subBuffer(int start, int finish)
{
    return subBuffer({start, finish});
}

template<typename T>
SubBuffer<T> CircularBuffer<T>::subBuffer(const std::pair<int, int> &indexes)
{
    return SubBuffer<T>(this, indexes);
}

template<typename T>
SelectiveSubBuffer<T> CircularBuffer<T>::subBuffer(const QVector<int> &indexes)
{
    return SelectiveSubBuffer<T>(*this, indexes);
}

template<typename T>
std::pair<int, int> CircularBuffer<T>::range()
{
    QMutexLocker locker(&mutex_lastIndex);
    int start = isFull ? nextIndex(lastIndex) : 0;
    int finish = lastIndex;
    return {start, finish};
}

template<typename T>
std::pair<int, int> CircularBuffer<T>::reverse_range()
{
    QMutexLocker locker(&mutex_lastIndex);
    int start = prevIndex(lastIndex);
    int finish = isFull ? lastIndex : size - 1;
    return {start, finish};
}

template<typename T>
std::pair<CircularIterator_Forward<T>, CircularIterator_Forward<T>> CircularBuffer<T>::iterators()
{
    auto r = range();
    CircularIterator_Forward<T> start(this, &buffer[r.first]);
    CircularIterator_Forward<T> finish(this, &buffer[r.second]);
    return {std::move(start), std::move(finish)};
}

template<typename T>
std::pair<CircularIterator_Reverse<T>, CircularIterator_Reverse<T>> CircularBuffer<T>::iterators_reverse()
{
    auto r = reverse_range();
    CircularIterator_Forward<T> start(this, &buffer[r.first]);
    CircularIterator_Forward<T> finish(this, &buffer[r.second]);
    return {std::move(start), std::move(finish)};
}
