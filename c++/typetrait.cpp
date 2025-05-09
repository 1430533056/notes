#include<iostream>

class A {
public:
    using Type = int;
};

class B {
public:
    using Type = int;
    using DType = double;
};

template<class T, class = void>
class DType_Or_Default {
public:
    using type = typename T::Type;
};

template<class T>
using void_t = void;

template<class T>
class DType_Or_Default<T, void_t<typename T::DType>> {
public:
    using type = typename T::DType;
};

int main() {

    typename DType_Or_Default<A>::type a;
    typename DType_Or_Default<B>::type b;

    std::cout << sizeof(a) << " " << sizeof(b) << "\n";

}
