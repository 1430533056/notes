#include <pybind11/pybind11.h>
#include <pybind11/detail/common.h>
#include <iostream>
#include <cstdint>
#include <vector>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>
#include <memory>

namespace py = pybind11;

int g = 0;

int add(int i, int j) {
    return i + j + g;
}

class Tensor {
public:
  int *data = nullptr;
  int len;
  void show() const {
    std::cout << "ptr:" << data << "\n";
    for (int i = 0; i < len - 1; i++) {
      std::cout << data[i] << " ";
    }
    std::cout << data[len-1] << ".\n";
  }
  Tensor(int length) : len(length) {
    data = new int[len];
    for (int i = 0; i < len; i++) {
      data[i] = i;
    }
    std::cout << "Tensor ctr\n";
  }
  Tensor(Tensor &&t){
    if (data) delete[] data;
    len = t.len;
    data = t.data;
    t.data = nullptr;
    std::cout << "Tensor move ctr\n";
  }
  Tensor(Tensor &t){
    if (data) delete[] data;
    len = t.len;
    data = t.data;
    t.data = nullptr;
    std::cout << "Tensor cpy ctr\n";
  }
  ~Tensor() {
    if (data) {
      delete[] data;
    }
    std::cout << "Tensor ~ ctr\n";
  }
  std::uintptr_t  get_data() {
    // return py::cast(data);
    return reinterpret_cast<std::uintptr_t>(data);
  }
};

Tensor get(int m) {
  return Tensor(m);
}

void print_tensor(const Tensor& t) {
  t.show();
}

PYBIND11_MAKE_OPAQUE(std::vector<int>);

int add_vector(std::vector<int> &v) {
  int sum = 0;
  for (unsigned int i = 0; i < v.size(); i++) {
    sum += v[i];
    v[i] += 1;
  }
  return sum;
}

PYBIND11_MAKE_OPAQUE(std::vector<float>);

template<typename T>
void print_vector(std::vector<T> &v) {
  for (auto ele : v) {
    std::cout << ele << " " << typeid(ele).name() << " ";
  }
  std::cout << "\n";
}

Tensor *retTensor() {
  Tensor *p = new Tensor(2);
  std::cout << p << " " << p->get_data() << "\n";
  return p;
}

void modify(int *a) {
  std::cout << "before:" << *a << "\n";
  *a = 100;
  std::cout << "after:" << *a << "\n";
}

class A {
public:
  A() {std::cout << "A ctr\n";}
  ~A() {std::cout << "A ~ctr\n";}
};


static A a;

std::shared_ptr<A> get_shared_ptr() {
  // return std::make_shared<A>();
  return std::shared_ptr<A>(&a);
}

PYBIND11_MODULE(e, m) {
    m.doc() = "pybind11 example plugin";

    m.def("add", &add, "A function that adds two numbers", py::arg("i")=1, py::arg("j")=2);
    m.attr("kk") = 42;

    py::class_<std::vector<int>>(m, "IntVector")
    .def(py::init<>())
    .def("clear", &std::vector<int>::clear)
    .def("push_back", (void(std::vector<int>::*)(const int &))&std::vector<int>::push_back)
    .def("__len__", [](const std::vector<int> &v) { return v.size(); })
    .def("__iter__", [](std::vector<int> &v) {
       return py::make_iterator(v.begin(), v.end());
    }, py::keep_alive<0, 1>()); /* Keep vector alive while iterator is used */

    py::bind_vector<std::vector<float>>(m, "FloatVector");
    py::implicitly_convertible<py::list, std::vector<int>>();
    py::implicitly_convertible<py::list, std::vector<float>>();

    py::class_<Tensor>(m, "Tensor")
      .def(py::init<int>())
      .def(py::init<Tensor &>())
      .def_readwrite("len", &Tensor::len)
      .def_readonly("data", &Tensor::data)
      .def("show", &Tensor::show)
      .def("get_data", &Tensor::get_data);

    m.def("modify", &modify);
    m.def("get", &get);
    m.def("print_tensor", &print_tensor);
    m.def("add_vector", &add_vector);
    m.def("print_vector", &print_vector<float>);
    m.def("retTensor", &retTensor);

    m.def("f", []() {
        size_t shape[3] = {2, 10, 10};
        auto a = py::array_t<double>(shape);
        auto view = a.mutable_unchecked<3>();

        for(size_t i = 0; i < a.shape(0); i++)
        {
          for(size_t j = 0; j < a.shape(1); j++)
          {
            for(size_t k = 0; k < a.shape(2); k++)
            {
              view(i,j,k) = i * a.shape(1) * a.shape(2) + j * a.shape(2) + k;
            }
          }
        }
        return a;
    });

    m.def("clear", [](py::array_t<double> & a) {
        auto view = a.mutable_unchecked<3>();
        std::cout << a.request().ptr << "\n";
        for(size_t i = 0; i < a.shape(0); i++)
        {
          for(size_t j = 0; j < a.shape(1); j++)
          {
            for(size_t k = 0; k < a.shape(2); k++)
            {
              view(i,j,k) = 0;
            }
          }
        }
      }
    );

    m.def("f2", [](std::string str) {
        py::dtype dtype = py::dtype(str);
        void *ptr = (void *)new int[2 * 10];
        auto a = py::array(dtype, {2,10}, ptr);
        return a;
    });

    class Internal{};

    class Example {
      public:
          void print() { std::cout << &internal << "\n"; }
          Internal &get_internal() { return internal; }
      private:
          Internal internal;
    };

    py::class_<Internal>(m, "Internal");

    py::class_<Example>(m, "Example")
        .def(py::init<>())
        .def("print", &Example::print)
        .def("get_internal", &Example::get_internal, "Return the internal data",
                             py::return_value_policy::reference_internal);
    py::class_<A, std::shared_ptr<A>>(m, "A")
      .def(py::init<>());

    m.def("get_shared_ptr", &get_shared_ptr, py::return_value_policy::reference);

}