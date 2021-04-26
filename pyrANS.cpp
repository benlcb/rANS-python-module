//set set expandtab tabstop=4 shiftwidth=4 softtabstop=4
#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1;
#include <boost/python/numpy.hpp>
#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>
#include <boost/python/args.hpp>
#include <iostream>
#include "rANSCoder.h"

namespace np = boost::python::numpy;
namespace py = boost::python;

using namespace std;
typedef unsigned char uchar;
typedef unsigned long ulong;

class pyrANS : public rANSCoder{

public:

    void init_dc(np::ndarray r){
        std::vector<uint32_t> vdc_bs = std::vector<uint32_t>((uint32_t*)r.get_data(),(uint32_t*)r.get_data()+r.shape(0));
        rANSCoder::init_dc(vdc_bs);
    }

    np::ndarray get_ec_buf(){

        std::vector<uint32_t> data = get_buffer();

        np::ndarray r = np::from_data(data.data(), np::dtype::get_builtin<uint32_t>(),
                                      py::make_tuple(data.size()),
                                      py::make_tuple(sizeof(uint32_t)),
                                      py::object());
        return  r.copy();
    }


    void encode_sym(uint32_t sym, np::ndarray pdf){
        np::ndarray pdf_as_float = pdf.astype(np::dtype::get_builtin<float>());
        std::vector<float> vpdf = std::vector<float>((float*)pdf_as_float.get_data(),(float*)pdf_as_float.get_data()+pdf_as_float.shape(0));
        rANSCoder::encode_sym(sym, vpdf);
    }

    uint32_t decode_sym(np::ndarray pdf){
        np::ndarray pdf_as_float = pdf.astype(np::dtype::get_builtin<float>());
        auto vpdf = std::vector<float>((float*)pdf_as_float.get_data(),(float*)pdf_as_float.get_data()+pdf_as_float.shape(0));
        return rANSCoder::decode_sym(vpdf);
    }

    pyrANS(const unsigned int& floatshift, const unsigned int& prob_bits) {
        rANSCoder(floatshift, prob_bits);
    }

    pyrANS() {
        rANSCoder();
    }

};


BOOST_PYTHON_MODULE(pyrANS)
{
    Py_Initialize();
    np::initialize();

    py::class_<pyrANS>("pyrANS")
        .def(py::init<uint32_t, uint32_t>())
        .def("encode_sym",&pyrANS::encode_sym, boost::python::args("symbol","pdf"), "Encodes a symbol, which is an uint32_t value. Symbol is the symbol to encode, pdf is the corresponding probability density function, where pdf[i] is the probability of symbol i. pdf.size() has to be equal to the alphabet size.")
        .def("decode_sym",&pyrANS::decode_sym,  boost::python::args("pdf"), "Decodes and advances the coder to the next symbol. Pdf is the probability density function, where pdf[i] is the probability of symbol i. pdf.size() has to be equal to the alphabet size.")

        .def("init_ec",&pyrANS::init_ec, "Initializes encoder. This is usually not necessary since the Coder should always be in a valid state.")
        .def("init_dc",&pyrANS::init_dc, boost::python::args("data"), "Initializes the decoder with the buffer obtained by calling get_ec_buf.")
        .def("get_ec_buf",&pyrANS::get_ec_buf, "Flushes the coder state into the buffer and returns the buffer. Coder is reset after calling this function.")
        ; 

}

