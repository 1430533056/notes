#include <immintrin.h>
#include <iostream>

class Base {
public:
  Base(Base && b) {std::cout << "move ctr\n";}
  Base(const Base & b) {std::cout << "copy ctr\n";}
  Base() {std::cout << "Base\n";}
  ~Base() {std::cout << "~Base\n";}
  int a = 0;
};

Base get1() {
  return Base();
}

Base& get2(Base && b) {
  std::cout << &b << " 2\n";
  return b;
}

Base&& get3(Base && b) {
  std::cout << &b << " 2\n";
  return std::move(b);
}

int test() {
  // Base b = get1();
  Base b;
  std::cout << "-- " << &b << " --\n";
  Base b1 = get1();
  std::cout << &b1 << " 1\n";
  std::cout << "--------\n";

  return 0;
}

void print(Base &base) {
  std::cout << "Base &\n";
}

void print(Base &&base) {
  std::cout << "Base &&\n";
}

template<typename _Tp>
constexpr _Tp&&
my_forward(typename std::remove_reference<_Tp>::type& __t) noexcept
{
  std::cout << "forward &\n";
  return static_cast<_Tp&&>(__t); }

template<typename _Tp>
constexpr _Tp&&
my_forward(typename std::remove_reference<_Tp>::type&& __t) noexcept
{
  static_assert(!std::is_lvalue_reference<_Tp>::value, "template argument"
		    " substituting _Tp is an lvalue reference type");
  std::cout << "forward &&\n";
  return static_cast<_Tp&&>(__t);
}

template<typename T>
void printHelp(T && base) {
  print(my_forward<T>(std::move(base)));
}

int main() {
  Base b1;
  printHelp(b1);
  printHelp(Base());
  return 0;
}