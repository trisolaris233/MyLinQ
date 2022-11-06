#include "linq.h"
#include <iostream>
#include <utility>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <set>
#include <cassert>
#include <memory>
#include <initializer_list>


void test();

int main() {
    test();
    return 0;
}

void test() {
    ////////////////////////////////////////
    //////// forward_iterator
    ///////////////////////////////////////
    {
        std::vector<int> xs({1, 2, 3, 4, 5});
        using namespace trisolaris;

        forward_iterator<int> p1(xs.begin());
        for(auto i = 0; i < xs.size(); ++i) {
            std::cout << *(p1++) << " ";
        }
        std::cout << std::endl;
        // prints 1 2 3 4 5

        auto ys = {1, 2, 3, 4, 5, 6, 7};
        forward_iterator<int> p2(std::begin(ys));
        for(auto i : ys) {
            std::cout << *p2 << " ";
            ++p2;
        }
        std::cout << std::endl;
        // prints 1 2 3 4 5 6 7

        auto zs = {1, 2, 3, 4, 5, 6};
        forward_iterator<int> p3(std::begin(zs));
        forward_iterator<int> p4(std::begin(zs));
        forward_iterator<int> p5(std::begin(ys));

        assert((p3 == p4) && (p4 != p5) && (p3 != p5));
        std::cout << "forward_iterator test complete." << std::endl;
    }

    ////////////////////////////////////////
    //////// storage_iteartor
    ///////////////////////////////////////
    {
        // std::vector<int> xs({11, 22, 33, 44, 55});
        // trisolaris::storage_iterator<int> p1(std::make_shared<std::vector<int>>(xs));
        // for(auto i = 0; i < xs.size(); ++i) {
        //     std::cout << *p1 << " ";
        //     p1++;
        // }
        // std::cout << std::endl;
        // prints 11 22 33 44 55
    }

    ////////////////////////////////////////
    //////// select_iterator
    ///////////////////////////////////////
    {
        struct pfun1 {
            int operator()(int x) const {
                return x*x;
            }
        };
        auto pfun2 = [](int x) { return x*x*x; };
        std::vector<int> xs({2, 3, 4, 5, 6, 7, 8, 9, 10});
        trisolaris::select_iterator<decltype(xs.begin()), pfun1> p1(xs.begin(), pfun1());
        for(auto i : xs) {
            std::cout << *p1 << " ";
            ++p1;
        }
        std::cout << std::endl;
        // prints 4 9 16 25 36 49 64 81 100
        trisolaris::select_iterator<decltype(xs.begin()), decltype(pfun2)> p2(xs.begin(), pfun2);
        for(auto i : xs) {
            std::cout << *p2 << " ";
            ++p2;
        }
        std::cout << std::endl;
        // prints 8 27 64 125 216 343 512 729 1000
    }

    ////////////////////////////////////////
    //////// where_iterator
    ///////////////////////////////////////
    {
        struct pfun1 {
            bool operator()(int x) const {
                return(x % 2) != 0;
            }
        };
        auto pfun2 = [](int x) { return (x % 2) == 0; };
        std::vector<int> xs({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
        trisolaris::where_iterator<decltype(xs.begin()), pfun1> p1(xs.begin(), xs.end(), pfun1());
        
        for(auto i = 0; i < 5; ++i) {
            std::cout << *p1 << " ";
            ++p1;
        }
        std::cout << std::endl;
        // prints 1 3 5 7 9

        trisolaris::where_iterator<decltype(xs.begin()), decltype(pfun2)> p2(xs.begin(), xs.end(), pfun2);
        for(auto i = 0; i < 5; ++i) {
            std::cout << *p2 << " ";
            p2++;
        }
        std::cout << std::endl;
        // prints 2 4 6 8 10
        
    }

    ////////////////////////////////////////
    //////// from
    ///////////////////////////////////////
    {   
        std::vector<int> xs({233, 466, 998});
        for(auto i : trisolaris::from(xs.begin(), xs.end())) {
            std::cout << i << " ";
        }
        std::cout << std::endl;

        for(auto i : trisolaris::from(xs)) {
            std::cout << i << " ";
        }
        std::cout << std::endl;

        for(auto i : trisolaris::from(trisolaris::from(trisolaris::from(xs))))
            std::cout << i << " ";
        // prints 233 466 998
        // for(auto i : trisolaris::from(xs).select([](int x){ return x*x; })) {
        //     std::cout << i << " ";
        // }
        // std::cout << std::endl;
    }

    ////////////////////////////////////////
    //////// select
    ///////////////////////////////////////
    {
        std::vector<int> xs({233, 466, 998});
        for(auto i : trisolaris::from(xs).select([](int x){ return x*x; })) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
    }

    ////////////////////////////////////////
    //////// where
    ///////////////////////////////////////
    {
        struct person {
            std::string     name;
            bool            is_male;
        };

        auto persons = { person{"wey", false}, person{"icelolly", false}, person{"404", false}, person{"sunshine+ice", true} };
        for(auto p : trisolaris::from(persons).where([](person p){ return !p.is_male; })) {
            printf("%s is a girl\n", p.name.c_str());
        }
        // prints:
        // wey is a girl
        // icelolly is a girl
        // 404 is a girl
        std::cout << std::endl;

        for(auto p : trisolaris::from(persons).select([](person p){ return person{p.name + "2333", !p.is_male}; }).where([](person p){ return !p.is_male; })) {
            printf("%s is a girl", p.name.c_str());
        }
        std::cout << std::endl;
        // prints:
        // sunshine+ice233 is a girl
    }

    {
        auto s = {1, 2, 3, 4};
        trisolaris::linq<int> p1 = trisolaris::from_values(s);
        for(auto i : p1.select([](int x){ return x * 3 + 2; })) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
        // prints 5 8 11 14

        for(auto i : trisolaris::from_value(5).select([](int x) { return (-x) - 2 * (x + 1); })) {
            std::cout << i << " ";
        }
        std::cout << std::endl;
        // prints -17

        
    }
}