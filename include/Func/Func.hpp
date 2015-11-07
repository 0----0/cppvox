#include <iterator>
template<typename IterT, typename F>
class FilterIterRange {
private:
        IterT start;
        IterT stop;
        F f;

        class FilterIter {
        private:
                FilterIterRange const& parent;
                IterT curr;

                using traits = std::iterator_traits<IterT>;
        public:
                using difference_type = typename traits::difference_type;
                using value_type = typename traits::value_type;
                using pointer = typename traits::pointer;
                using reference = typename traits::reference;
                using iterator_category	 = typename std::forward_iterator_tag;

                FilterIter(FilterIterRange& parent, IterT curr):
                        parent(parent), curr(curr) {}
                void operator++() {
                        do {
                                ++curr;
                        }
                        while (curr != parent.stop && !parent.f(*curr));
                }
                auto operator*() {
                        return *curr;
                }
                bool operator!=(FilterIter& rhs) {
                        return curr != rhs.curr;
                }
                bool operator==(FilterIter& rhs) {
                        return curr == rhs.curr;
                }
        };
public:
        FilterIterRange(F f, IterT _start, IterT _stop):
                start(_start), stop(_stop), f(f) {
                while (start != stop && !f(*start)) { ++start; }
        }
        FilterIter begin() {
                return FilterIter{*this, start};
        }
        FilterIter end() {
                return FilterIter{*this, stop};
        }
};

template<typename IterT, typename F>
FilterIterRange<IterT, F> filter(F f, IterT start, IterT stop) {
        return FilterIterRange<IterT, F>{f, start, stop};
}

template<typename RangeT, typename F>
auto filter(F f, RangeT range) {
        using IterT = decltype(range.begin());
        return FilterIterRange<IterT, F>{f, range.begin(), range.end()};
}

template<typename T>
class NumIter : public std::iterator<std::random_access_iterator_tag, T> {
private:
        T value;
        using traits = std::iterator_traits<NumIter>;
public:
        NumIter(T value): value(value) {}
        T operator*() {
                return value;
        }
        void operator++() {
                ++value;
        }
        void operator--() {
                --value;
        }
        bool operator==(const NumIter& rhs) const {
                return value == rhs.value;
        }
        bool operator!=(const NumIter& rhs) const {
                return value != rhs.value;
        }
        NumIter operator+(typename traits::difference_type diff) const {
                return NumIter{value+diff};
        }
        NumIter operator-(typename traits::difference_type diff) const {
                return NumIter{value-diff};
        }
        NumIter& operator+=(typename traits::difference_type diff) {
                value += diff;
                return *this;
        }
        NumIter& operator-=(typename traits::difference_type diff) {
                value -= diff;
                return *this;
        }
};

template<typename T>
class RangeIterRange {
private:
        T start;
        T stop;
public:
        RangeIterRange(T start, T stop): start(start), stop(stop) {}
        auto begin() { return NumIter<T>{start}; }
        auto end() { return NumIter<T>{stop}; }
};

template<typename T>
RangeIterRange<T> range(T start, T stop) {
        return RangeIterRange<T>{start, stop};
}
