#include <iostream>

class Object {};

template<typename T>
class ObjectPtr {
public:
    void *ptr;
    template<typename U,  typename = std::enable_if_t<std::is_base_of<T, U>::value>>
    ObjectPtr(const ObjectPtr<U> &object) {
        ptr = object.ptr;
    }
    ObjectPtr() = default;
};

class ObjectRef {
public:
    ObjectPtr<Object> data;
    template<typename T>
    ObjectRef(const ObjectPtr<T>& ptr) : data(ptr) {}
};

class Cat : public Object {
public:
    int id;
};

static auto l = [] () -> bool {
  std::cout << "hello\n";
  return true;
}();

int main() {
    Cat cat;
    ObjectPtr<Cat> ptr;
    ObjectRef ref(ptr);
    return 1;
}