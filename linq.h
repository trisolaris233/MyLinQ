#pragma once

#include <memory>
#include <vector>
#include <string>
#include <utility>
#include <typeinfo>
#include <iostream>
#include <exception>
#include <initializer_list>

namespace trisolaris {

    class linq_exception : public std::exception {
    public:
        linq_exception() = default;
        linq_exception(const linq_exception&) = default;
        linq_exception(const std::string& str) : message(str) {}
        linq_exception(const char* str) : message(str) {}

        std::string         message;

        virtual const char* what() noexcept {
            return message.c_str();
        }
    };

    
    template < typename T >
    class forward_iterator {
    private:
        // forward_iterator要实现的接口
        class forward_iterator_interface {
            typedef forward_iterator_interface TSelf;
        public:
            virtual std::shared_ptr<TSelf>  advance() = 0;
            virtual T                       deref() = 0;
            virtual bool                    equal(const std::shared_ptr<TSelf>& rhand) = 0;
        };

        template < typename TIterator >
        class forward_iterator_implement : public forward_iterator_interface {
            typedef forward_iterator_implement<TIterator> TSelf;
        private:
            TIterator       _iterator; 
        public:
            forward_iterator_implement(const TIterator& _iterator_): _iterator(_iterator_) {

            }

            std::shared_ptr<forward_iterator_interface> advance() override {
                auto t = _iterator;
                t++;
                // std::make_shared(t);
                return std::make_shared<TSelf>(t);
            }

            T deref() override {
                return *_iterator;
            }

            virtual bool equal(const std::shared_ptr<forward_iterator_interface>& rhand) override {
                auto impl = std::dynamic_pointer_cast<TSelf>(rhand);
                return impl && (impl->_iterator == _iterator);
            }
        };

        typedef forward_iterator<T>         TSelf;

        std::shared_ptr<forward_iterator_interface>     _iterator;
    public:
        template < typename TIterator>
        forward_iterator(const TIterator& _iterator_) : _iterator(std::make_shared<forward_iterator_implement<TIterator>>(_iterator_)){
        }
        
        // 前缀++
        TSelf& operator++() {
            _iterator = _iterator->advance();
            return *this;
        }   

        // 后缀++
        TSelf operator++(int) {
            TSelf tmp = *this;
            _iterator = _iterator->advance();
            return tmp;
        }

        T operator*() const {
            // std::cout << typeid((*_iterator).deref()).name() << std::endl;
            // return T(0);
            return (*_iterator).deref();
        }

        bool operator==(const TSelf& rhand) const {
            return _iterator->equal(rhand._iterator);
        }

        bool operator!=(const TSelf& rhand) const {
            return !(this->operator==(rhand));
        }
    };


    template < typename T >
    class storage_iterator {
        typedef storage_iterator<T>         TSelf;
    private:
        std::shared_ptr<std::vector<T>>     _values;
        typename std::vector<T>::iterator   _iterator;

    public:
        storage_iterator(const std::shared_ptr<std::vector<T>>& _values_, const typename std::vector<T>::iterator& _iterator_) :
            _values(_values_), _iterator(_iterator_) {
        }

        TSelf& operator++() {
            _iterator++;
            return *this;
        }

        TSelf operator++(int) {
            auto tmp = *this;
            _iterator++;
            return tmp;
        }

        T operator*() const {
            return *_iterator;
        }

        bool operator==(const TSelf& rhand) const {
            return (_iterator == rhand._iterator);
        }

        bool operator!=(const TSelf& rhand) const {
            return _iterator != rhand._iterator;
        }
    };

    template < typename T >
    class empty_iterator {
        typedef empty_iterator<T>       TSelf;
    private:
    public:
        empty_iterator() {}

        TSelf& operator++() {
            return *this;
        }

        TSelf operator++(int) {
            return *this;
        }

        T operator*() const {
            throw linq_exception("Failed to get value from an empty iterator.");
        }

        bool operator==(const TSelf&) const {
            return true;
        }

