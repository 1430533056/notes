#include <pybind11/pybind11.h>
#include <pybind11/detail/common.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>
#include <iostream>
#include <cstdint>
#include <vector>
#include <memory>

namespace py = pybind11;

using IntArray = std::vector<size_t>;

typedef enum {
  INT8,
  INT16,
  INT32,
} Dtype;

std::string dtype2str(const Dtype& dtype) {
  switch (dtype) {
    case INT8:  return "int8";
    case INT16: return "int16";
    case INT32: return "int32";
    default: std::cout << "Not supported Dtype\n";
  }
  return "int32";
}

size_t dtype2size(const Dtype& dtype) {
  switch (dtype) {
    case INT8:  return sizeof(char);
    case INT16: return sizeof(short);
    case INT32: return sizeof(int);
    default: std::cout << "Not supported Dtype\n";
  }
  return sizeof(int);
}

Dtype numpy_dtype_to_dtype(py::dtype dtype) {
  int python_dtype_num = dtype.num();
  switch (python_dtype_num) {
    case py::detail::npy_api::NPY_BYTE_:
      return INT8;
    case py::detail::npy_api::NPY_SHORT_:
      return INT16;
    case py::detail::npy_api::NPY_INT32_:
      return INT32;
    default: return INT32;
  }
}

class Tensor {
public:
  Tensor(void *data, IntArray shape, Dtype dtype) {
    set_items(data, shape, dtype);
    std::cout << "Tensor ctr" << " ptr:" << data_ << "\n";
  }
  Tensor(const Tensor& t) {
    set_items(t.data_, t.shape_, t.dtype_);
    std::cout << "Tensor copy ctr" << " ptr:" << data_ << "\n";
  }
  Tensor(Tensor&& t) {
    set_items(t.data_, t.shape_, t.dtype_);
    std::cout << "Tensor move ctr" << " ptr:" << data_ << "\n";
  }
  void set_items(void *data, IntArray shape, Dtype dtype) {
    data_ = data;
    shape_ = shape;
    dtype_ = dtype;
    size_t size = 1;
    for (size_t i = 0; i < shape_.size(); i++)
      size *= shape[i];
    size_ = size;
    inc_ref();
  }
  void inc_ref() { ref_count++; }
  void dec_ref() { ref_count--; }

  ~Tensor() {
    std::cout << "Tensor ~ctr" << " ptr:" << data_ << "\n";
    dec_ref();
    if (ref_count == 0 && data_)
      free(data_);
  }
  void print() {
    for (size_t i = 0; i < size_; i++) {
      std::cout << ((int*)data_)[i] << " ";
    }
    std::cout << "\n";
  }

  py::array to_numpy() {
    py::capsule buffer_handle(data_, [](void *ptr){ if (ptr) free(ptr); std::cout << "handle\n"; });
    auto str = dtype2str(dtype_);
    auto arr = py::array(py::dtype(str), shape_, data_, buffer_handle);
    arr.inc_ref();
    inc_ref();
    return arr;
  }

  std::uintptr_t get_ptr() { return reinterpret_cast<std::uintptr_t>(data_); }

  static Tensor make_tensor(IntArray shape, Dtype dtype) {
    size_t size = 1;
    for (size_t i = 0; i < shape.size(); i++)
      size *= shape[i];
    size_t item_size = dtype2size(dtype);
    void *ptr = malloc(size * item_size);
    std::cout << "make_tensor ptr:" << ptr << "\n";
    return Tensor(ptr, shape, dtype);
  }

  void *data_ = nullptr;
  IntArray shape_;
  Dtype dtype_;
  size_t size_;
  int ref_count = 0;
};

Tensor from_numpy(py::array array) {
  auto arr_dtype = array.dtype();
  Dtype dtype = numpy_dtype_to_dtype(arr_dtype);
  void *data = (void *)array.data();
  auto shape = IntArray(array.shape(), array.shape() + array.ndim());
  std::cout << "ptr:" << data << " dtype:" << dtype << "\n";
  array.inc_ref();
  auto t = Tensor(data, shape, dtype);
  t.inc_ref();
  return t;
  // return Tensor(data_ptr, shape, [array](void *data){ array.dec_ref();});
  // return Tensor(data, shape, dtype);
}

static Tensor tensor(nullptr, {2, 4}, Dtype::INT32);

Tensor *get_tensor() {
  std::cout << "get_tensor:" << &tensor << "\n";
  return &tensor;
}

PYBIND11_MODULE(e, m) {
    m.doc() = "pybind11 example plugin";

    py::enum_<Dtype>(m, "Dtype")
      .value("int8", Dtype::INT8)
      .value("int16", Dtype::INT16)
      .value("int32", Dtype::INT32)
      .export_values();

    py::class_<Tensor>(m, "Tensor")
      .def(py::init<void*, const IntArray&, Dtype>())
      .def(py::init<const Tensor &>())
      .def_readwrite_static("ref_count", &Tensor::ref_count)
      .def_readwrite("data", &Tensor::data_)
      .def("to_numpy", &Tensor::to_numpy)
      .def("print", &Tensor::print)
      .def("get_ptr", &Tensor::get_ptr);

    m.def("make_tensor", &Tensor::make_tensor, py::arg("shape"), py::arg("dtype")=Dtype::INT32);
    // m.def("from_numpy", &from_numpy, py::return_value_policy::take_ownership);
    m.def("from_numpy", &from_numpy);
    m.def("get_tensor", &get_tensor, py::return_value_policy::copy);
}

// c++ -O3 -Wall -shared -std=c++11 -fPIC $(python3 -m pybind11 --includes) tensor.cpp -o tensor$(python3-config --extension-suffix)