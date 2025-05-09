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

class Dim {
public:
    int row;
    int col;
    Dim(int row, int col) : row(row), col(col) {
        std::cout << "Dim ctr\n";
    }
    Dim(const Dim &dim) : row(dim.row), col(dim.col) {
        std::cout << "Dim copy ctr\n";
    }
    ~Dim() {
        std::cout << "Dim ~dtr\n";
    }
    void print() {
        std::cout << "row: " << row << " col: " << col << "\n";
    }
};

class Matrix {
public:
    Matrix(size_t rows, size_t cols) : m_rows(rows), m_cols(cols) {
        std::cout << "Matrix ctr1\n";
        m_data = new float[rows*cols];
        for (int i = 0; i < rows * cols; i++){
          m_data[i] = i;
        }
        owned_data = 1;
    }
    Matrix(float *data, size_t rows, size_t cols) {
        std::cout << "Matrix ctr2\n";
        m_data = data;
        m_rows = rows;
        m_cols = cols;
        owned_data = 0;
    }
    ~Matrix() {
        std::cout << "Matrix ~dtr1\n";
        if (owned_data) {
            delete[] m_data;
        }
    }
    void print() {
        for (int i = 0; i < m_rows * m_cols; i++){
            std::cout << m_data[i] << " ";
        }
        std::cout << "\n";
    }
    std::shared_ptr<Dim> test_shared() {
        return std::make_shared<Dim>(2, 3);
    }
    std::unique_ptr<Dim> test_unique() {
        return std::make_unique<Dim>(3, 4);
    }
    std::uintptr_t ptr() { return reinterpret_cast<std::uintptr_t>((void *)m_data); }
    float* data () const { return m_data; }
    size_t rows() const { return m_rows; }
    size_t cols() const { return m_cols; }
private:
    size_t m_rows, m_cols;
    float *m_data;
    int owned_data;
};

PYBIND11_MODULE(e, m) {
    m.doc() = "pybind11 example plugin";

    py::class_<Dim, std::shared_ptr<Dim>>(m, "Dim")
    // py::class_<Dim, std::unique_ptr<Dim>>(m, "Dim")
      .def(py::init<int, int>())
      .def("print",&Dim::print);

    py::class_<Matrix>(m, "Matrix", py::buffer_protocol())
      .def(py::init<size_t, size_t>())
      .def(py::init([] (py::buffer b) {
        py::buffer_info info = b.request();
        return Matrix((float *)(info.ptr), info.shape[0], info.shape[1]);
      }))
      .def("print", &Matrix::print)
      .def("test_shared", &Matrix::test_shared)
      .def("test_unique", &Matrix::test_unique)
      .def("ptr", &Matrix::ptr)
      .def_buffer([](Matrix &m) -> py::buffer_info {
            return py::buffer_info(
                m.data(),                               /* Pointer to buffer */
                sizeof(float),                          /* Size of one scalar */
                py::format_descriptor<float>::format(), /* Python struct-style format descriptor */
                2,                                      /* Number of dimensions */
                { m.rows(), m.cols() },                 /* Buffer dimensions */
                { sizeof(float) * m.cols(),             /* Strides (in bytes) for each index */
                  sizeof(float) }
            );
        });
}