        bool operator!=(const TSelf& it) const {
            return false;
        }
    };

    template < typename TIterator, typename TFunction >
    class select_iterator {
        typedef select_iterator<TIterator, TFunction>       TSelf;
    private:
        TIterator       _iterator;
        TFunction       _f;

    public:
        select_iterator(const TIterator& _iterator_, const TFunction& _f_) :
            _iterator(_iterator_), _f(_f_) {

        }

        TSelf& operator++() {
            _iterator++;
            return *this;
        }

        TSelf operator++(int) {
            auto tmp = *this;
            _iterator++;
            return tmp;
        }

        auto operator*() const -> decltype(_f(*_iterator)) {
            return _f(*_iterator);
        }

        bool operator==(const TSelf& rhand) const {
            return _iterator == rhand._iterator;
        }

        bool operator!=(const TSelf& rhand) const {
            return _iterator != rhand._iterator;
        }
    };

    template < typename TIterator, typename TFunction >
    class where_iterator {
        typedef where_iterator<TIterator, TFunction>       TSelf;
    private:
        TIterator   _iterator;
        TIterator   _end;
        TFunction   _f;

        // advance方法, 直到_f(*_iterator)为true
        inline void advance(bool next) {
            if(next) _iterator++;
            for(; _iterator != _end && !_f(*_iterator); ++_iterator);
        }

    public:
        where_iterator(const TIterator& _iterator_, const TIterator& _end_, const TFunction& _f_) :
            _iterator(_iterator_),
            _end(_end_),
            _f(_f_) {
            advance(false);
        }

        TSelf& operator++() {
            advance(true);
            return *this;
        }

        TSelf operator++(int) {
            auto tmp = *this;
            advance(true);
            return tmp;
        }

        auto operator*() const -> decltype(*_iterator) {
            return *_iterator;
        }

        bool operator==(const TSelf& rhand) const {
            return _iterator == rhand._iterator;
        }

        bool operator!=(const TSelf& rhand) const {
            return _iterator != rhand._iterator;
        }
    };

    // iteratorable包装一对iterators(begin & end)
    // 根据包装不同的iterator有不同的效果
    // 利用范围for循环使用
    template < typename TIterator >
    class iteratorable {
        typedef typename std::remove_cv<typename std::remove_reference<TIterator>::type>::type  TElement;
        typedef iteratorable<TIterator>     TSelf;
    private:
        TIterator       _begin;
        TIterator       _end;

    public:
        // 增加STL支持， 除了可以传入iteratorable对象外， 还可以传入STL容器(第二个模板函数中的TContainer), 亦或是
        // 传入一个列表{}(会调用第三个)。
#define SUPPORT_STL_CONTAINERS(NAME)\
        template < typename TIterator2 >\
        auto NAME(const iteratorable<TIterator2>& e) const -> decltype(this->NAME ## _(e)) {\
            return NAME ## _(e);\
        }\
        template < typename TContainer >\
        auto NAME(const TContainer& container) const -> decltype(this->NAME ## _(from(container))) {\
            return NAME ## _(from(container));\
        }\
        template < typename TElement >\
        auto NAME(const std::initializer_list<TElement>& init_list) -> decltype(this->NAME ## _(from(init_list))) {\
            return NAME ## _(from(init_list));\
        }\


        iteratorable() = default;
        iteratorable(const TIterator& _begin_, const TIterator& _end_):
            _begin(_begin_), _end(_end_) {}

        TIterator begin() const {
            return _begin;
        }

        TIterator end() const {
            return _end;
        }

        template < typename T >
        bool contains(const T& element) const {
            for(auto itr = _begin; itr != _end; ++itr) 
                if(*itr == element) return true;
            return false;
        }

        std::size_t count() const {
            std::size_t num = {0};
            for(auto itr = _begin; itr!= _end; ++itr) ++num;
            return num;
        }

        TElement first() const {
            if(empty()) throw linq_exception("failed to get the first element from an empty collection.");
            return *_begin;
        }

