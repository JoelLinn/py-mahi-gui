#include <Mahi/Gui.hpp>
#include <pybind11/pybind11.h>

#include "pybind-cast.hpp"

namespace py = pybind11;

void py_init_module_imgui(py::module&);

// helper type for exposing protected functions
class PubApplication : public mahi::gui::Application {
public:
  // inherited with different access modifier
  using mahi::gui::Application::draw_nanovg;
  using mahi::gui::Application::draw_opengl;
  using mahi::gui::Application::m_window;
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

  py::class_<mahi::gui::Application::Config>(m, "ApplicationConfig")
      .def(py::init<>())
      .def_readwrite("title", &mahi::gui::Application::Config::title)
      .def_readwrite("width", &mahi::gui::Application::Config::width)
      .def_readwrite("height", &mahi::gui::Application::Config::height)
      .def_readwrite("monitor", &mahi::gui::Application::Config::monitor)
      .def_readwrite("fullscreen", &mahi::gui::Application::Config::fullscreen)
      .def_readwrite("resizable", &mahi::gui::Application::Config::resizable)
      .def_readwrite("visible", &mahi::gui::Application::Config::visible)
      .def_readwrite("decorated", &mahi::gui::Application::Config::decorated)
      .def_readwrite("transparent",
                     &mahi::gui::Application::Config::transparent)
      .def_readwrite("center", &mahi::gui::Application::Config::center)
      .def_readwrite("msaa", &mahi::gui::Application::Config::msaa)
      .def_readwrite("nvg_aa", &mahi::gui::Application::Config::nvg_aa)
      .def_readwrite("vsync", &mahi::gui::Application::Config::vsync)
      .def_property(
          "background",
          [](const mahi::gui::Application::Config& self) -> py::tuple {
            auto& bg = self.background;
            return py::make_tuple(bg.r, bg.g, bg.b, bg.a);
          },
          [](mahi::gui::Application::Config& self,
             std::tuple<float, float, float, float> c) {
            auto& bg = self.background;
            std::tie(bg.r, bg.g, bg.b, bg.a) = c;
          });

  py::class_<mahi::gui::Application::Profile>(m, "ApplicationProfile")
      .def(py::init<>())
      .def_readwrite("poll", &mahi::gui::Application::Profile::t_poll)
      .def_readwrite("update", &mahi::gui::Application::Profile::t_update)
      .def_readwrite("coroutines",
                     &mahi::gui::Application::Profile::t_coroutines)
      .def_readwrite("gl", &mahi::gui::Application::Profile::t_gl)
      .def_readwrite("nvg", &mahi::gui::Application::Profile::t_nvg)
      .def_readwrite("imgui", &mahi::gui::Application::Profile::t_imgui)
      .def_readwrite("idle", &mahi::gui::Application::Profile::t_idle)
      .def_readwrite("buffers", &mahi::gui::Application::Profile::t_buffers);

#define APP_SELF mahi::gui::Application& self
  py::class_<mahi::gui::Application, PyApplication>(m, "Application")
      // public:
      .def(py::init<>())
      .def(py::init<const std::string&>())
      .def(py::init<const std::string&, int>())
      .def(py::init<int, int, const std::string&>())
      .def(py::init<int, int, const std::string&, bool>())
      .def(py::init<int, int, const std::string&, bool, int>())
      .def(py::init<mahi::gui::Application::Config>())
      // ======================================================================
      .def("run", &mahi::gui::Application::run)
      .def("quit", &mahi::gui::Application::quit)
      // ======================================================================
      .def("time", &mahi::gui::Application::time)
      .def("delta_time", &mahi::gui::Application::delta_time)
      .def("set_time", &mahi::gui::Application::set_time,
           py::arg("microseconds"))
      .def("set_time_scale", &mahi::gui::Application::set_time_scale)
      // ======================================================================
      .def(
          "set_background",
          [](APP_SELF, std::tuple<float, float, float, float> c) {
            mahi::gui::Color bg;
            std::tie(bg.r, bg.g, bg.b, bg.a) = c;
            self.set_background(bg);
          },
          py::arg("rgba"))
      .def("set_window_title", &mahi::gui::Application::set_window_title)
      .def("set_window_pos", &mahi::gui::Application::set_window_pos,
           py::arg("x"), py::arg("y"))
      .def("get_window_pos",
           [](const APP_SELF) -> py::tuple {
             auto [x, y] = self.get_window_pos();
             return py::make_tuple(x, y);
           })
      .def("set_window_size", &mahi::gui::Application::set_window_size,
           py::arg("width"), py::arg("height"))
      .def("get_window_size",
           [](const APP_SELF) -> py::tuple {
             auto [w, h] = self.get_window_size();
             return py::make_tuple(w, h);
           })
      .def("set_window_size_limits",
           &mahi::gui::Application::set_window_size_limits,
           py::arg("min_width"), py::arg("min_height"), py::arg("max_width"),
           py::arg("max_height"))
      .def("center_window", &mahi::gui::Application::center_window,
           py::arg("monitor") = 0)
      .def("minimize_window", &mahi::gui::Application::minimize_window)
      .def("maximize_window", &mahi::gui::Application::maximize_window)
      .def("restore_window", &mahi::gui::Application::restore_window)
      .def("hide_window", &mahi::gui::Application::hide_window)
      .def("show_window", &mahi::gui::Application::show_window)
      // .def("focus_window", &mahi::gui::Application::focus_window) // don't.
      .def("request_window_attention",
           &mahi::gui::Application::request_window_attention)
      .def("get_framebuffer_size",
           [](const APP_SELF) -> py::tuple {
             auto [w, h] = self.get_framebuffer_size();
             return py::make_tuple(w, h);
           })
      .def("get_pixel_ratio", &mahi::gui::Application::get_pixel_ratio)
      .def("set_vsync", &mahi::gui::Application::set_vsync, py::arg("enabled"))
      .def(
          "set_frame_limit",
          [](APP_SELF, int64_t freq) {
            self.set_frame_limit(mahi::util::hertz(freq));
          },
          py::arg("frequency"))
      .def("get_mouse_pos",
           [](const APP_SELF) -> py::tuple {
             auto [x, y] = self.get_mouse_pos();
             return py::make_tuple(x, y);
           })
      // ======================================================================
      .def("profile", &mahi::gui::Application::profile)
      // ======================================================================
      // protected:
      .def("update", &PubApplication::update)
      .def("draw_opengl", &PubApplication::draw_opengl)
      // .def("draw_nanovg", &mahi::gui::Application::draw);
      // ======================================================================
      // public:
      // TODO events
      // ======================================================================
      // protected:
      //.def("get_glfw_window", [](const APP_SELF) -> void* {
      //  return (static_cast<const PubApplication& >(self)).m_window;
      //})
      ;

  // py::class_<NVGcontext>(m, "NVGcontext");
}
