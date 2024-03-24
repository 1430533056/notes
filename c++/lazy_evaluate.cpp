#include<iostream>

template<typename T>
struct Expr{
  const T& self(void) const {
    return *(static_cast<const T*>(this));
  }
};

// var
struct Vec : public Expr<Vec>
{
  float *data;
  int len;
  Vec(float *ptr, int count) : data(ptr), len(count) {}
  float eval(int i) const {
    return data[i];
  }
  template<typename T>
  Vec& operator=(const Expr<T> &p) {
    for (int i = 0; i < len; i++) {
      data[i] = p.self().eval(i);
    }
    return *this;
  }
  void print() {
    for (int i = 0; i < len; i++) {
      std::cout << data[i] << " ";
    }
    std::cout << "\n";
  }
};

struct AddOp {
  inline static float Map(float a, float b) {
    return a + b;
  }
};

struct MulOp {
  inline static float Map(float a, float b) {
    return a * b;
  }
};


template<typename LH, typename RH, typename Op>
struct BinaryOp : public Expr<BinaryOp<LH, RH, Op>> {
  const LH &lh;
  const RH &rh;
  BinaryOp(const LH &lh, const RH &rh) : lh(lh), rh(rh) {}
  float eval(int i) const {
    return Op::Map(lh.eval(i), rh.eval(i));
  }
};

template<typename LH, typename RH>
BinaryOp<LH, RH, AddOp> operator+(const Expr<LH> &lh, const Expr<RH> &rh) {
  return BinaryOp<LH, RH, AddOp>(lh.self(), rh.self());
}

template<typename LH, typename RH>
BinaryOp<LH, RH, MulOp> operator*(const Expr<LH> &lh, const Expr<RH> &rh) {
  return BinaryOp<LH, RH, MulOp>(lh.self(), rh.self());
}


int main() {
  float a[4] = {1,2,3,4};
  float b[4] = {2,3,4,5};
  float c[4] = {7,7,7,7};
  Vec v1(a, 4), v2(b, 4), v3(c, 4);
  v3 = v1 + v2 * v1;
  auto v4 = v1 + v2;
  v1.print();
  v2.print();
  v3.print();
  std::cout << v4.eval(0) << "\n";
}