        inline TElement first_or_default(const TElement& def) const {
            return (empty()) ? def : *_begin;
        }

        TElement last() const {
            if(empty()) throw linq_exception("failed to get the last element from an empty collection.");
            auto itr = _begin;
            TElement res = *itr;
            while(++itr != _end)
                res = *itr;
            return res;
        }

        TElement last_of_default(const TElement& def) const {
            auto res = def;
            for(auto itr = _begin; itr != _end; ++itr)
                res = *itr;
            return res;
        }

        inline bool empty() const {
            return _end == _begin;
        }

        TElement at(int index) const {
            if(index >= 0) {
                auto itr = _begin;
                int num = 0;
                for(auto itr = _begin; itr != _end; ++itr)
                    if(num == index) return *itr;
            }
            throw linq_exception("index out of range.");
        }

        template < typename TIterator2 >
        bool sequence_equal_(const iteratorable<TIterator2>& rhand) const {
            auto itr_left = _begin;
            auto itr_right = rhand.begin();
            auto itr_right_end = rhand.end();

            while(itr_left != _end && itr_right != itr_right_end)
                if(*itr_left != *itr_right) return false;
            return(itr_left == _end && itr_right == itr_right_end);
        }
        SUPPORT_STL_CONTAINERS(sequence_equal)

        TElement operator[](int index) const {
            return at(index);
        }

        std::vector<TElement> to_vector() const {
            std::vector<TElement> res;
            for(auto itr = _begin; itr != _end; ++itr) 
                res.push_back(*itr);
            return std::move(res);
        }

        // 包装select_iterator
        template < typename TFunction >
        iteratorable<select_iterator<TIterator, TFunction>> select(const TFunction& f) const {
            return iteratorable<select_iterator<TIterator, TFunction>>(
                select_iterator<TIterator, TFunction>(_begin, f),
                select_iterator<TIterator, TFunction>(_end, f)
            );
        }

        // 包装where_iterator
        template < typename TFunction >
        iteratorable<where_iterator<TIterator, TFunction>> where(const TFunction& f) const {
            return iteratorable<where_iterator<TIterator, TFunction>>(
                where_iterator<TIterator, TFunction>(_begin, _end, f),
                where_iterator<TIterator, TFunction>(_end, _end, f)
            );
        }
        
    };

    template < typename T >
    class linq : public iteratorable<forward_iterator<T>> {
    public:
        linq() = default;
        template < typename TIterator >
        linq(const iteratorable<TIterator>& e) :
        iteratorable<forward_iterator<T>>(
            forward_iterator<T>(e.begin()),
            forward_iterator<T>(e.end())
        ) {}
    };


    
    template < typename TIterator >
    iteratorable<TIterator> from(const TIterator& begin, const TIterator& end) {
        return iteratorable<TIterator>(begin, end);
    }
    
    template < typename TContainer >
    auto from(const TContainer& container)->iteratorable<decltype(std::begin(container))> {
        return iteratorable<decltype(std::begin(container))>(std::begin(container), std::end(container));
    }

    template < typename T >
    linq<T> from_values(std::shared_ptr<std::vector<T>> ptr) {
        return iteratorable<storage_iterator<T>>(
                storage_iterator<T>(ptr, ptr->begin()),
                storage_iterator<T>(ptr, ptr->end())
            );
    }

    template < typename TElement >
    linq<TElement> from_values(const std::initializer_list<TElement>& init_list) {
        auto ptr = std::make_shared<std::vector<TElement>>(init_list.begin(), init_list.end());
        return iteratorable<storage_iterator<TElement>>(
                storage_iterator<TElement>(ptr, ptr->begin()),
                storage_iterator<TElement>(ptr, ptr->end())
            );
    }

    template < typename TElement >
    linq<TElement> from_value(const TElement& element) {
        auto ptr = std::make_shared<std::vector<TElement>>(std::vector<TElement>({element}));
        return from_values(ptr);
    }


    


}