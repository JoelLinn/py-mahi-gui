#include <Mahi/Gui.hpp>
#include <pybind11/pybind11.h>

#include <iostream>

namespace py = pybind11;

// helper type for exposing protected functions
class PubApplication : public mahi::gui::Application {
public:
  // inherited with different access modifier
  using mahi::gui::Application::draw;
  using mahi::gui::Application::update;
};

class PyApplication : public mahi::gui::Application {
public:
  using mahi::gui::Application::Application;

protected:
  void update() override {
    PYBIND11_OVERLOAD(void, mahi::gui::Application, update);
  }
  void draw() override {
    // PYBIND11_OVERLOAD_NAME(void, mahi::gui::Application, draw, draw_opengl);
  }

  void draw(NVGcontext* nvg) override {
    // PYBIND11_OVERLOAD_NAME(void, mahi::gui::Application, draw, draw_nanovg,
    //                        NVGcontext*);
  }
};

PYBIND11_MODULE(mahi_gui, m) {
  py::class_<mahi::gui::Application, PyApplication>(m, "Application")
      .def(py::init<>())
      .def(py::init<const std::string&>())
      .def(py::init<const std::string&, int>())
      .def(py::init<int, int, const std::string&>())
      .def(py::init<int, int, const std::string&, bool>())
      .def(py::init<int, int, const std::string&, bool, int>())
      // .def(py::init<mahi::gui::Config>())
      .def("update", &PubApplication::update)
      // .def("draw_opengl", &mahi::gui::Application::draw)
      // .def("draw_nanovg", &mahi::gui::Application::draw);
      .def("run", &mahi::gui::Application::run)
      .def("quit", &mahi::gui::Application::quit);

  // py::class_<NVGcontext>(m, "NVGcontext");

  m.def("demo_imgui", []() -> bool {
    bool open = true;
    ImGui::ShowDemoWindow(&open);
    return open;
  });
}
