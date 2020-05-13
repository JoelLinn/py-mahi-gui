#include <Mahi/Gui.hpp>
#include <pybind11/pybind11.h>

namespace py = pybind11;

void py_init_module_imgui(py::module&);

// helper type for exposing protected functions
class PubApplication : public mahi::gui::Application {
public:
  // inherited with different access modifier
  using mahi::gui::Application::draw_nanovg;
  using mahi::gui::Application::draw_opengl;
  using mahi::gui::Application::update;
};

class PyApplication : public mahi::gui::Application {
public:
  using mahi::gui::Application::Application;

protected:
  void update() override {
    PYBIND11_OVERLOAD(void, mahi::gui::Application, update);
  }
  void draw_opengl() override {
    PYBIND11_OVERLOAD(void, mahi::gui::Application, draw_opengl);
  }

  // void draw_nanovg(NVGcontext* nvg) override {
  //  PYBIND11_OVERLOAD(void, mahi::gui::Application, draw_nanovg, nvg);
  //}
};

PYBIND11_MODULE(mahi_gui, m) {
  py_init_module_imgui(m);

  py::class_<mahi::gui::Application, PyApplication>(m, "Application")
      .def(py::init<>())
      .def(py::init<const std::string&>())
      .def(py::init<const std::string&, int>())
      .def(py::init<int, int, const std::string&>())
      .def(py::init<int, int, const std::string&, bool>())
      .def(py::init<int, int, const std::string&, bool, int>())
      // .def(py::init<mahi::gui::Config>())
      .def("update", &PubApplication::update)
      .def("draw_opengl", &PubApplication::draw_opengl)
      // .def("draw_nanovg", &mahi::gui::Application::draw);
      .def("get_window_size_width",
           [](const mahi::gui::Application& self) -> int {
             auto size = self.get_window_size();
             return size.x;
           })
      .def("get_window_size_height",
           [](const mahi::gui::Application& self) -> int {
             auto size = self.get_window_size();
             return size.y;
           })
      .def("run", &mahi::gui::Application::run)
      .def("quit", &mahi::gui::Application::quit);

  // py::class_<NVGcontext>(m, "NVGcontext");
}
