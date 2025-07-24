template<typename T>
class Optional {
    Optional(T value) : value_(value), empty_(false) {}

    static Optional<T> Null() {
        return {T{}, true};
    }

    T Get() { return value_; }

    bool Empty() { return empty_; }

private:
    T value_;
    bool empty_;
};
