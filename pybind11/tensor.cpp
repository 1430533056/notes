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

typedef enum
{
    INT8,
    INT16,
    INT32,
} Dtype;

std::string dtype2str(const Dtype &dtype)
{
    switch (dtype)
    {
    case INT8:
        return "int8";
    case INT16:
        return "int16";
    case INT32:
        return "int32";
    default:
        std::cout << "Not supported Dtype\n";
    }
    return "int32";
}

size_t dtype2size(const Dtype &dtype)
{
    switch (dtype)
    {
    case INT8:
        return sizeof(char);
    case INT16:
        return sizeof(short);
    case INT32:
        return sizeof(int);
    default:
        std::cout << "Not supported Dtype\n";
    }
    return sizeof(int);
}

Dtype numpy_dtype_to_dtype(py::dtype dtype)
{
    int python_dtype_num = dtype.num();
    switch (python_dtype_num)
    {
    case py::detail::npy_api::NPY_BYTE_:
        return INT8;
    case py::detail::npy_api::NPY_SHORT_:
        return INT16;
    case py::detail::npy_api::NPY_INT32_:
        return INT32;
    default:
        return INT32;
    }
}

class TensorImpl
{
public:
    TensorImpl(void *data, IntArray shape, Dtype dtype)
    {
        data_ = data;
        shape_ = shape;
        dtype_ = Dtype::INT32;
        size_t size = 1;
        for (size_t i = 0; i < shape.size(); i++)
            size *= shape[i];
        size_ = size;
        std::cout << "Impl ctr " << dtype_ << "\n";
    }
    TensorImpl(const TensorImpl &impl)
    {
        TensorImpl(impl.data_, impl.shape_, impl.dtype_);
    }
    py::array to_numpy()
    {
        auto str = dtype2str(dtype_);
        auto arr = py::array(py::dtype(str), shape_, data_);
        std::cout << "to_numpy:" << (size_t)data_ << "\n";
        return arr;
        // auto arr = py::array_t<int>(size_);
        // std::cout << "to_numpy:" << (size_t)arr.data() << "\n";
        // return arr;
    }
    void print()
    {
        // assert(dtype_ == Dtype::INT32);
        for (size_t i = 0; i < size_; i++)
        {
            std::cout << ((int *)data_)[i] << " ";
        }
        std::cout << std::endl;
    }
    // std::uintptr_t get_ptr() { reinterpret_cast<std::uintptr_t>(data_); }
    size_t get_ptr() { (size_t)data_; }

// private:
    void *data_ = nullptr;
    IntArray shape_;
    Dtype dtype_;
    size_t size_;
};

class Tensor
{
public:
    Tensor(void *data, IntArray shape, Dtype dtype)
    {
        tensor_impl_ = new TensorImpl(data, shape, dtype);
        std::cout << "Tensor ctr " << size_t(tensor_impl_->data_) << "\n";
    }
    ~Tensor()
    {
        if (!tensor_impl_) {
            free(tensor_impl_);
        }
        std::cout << "Tensor ~dtr" << "\n";
    }
    Tensor(const Tensor &t)
    {
        tensor_impl_ = new TensorImpl(t.tensor_impl_->data_, t.tensor_impl_->shape_, t.tensor_impl_->dtype_);
        std::cout << "Tensor copy ctr" << "\n";
    }
    Tensor(Tensor &&t)
    {
        tensor_impl_ = t.tensor_impl_;
        t.tensor_impl_ = nullptr;
        std::cout << "Tensor move ctr " << size_t(tensor_impl_->data_) << "\n";
    }

    py::array to_numpy()
    {
        return tensor_impl_->to_numpy();
    }

    void print() { tensor_impl_->print(); }

    size_t get_ptr() {
        std::cout << "get_ptr:" << (size_t)tensor_impl_->data_ << "\n";
        std::cout << "get_ptr:" << (size_t)(tensor_impl_->get_ptr()) << "\n";
        return (size_t)tensor_impl_->data_;
    }

    static Tensor make_tensor(IntArray shape, Dtype dtype)
    {
        size_t size = 1;
        for (size_t i = 0; i < shape.size(); i++)
            size *= shape[i];
        size_t item_size = dtype2size(dtype);
        void *ptr = malloc(size * item_size);
        std::cout << "make_tensor ptr:" << ptr << "\n";
        return Tensor(ptr, shape, dtype);
    }

// private:
    TensorImpl *tensor_impl_ = nullptr;
};

Tensor from_numpy(py::array array)
{
    auto arr_dtype = array.dtype();
    Dtype dtype = numpy_dtype_to_dtype(arr_dtype);
    // void *data = array.mutable_data();
    void *data = (void *)array.data();
    std::cout << (size_t)data << " 1-\n";
    auto shape = IntArray(array.shape(), array.shape() + array.ndim());
    // array.inc_ref();
    // return Tensor(data_ptr, shape, [array](void *data){ array.dec_ref();});
    return Tensor(data, shape, dtype);
}

PYBIND11_MODULE(e, m)
{
    m.doc() = "pybind11 example plugin";

    py::class_<TensorImpl>(m, "TensorImpl")
        .def_readwrite("data", &TensorImpl::data_)
        .def_readwrite("dtype", &TensorImpl::dtype_);

    py::enum_<Dtype>(m, "Dtype")
        .value("int8", Dtype::INT8)
        .value("int16", Dtype::INT16)
        .value("int32", Dtype::INT32)
        .export_values();

    py::class_<Tensor>(m, "Tensor")
        .def(py::init<void *, const IntArray &, Dtype>())
        .def(py::init<const Tensor &>())
        // .def_readwrite("impl", &Tensor::tensor_impl_)
        .def("to_numpy", &Tensor::to_numpy)
        .def("print", &Tensor::print)
        .def("get_ptr", &Tensor::get_ptr);

    m.def("make_tensor", &Tensor::make_tensor, py::arg("shape"), py::arg("dtype") = Dtype::INT32);
    // m.def("from_numpy", &from_numpy, py::return_value_policy::take_ownership);
    m.def("from_numpy", &from_numpy);
}

// c++ -O3 -Wall -shared -std=c++11 -fPIC $(python3 -m pybind11 --includes) tensor.cpp -o tensor$(python3-config --extension-suffix)