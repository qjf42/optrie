#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "op_trie.h"

namespace optrie {

namespace py = pybind11;
using namespace pybind11::literals;

PYBIND11_MODULE(optrie, m) {
    m.doc() = "A simple template matcher";
    m.def("set_max_match_len", &set_max_match_len,
      "Set max string length to match.\n"
      "Default 64, lower for better performance in some cases,"
      "called before loading template files.",
      "max_match_len"_a);
    py::class_<MatchResult>(m, "MatchResult")
        .def_readonly("matched", &MatchResult::matched)
        .def_readonly("score", &MatchResult::score)
        .def_readonly("groups", &MatchResult::groups)
        .def_readonly("extra_info", &MatchResult::extra)
        .def_readonly("template", &MatchResult::tpl);
    py::class_<OpTrie>(m, "OpTrie")
        .def(py::init<>())
        .def("load", &OpTrie::load, "load template and dict files", "template_files"_a, "dict_files"_a)
        .def("show", &OpTrie::show, "print op trie")
        .def("match", &OpTrie::match, "match string", "string"_a);
}

}  // namespace optrie
