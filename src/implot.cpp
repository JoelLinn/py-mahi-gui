/******************************************************************************

Copyright 2020 Joel Linn

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

You are under no obligation whatsoever to provide any bug fixes, patches, or
upgrades to the features, functionality or performance of the source code
("Enhancements") to anyone; however, if you choose to make your Enhancements
available either publicly, or directly to the author of this software, without
imposing a separate written license agreement for such Enhancements, then you
hereby grant the following license: a non-exclusive, royalty-free perpetual
license to install, use, modify, prepare derivative works, incorporate into
other computer software, distribute, and sublicense such enhancements or
derivative works thereof, in binary and source code form.

******************************************************************************/

#include <implot.h>
#include <pybind11/pybind11.h>

#include "imgui_helper.h"
#include "leaked_ptr.h"

namespace py = pybind11;

#define PY_BUF_IS_TYPE(__type__, __buffer_info__)                              \
  (py::detail::compare_buffer_info<__type__>::compare(__buffer_info__))

// RAII Helper to pin buffers and template expand correct callback getter func.
struct ValueGetter {
public:
  explicit ValueGetter(const py::buffer& bufY)
      : hasX(false), infoY(bufY.request()) {
    if (this->infoY.ndim != 1) {
      throw std::runtime_error(error_dim);
    }
  }
  explicit ValueGetter(const py::buffer& bufX, const py::buffer& bufY)
      : hasX(true), infoX(bufX.request()), infoY(bufY.request()) {
    if (this->infoX.ndim != 1 || this->infoY.ndim != 1 ||
        this->infoX.shape.at(0) != this->infoY.shape.at(0)) {
      throw std::runtime_error(error_dim);
    }
  }

  typedef ImPlotPoint getter_func(void* data, int idx);
  [[nodiscard]] getter_func* get_getter_func() const {
#define VG_EMIT_GET_GETTER_Y(__type__)                                         \
  if (PY_BUF_IS_TYPE(__type__, this->infoX)) {                                 \
    return get_getter_func_y<__type__>();                                      \
  }

    if (hasX) {
      VG_EMIT_GET_GETTER_Y(bool);
      VG_EMIT_GET_GETTER_Y(float);
      VG_EMIT_GET_GETTER_Y(double);
      VG_EMIT_GET_GETTER_Y(int8_t);
      VG_EMIT_GET_GETTER_Y(uint8_t);
      VG_EMIT_GET_GETTER_Y(int16_t);
      VG_EMIT_GET_GETTER_Y(uint16_t);
      VG_EMIT_GET_GETTER_Y(int32_t);
      VG_EMIT_GET_GETTER_Y(uint32_t);
      VG_EMIT_GET_GETTER_Y(int64_t);
      VG_EMIT_GET_GETTER_Y(uint64_t);
      throw std::runtime_error(error_type);
    } else {
      return get_getter_func_y<void>();
    }
#undef VG_EMIT_GET_GETTER_Y
  }

  [[nodiscard]] int count() const {
    auto count = this->infoY.shape.at(0);
    assert(count >= 0);
    assert(count <= std::numeric_limits<int>::max());
    return static_cast<int>(count);
  };

protected:
  static const constexpr char* error_dim = "Incompatible buffer dimension!";
  static const constexpr char* error_type =
      "Incompatible format: expected array of bool, float, double or "
      "unsigned/signed int 8, 16, 32 or 64!";

  template <typename X, typename Y>
  static ImPlotPoint getValue(void* data, int idx) {
    const auto* this_ = static_cast<ValueGetter*>(data);
    double x, y;
    if constexpr (std::is_void<X>::value) {
      x = static_cast<double>(idx);
    } else {
      x = static_cast<double>((static_cast<X*>(this_->infoX.ptr))[idx]);
    }
    y = static_cast<double>((static_cast<Y*>(this_->infoY.ptr))[idx]);
    return ImPlotPoint(x, y);
  }

  template <typename X>
  [[nodiscard]] inline getter_func* get_getter_func_y() const {
#define VG_EMIT_RET_GETTER(__type__)                                           \
  if (PY_BUF_IS_TYPE(__type__, this->infoY)) {                                 \
    return &getValue<X, __type__>;                                             \
  }

    VG_EMIT_RET_GETTER(bool);
    VG_EMIT_RET_GETTER(float);
    VG_EMIT_RET_GETTER(double);
    VG_EMIT_RET_GETTER(int8_t);
    VG_EMIT_RET_GETTER(uint8_t);
    VG_EMIT_RET_GETTER(int16_t);
    VG_EMIT_RET_GETTER(uint16_t);
    VG_EMIT_RET_GETTER(int32_t);
    VG_EMIT_RET_GETTER(uint32_t);
    VG_EMIT_RET_GETTER(int64_t);
    VG_EMIT_RET_GETTER(uint64_t);
    throw std::runtime_error(error_type);
#undef VG_EMIT_RET_GETTER
  }

private:
  const bool hasX;
  const py::buffer_info infoX;
  const py::buffer_info infoY;
};

void py_init_module_implot(py::module& m) {

  py::enum_<ImPlotFlags_>(m, "Flags", py::arithmetic(), "Options for plots.")
      // None is a keyword in Python
      .value("None_", ImPlotFlags_None, "default")
      .value("NoTitle", ImPlotFlags_NoTitle,
             "the plot title will not be displayed (titles are also hidden if "
             "preceeded by double hashes, e.g. \"##MyPlot\")")
      .value("NoLegend", ImPlotFlags_NoLegend,
             "the legend will not be displayed")
      .value("NoMenus", ImPlotFlags_NoMenus,
             "the user will not be able to open context menus with "
             "double-right click")
      .value("NoBoxSelect", ImPlotFlags_NoBoxSelect,
             "the user will not be able to box-select with right-mouse")
      .value("NoMousePos", ImPlotFlags_NoMousePos,
             "the mouse position, in plot coordinates, will not be displayed "
             "inside of the plot")
      .value("NoHighlight", ImPlotFlags_NoHighlight,
             "plot items will not be highlighted when their legend entry is "
             "hovered")
      .value("NoChild", ImPlotFlags_NoChild,
             "a child window region will not be used to capture mouse scroll "
             "(can boost performance for single ImGui window applications)")
      .value("Equal", ImPlotFlags_Equal,
             "primary x and y axes will be constrained to have the same "
             "units/pixel (does not apply to auxiliary y axes)")
      .value("YAxis2", ImPlotFlags_YAxis2,
             "enable a 2nd y-axis on the right side")
      .value("YAxis3", ImPlotFlags_YAxis3,
             "enable a 3rd y-axis on the right side")
      .value("Query", ImPlotFlags_Query,
             "the user will be able to draw query rects with middle-mouse")
      .value("Crosshairs", ImPlotFlags_Crosshairs,
             "the default mouse cursor will be replaced with a crosshair when "
             "hovered")
      .value("AntiAliased", ImPlotFlags_AntiAliased,
             "plot lines will be software anti-aliased (not recommended for "
             "density plots, prefer MSAA)")
      .value("CanvasOnly", ImPlotFlags_CanvasOnly);

  py::enum_<ImPlotAxisFlags_>(m, "AxisFlags", py::arithmetic(),
                              "Options for plot axes (X and Y).")
      // None is a keyword in Python
      .value("None_", ImPlotAxisFlags_None, "default")
      .value("NoGridLines", ImPlotAxisFlags_NoGridLines,
             "no grid lines will be displayed")
      .value("NoTickMarks", ImPlotAxisFlags_NoTickMarks,
             "no tick marks will be displayed")
      .value("NoTickLabels", ImPlotAxisFlags_NoTickLabels,
             "no text labels will be displayed")
      .value("LogScale", ImPlotAxisFlags_LogScale,
             "a logartithmic (base 10) axis scale will be used (mutually "
             "exclusive with ImPlotAxisFlags_Time)")
      .value("Time", ImPlotAxisFlags_Time,
             "axis will display date/time formatted labels (mutually exclusive "
             "with ImPlotAxisFlags_LogScale)")
      .value("Invert", ImPlotAxisFlags_Invert, "the axis will be inverted")
      .value("LockMin", ImPlotAxisFlags_LockMin,
             "the axis minimum value will be locked when panning/zooming")
      .value("LockMax", ImPlotAxisFlags_LockMax,
             "the axis maximum value will be locked when panning/zooming")
      .value("Lock", ImPlotAxisFlags_Lock)
      .value("NoDecorations", ImPlotAxisFlags_NoDecorations);

  py::enum_<ImPlotCol_>(m, "Color", "Plot styling colors.")
      // item styling colors
      .value("Line", ImPlotCol_Line,
             "plot line/outline color (defaults to next unused color in "
             "current colormap)")
      .value("Fill", ImPlotCol_Fill,
             "plot fill color for bars (defaults to the current line color)")
      .value("MarkerOutline", ImPlotCol_MarkerOutline,
             "marker outline color (defaults to the current line color)")
      .value("MarkerFill", ImPlotCol_MarkerFill,
             "marker fill color (defaults to the current line color)")
      .value("ErrorBar", ImPlotCol_ErrorBar,
             "error bar color (defaults to ImGuiCol_Text)")
      // plot styling colors
      .value("FrameBg", ImPlotCol_FrameBg,
             "plot frame background color (defaults to ImGuiCol_FrameBg)")
      .value("PlotBg", ImPlotCol_PlotBg,
             "plot area background color (defaults to ImGuiCol_WindowBg)")
      .value("PlotBorder", ImPlotCol_PlotBorder,
             "plot area border color (defaults to ImGuiCol_Border)")
      .value("LegendBg", ImPlotCol_LegendBg,
             "legend background color (defaults to ImGuiCol_PopupBg)")
      .value("LegendBorder", ImPlotCol_LegendBorder,
             "legend border color (defaults to ImPlotCol_PlotBorder)")
      .value("LegendText", ImPlotCol_LegendText,
             "legend text color (defaults to ImPlotCol_InlayText)")
      .value("TitleText", ImPlotCol_TitleText,
             "plot title text color (defaults to ImGuiCol_Text)")
      .value(
          "InlayText", ImPlotCol_InlayText,
          "color of text appearing inside of plots (defaults to ImGuiCol_Text)")
      .value("XAxis", ImPlotCol_XAxis,
             "x-axis label and tick lables color (defaults to ImGuiCol_Text)")
      .value("XAxisGrid", ImPlotCol_XAxisGrid,
             "x-axis grid color (defaults to 25% ImPlotCol_XAxis)")
      .value("YAxis", ImPlotCol_YAxis,
             "y-axis label and tick labels color (defaults to ImGuiCol_Text)")
      .value("YAxisGrid", ImPlotCol_YAxisGrid,
             "y-axis grid color (defaults to 25% ImPlotCol_YAxis)")
      .value(
          "YAxis2", ImPlotCol_YAxis2,
          "2nd y-axis label and tick labels color (defaults to ImGuiCol_Text)")
      .value("YAxisGrid2", ImPlotCol_YAxisGrid2,
             "2nd y-axis grid/label color (defaults to 25% ImPlotCol_YAxis2)")
      .value(
          "YAxis3", ImPlotCol_YAxis3,
          "3rd y-axis label and tick labels color (defaults to ImGuiCol_Text)")
      .value("YAxisGrid3", ImPlotCol_YAxisGrid3,
             "3rd y-axis grid/label color (defaults to 25% ImPlotCol_YAxis3)")
      .value("Selection", ImPlotCol_Selection,
             "box-selection color (defaults to yellow)")
      .value("Query", ImPlotCol_Query, "box-query color (defaults to green)")
      .value("Crosshairs", ImPlotCol_Crosshairs,
             "crosshairs color (defaults to ImPlotCol_PlotBorder)");

  py::enum_<ImPlotStyleVar_>(m, "StyleVar", "Plot styling variables.")
      // item styling variables
      .value("LineWeight", ImPlotStyleVar_LineWeight,
             "float,  plot item line weight in pixels")
      .value("Marker", ImPlotStyleVar_Marker, "int,    marker specification")
      .value("MarkerSize", ImPlotStyleVar_MarkerSize,
             "float,  marker size in pixels (roughly the marker's \"radius\")")
      .value("MarkerWeight", ImPlotStyleVar_MarkerWeight,
             "float,  plot outline weight of markers in pixels")
      .value("FillAlpha", ImPlotStyleVar_FillAlpha,
             "float,  alpha modifier applied to all plot item fills")
      .value("ErrorBarSize", ImPlotStyleVar_ErrorBarSize,
             "float,  error bar whisker width in pixels")
      .value("ErrorBarWeight", ImPlotStyleVar_ErrorBarWeight,
             "float,  error bar whisker weight in pixels")
      .value("DigitalBitHeight", ImPlotStyleVar_DigitalBitHeight,
             "float,  digital channels bit height (at 1) in pixels")
      .value("DigitalBitGap", ImPlotStyleVar_DigitalBitGap,
             "float,  digital channels bit padding gap in pixels")
      // plot styling variables
      .value("PlotBorderSize", ImPlotStyleVar_PlotBorderSize,
             "float,  thickness of border around plot area")
      .value("MinorAlpha", ImPlotStyleVar_MinorAlpha,
             "float,  alpha multiplier applied to minor axis grid lines")
      .value("MajorTickLen", ImPlotStyleVar_MajorTickLen,
             "ImVec2, major tick lengths for X and Y axes")
      .value("MinorTickLen", ImPlotStyleVar_MinorTickLen,
             "ImVec2, minor tick lengths for X and Y axes")
      .value("MajorTickSize", ImPlotStyleVar_MajorTickSize,
             "ImVec2, line thickness of major ticks")
      .value("MinorTickSize", ImPlotStyleVar_MinorTickSize,
             "ImVec2, line thickness of minor ticks")
      .value("MajorGridSize", ImPlotStyleVar_MajorGridSize,
             "ImVec2, line thickness of major grid lines")
      .value("MinorGridSize", ImPlotStyleVar_MinorGridSize,
             "ImVec2, line thickness of minor grid lines")
      .value("PlotPadding", ImPlotStyleVar_PlotPadding,
             "ImVec2, padding between widget frame and plot area and/or labels")
      .value("LabelPadding", ImPlotStyleVar_LabelPadding,
             "ImVec2, padding between axes labels, tick labels, and plot edge")
      .value("LegendPadding", ImPlotStyleVar_LegendPadding,
             "ImVec2, legend padding from top-left of plot")
      .value("LegendInnerPadding", ImPlotStyleVar_LegendInnerPadding,
             "ImVec2, legend inner padding from legend edges")
      .value("LegendSpacing", ImPlotStyleVar_LegendSpacing,
             "ImVec2, spacing between legend entries")
      .value("MousePosPadding", ImPlotStyleVar_MousePosPadding,
             "ImVec2, padding between plot edge and interior info text")
      .value("AnnotationPadding", ImPlotStyleVar_AnnotationPadding,
             "ImVec2, text padding around annotation labels")
      .value(
          "PlotDefaultSize", ImPlotStyleVar_PlotDefaultSize,
          "ImVec2, default size used when ImVec2(0,0) is passed to BeginPlot")
      .value("PlotMinSize", ImPlotStyleVar_PlotMinSize,
             "ImVec2, minimum size plot frame can be when shrunk");

  py::enum_<ImPlotMarker_>(m, "Marker", "Marker specifications.")
      // None is a keyword in Python
      .value("None_", ImPlotMarker_None, "no marker")
      .value("Circle", ImPlotMarker_Circle, "a circle marker")
      .value("Square", ImPlotMarker_Square, "a square maker")
      .value("Diamond", ImPlotMarker_Diamond, "a diamond marker")
      .value("Up", ImPlotMarker_Up, "an upward-pointing triangle marker")
      .value("Down", ImPlotMarker_Down, "an downward-pointing triangle marker")
      .value("Left", ImPlotMarker_Left, "an leftward-pointing triangle marker")
      .value("Right", ImPlotMarker_Right,
             "an rightward-pointing triangle marker")
      .value("Cross", ImPlotMarker_Cross, "a cross marker (not fillable)")
      .value("Plus", ImPlotMarker_Plus, "a plus marker (not fillable)")
      .value("Asterisk", ImPlotMarker_Asterisk,
             "a asterisk marker (not fillable)");

  py::enum_<ImPlotColormap_>(m, "Colormap", "Built-in colormaps")
      .value("Default", ImPlotColormap_Default,
             "ImPlot default colormap         (n=10)")
      .value("Deep", ImPlotColormap_Deep,
             "a.k.a. seaborn deep             (n=10)")
      .value("Dark", ImPlotColormap_Dark,
             "a.k.a. matplotlib \"Set1\"        (n=9)")
      .value("Pastel", ImPlotColormap_Pastel,
             "a.k.a. matplotlib \"Pastel1\"     (n=9)")
      .value("Paired", ImPlotColormap_Paired,
             "a.k.a. matplotlib \"Paired\"      (n=12)")
      .value("Viridis", ImPlotColormap_Viridis,
             "a.k.a. matplotlib \"viridis\"     (n=11)")
      .value("Plasma", ImPlotColormap_Plasma,
             "a.k.a. matplotlib \"plasma\"      (n=11)")
      .value("Hot", ImPlotColormap_Hot,
             "a.k.a. matplotlib/MATLAB \"hot\"  (n=11)")
      .value("Cool", ImPlotColormap_Cool,
             "a.k.a. matplotlib/MATLAB \"cool\" (n=11)")
      .value("Pink", ImPlotColormap_Pink,
             "a.k.a. matplotlib/MATLAB \"pink\" (n=11)")
      .value("Jet", ImPlotColormap_Jet,
             "a.k.a. MATLAB \"jet\"             (n=11)");

  py::enum_<ImPlotLocation_>(
      m, "Location",
      "Used to position items on a plot (e.g. legends, labels, etc.)")
      .value("Center", ImPlotLocation_Center, "center-center")
      .value("North", ImPlotLocation_North, "top-center")
      .value("South", ImPlotLocation_South, "bottom-center")
      .value("West", ImPlotLocation_West, "center-left")
      .value("East", ImPlotLocation_East, "center-right")
      .value("NorthWest", ImPlotLocation_NorthWest, "top-left")
      .value("NorthEast", ImPlotLocation_NorthEast, "top-right")
      .value("SouthWest", ImPlotLocation_SouthWest, "bottom-left")
      .value("SouthEast", ImPlotLocation_SouthEast, "bottom-right");

  py::enum_<ImPlotOrientation_>(
      m, "Orientation",
      "Used to orient items on a plot (e.g. legends, labels, etc.)")
      .value("Horizontal", ImPlotOrientation_Horizontal, "left/right")
      .value("Vertical", ImPlotOrientation_Vertical, "up/down");

  py::enum_<ImPlotYAxis_>(m, "YAxis", "Enums for different y-axes.")
      .value("y1", ImPlotYAxis_1, "left (default)")
      .value("y2", ImPlotYAxis_2, "first on right side")
      .value("y3", ImPlotYAxis_3, "second on right side");

  py::class_<ImPlotPoint>(m, "Point",
                          "Double precision version of ImVec2 used by ImPlot.")
      .def(py::init<>())
      .def(py::init<double, double>(), py::arg("x"), py::arg("y"))
      .def(py::init<const ImVec2&>(), py::arg("p"))
      .def_readwrite("x", &ImPlotPoint::x)
      .def_readwrite("y", &ImPlotPoint::y);

  py::class_<ImPlotRange>(
      m, "Range",
      "A range defined by a min/max value. Used for plot axes ranges.")
      .def(py::init<>())
      .def(py::init<double, double>(), py::arg("Min"), py::arg("Max"))
      .def_readwrite("min", &ImPlotRange::Min)
      .def_readwrite("max", &ImPlotRange::Max)
      .def("contains", &ImPlotRange::Contains, py::arg("value"))
      .def("size", &ImPlotRange::Size);

  py::class_<ImPlotLimits>(m, "Limits",
                           "Combination of two ranges for X and Y axes.")
      .def(py::init<>())
      .def_readwrite("x", &ImPlotLimits::X)
      .def_readwrite("y", &ImPlotLimits::Y)
      .def(
          "contains",
          [](ImPlotLimits& self, ImPlotPoint& p) { return self.Contains(p); },
          py::arg("p"))
      .def(
          "contains",
          [](ImPlotLimits& self, double x, double y) {
            return self.Contains(x, y);
          },
          py::arg("x"), py::arg("y"));

  py::class_<ImPlotStyle, leaked_ptr<ImPlotStyle>>(m, "Style",
                                                   "Plot style structure")
      .def(py::init())
      // item styling variables
      .def_readwrite("line_weight", &ImPlotStyle::LineWeight,
                     "item line weight in pixels")
      .def_readwrite("marker", &ImPlotStyle::Marker, "marker specification")
      .def_readwrite("marker_size", &ImPlotStyle::MarkerSize,
                     "marker size in pixels (roughly the marker's \"radius\")")
      .def_readwrite("marker_weight", &ImPlotStyle::MarkerWeight,
                     "outline weight of markers in pixels")
      .def_readwrite("fill_alpha", &ImPlotStyle::FillAlpha,
                     "alpha modifier applied to plot fills")
      .def_readwrite("error_bar_size", &ImPlotStyle::ErrorBarSize,
                     "error bar whisker width in pixels")
      .def_readwrite("error_bar_weight", &ImPlotStyle::ErrorBarWeight,
                     "error bar whisker weight in pixels")
      .def_readwrite("digital_bit_height", &ImPlotStyle::DigitalBitHeight,
                     "digital channels bit height (at y = 1.0f) in pixels")
      .def_readwrite("digital_bit_gap", &ImPlotStyle::DigitalBitGap,
                     "digital channels bit padding gap in pixels")
      // plot styling variables
      .def_readwrite("plot_border_size", &ImPlotStyle::PlotBorderSize,
                     "line thickness of border around plot area")
      .def_readwrite("minor_alpha", &ImPlotStyle::MinorAlpha,
                     "alpha multiplier applied to minor axis grid lines")
      .def_readwrite("major_tick_len", &ImPlotStyle::MajorTickLen,
                     "major tick lengths for X and Y axes")
      .def_readwrite("minor_tick_len", &ImPlotStyle::MinorTickLen,
                     "minor tick lengths for X and Y axes")
      .def_readwrite("major_tick_size", &ImPlotStyle::MajorTickSize,
                     "line thickness of major ticks")
      .def_readwrite("minor_tick_size", &ImPlotStyle::MinorTickSize,
                     "line thickness of minor ticks")
      .def_readwrite("major_grid_size", &ImPlotStyle::MajorGridSize,
                     "line thickness of major grid lines")
      .def_readwrite("minor_grid_size", &ImPlotStyle::MinorGridSize,
                     "line thickness of minor grid lines")
      .def_readwrite("plot_padding", &ImPlotStyle::PlotPadding,
                     "padding between widget frame and plot area, labels, or "
                     "outside legends (i.e. main padding)")
      .def_readwrite("label_padding", &ImPlotStyle::LabelPadding,
                     "padding between axes labels, tick labels, and plot edge")
      .def_readwrite("legend_padding", &ImPlotStyle::LegendPadding,
                     "legend padding from plot edges")
      .def_readwrite("legend_inner_padding", &ImPlotStyle::LegendInnerPadding,
                     "legend inner padding from legend edges")
      .def_readwrite("legend_spacing", &ImPlotStyle::LegendSpacing,
                     "spacing between legend entries")
      .def_readwrite(
          "mouse_pos_padding", &ImPlotStyle::MousePosPadding,
          "padding between plot edge and interior mouse location text")
      .def_readwrite("annotation_padding", &ImPlotStyle::AnnotationPadding,
                     "text padding around annotation labels")
      .def_readwrite(
          "plot_default_size", &ImPlotStyle::PlotDefaultSize,
          "default size used when ImVec2(0,0) is passed to BeginPlot")
      .def_readwrite("plot_min_size", &ImPlotStyle::PlotMinSize,
                     "minimum size plot frame can be when shrunk")
      // colors
      .def("get_color",
           [](ImPlotStyle& self, ImPlotCol idx) {
             if (idx < 0 || idx >= ImPlotCol_COUNT) {
               throw std::out_of_range("Illegal implot.Col value.");
             }
             return self.Colors[idx];
           })
      .def("set_color",
           [](ImPlotStyle& self, ImPlotCol idx, ImVec4 c) {
             if (idx < 0 || idx >= ImPlotCol_COUNT) {
               throw std::out_of_range("Illegal implot.Col value.");
             }
             self.Colors[idx] = c;
           })
      // settings/flags
      .def_readwrite("anti_aliased_lines", &ImPlotStyle::AntiAliasedLines,
                     "enable global anti-aliasing on plot lines (overrides "
                     "ImPlotFlags_AntiAliased)")
      .def_readwrite("use_local_time", &ImPlotStyle::UseLocalTime,
                     "axis labels will be formatted for your timezone when "
                     "ImPlotAxisFlag_Time is enabled")
      .def_readwrite("use_iso8601", &ImPlotStyle::UseISO8601,
                     "dates will be formatted according to ISO 8601 where "
                     "applicable (e.g. YYYY-MM-DD, YYYY-MM, --MM-DD, etc.)")
      .def_readwrite("use_24_hour_clock", &ImPlotStyle::Use24HourClock,
                     "times will be formatted using a 24 hour clock");

  // TODO ImPlotInputMap is not available for now.

  m.def("begin_plot", &ImPlot::BeginPlot, py::arg("title_id"),
        py::arg("x_label") = nullptr, py::arg("y_label") = nullptr,
        py::arg("size") = ImVec2(-1, 0), py::arg("flags") = ImPlotFlags_None,
        py::arg("x_flags") = ImPlotAxisFlags_None,
        py::arg("y_flags") = ImPlotAxisFlags_None,
        py::arg("y2_flags") = ImPlotAxisFlags_NoGridLines,
        py::arg("y3_flags") = ImPlotAxisFlags_NoGridLines,
        "Starts a 2D plotting context. If this function returns true, "
        "EndPlot() must be called, e.g. \"if (BeginPlot(...)) { ... EndPlot(); "
        "}\". #title_id must be unique. If you need to avoid ID collisions or "
        "don't want to display a title in the plot, use double hashes (e.g. "
        "\"MyPlot##Hidden\" or \"##NoTitle\"). If #x_label and/or #y_label are "
        "provided, axes labels will be displayed.");
  m.def("end_plot", &ImPlot::EndPlot,
        "Only call EndPlot() if BeginPlot() returns true! Typically called at "
        "the end of an if statement conditioned on BeginPlot().");

  //---------------------------------------------------------------------------
  // Plot Items
  //---------------------------------------------------------------------------

  m.def(
      "plot_line",
      [](const char* label_id, const py::buffer& values) {
        auto value_getter = ValueGetter(values);
        py::gil_scoped_release release;
        ImPlot::PlotLineG(label_id, value_getter.get_getter_func(),
                          &value_getter, value_getter.count());
      },
      py::arg("label_id"), py::arg("values"), "Plots a standard 2D line plot.");
  m.def(
      "plot_line",
      [](const char* label_id, const py::buffer& xs, const py::buffer& ys) {
        auto value_getter = ValueGetter(xs, ys);
        py::gil_scoped_release release;
        ImPlot::PlotLineG(label_id, value_getter.get_getter_func(),
                          &value_getter, value_getter.count());
      },
      py::arg("label_id"), py::arg("xs"), py::arg("ys"),
      "Plots a standard 2D line plot.");

  m.def(
      "plot_scatter",
      [](const char* label_id, const py::buffer& values) {
        auto value_getter = ValueGetter(values);
        py::gil_scoped_release release;
        ImPlot::PlotScatterG(label_id, value_getter.get_getter_func(),
                             &value_getter, value_getter.count());
      },
      py::arg("label_id"), py::arg("values"),
      "Plots a standard 2D scatter plot. Default marker is "
      "ImPlotMarker_Circle.");
  m.def(
      "plot_scatter",
      [](const char* label_id, const py::buffer& xs, const py::buffer& ys) {
        auto value_getter = ValueGetter(xs, ys);
        py::gil_scoped_release release;
        ImPlot::PlotScatterG(label_id, value_getter.get_getter_func(),
                             &value_getter, value_getter.count());
      },
      py::arg("label_id"), py::arg("xs"), py::arg("ys"),
      "Plots a standard 2D scatter plot. Default marker is "
      "ImPlotMarker_Circle.");

  m.def(
      "plot_stairs",
      [](const char* label_id, const py::buffer& values) {
        auto value_getter = ValueGetter(values);
        py::gil_scoped_release release;
        ImPlot::PlotStairsG(label_id, value_getter.get_getter_func(),
                            &value_getter, value_getter.count());
      },
      py::arg("label_id"), py::arg("values"),
      "Plots a a stairstep graph. The y value is continued constantly from "
      "every x position, i.e. the interval [x[i], x[i+1]) has the value y[i].");
  m.def(
      "plot_stairs",
      [](const char* label_id, const py::buffer& xs, const py::buffer& ys) {
        auto value_getter = ValueGetter(xs, ys);
        py::gil_scoped_release release;
        ImPlot::PlotStairsG(label_id, value_getter.get_getter_func(),
                            &value_getter, value_getter.count());
      },
      py::arg("label_id"), py::arg("xs"), py::arg("ys"),
      "Plots a a stairstep graph. The y value is continued constantly from "
      "every x position, i.e. the interval [x[i], x[i+1]) has the value y[i].");

  m.def(
      "plot_shaded",
      [](const char* label_id, const py::buffer& xs, const py::buffer& ys1,
         const py::buffer& ys2) {
        // TODO
        throw std::runtime_error("not implemented");
      },
      py::arg("label_id"), py::arg("xs"), py::arg("ys1"), py::arg("ys2"),
      "Plots a shaded (filled) region between two lines, or a line and a "
      "horizontal reference.");
  m.def(
      "plot_shaded",
      [](const char* label_id, const py::buffer& xs, const py::buffer& ys,
         double y_ref) {
        // TODO
        throw std::runtime_error("not implemented");
      },
      py::arg("label_id"), py::arg("xs"), py::arg("ys"), py::arg("y_ref") = 0.0,
      "Plots a shaded (filled) region between two lines, or a line and a "
      "horizontal reference.");

  m.def(
      "plot_bars",
      [](const char* label_id, const py::buffer& values, double width) {
        auto value_getter = ValueGetter(values);
        py::gil_scoped_release release;
        ImPlot::PlotBarsG(label_id, value_getter.get_getter_func(),
                          &value_getter, value_getter.count(), width);
      },
      py::arg("label_id"), py::arg("values"), py::arg("width") = 0.67,
      "Plots a vertical bar graph. #width and #shift are in X units.");

  m.def(
      "plot_bars_h",
      [](const char* label_id, const py::buffer& values, double height) {
        auto value_getter = ValueGetter(values);
        py::gil_scoped_release release;
        ImPlot::PlotBarsHG(label_id, value_getter.get_getter_func(),
                           &value_getter, value_getter.count(), height);
      },
      py::arg("label_id"), py::arg("values"), py::arg("height") = 0.67,
      "Plots a horizontal bar graph. #height and #shift are in Y units.");

  m.def(
      "plot_error_bars",
      [](const char* label_id, const py::buffer& xs, const py::buffer& ys,
         const py::buffer& neg, const py::buffer& pos) {
        // TODO implement ValueGetter for 4 components
        throw std::runtime_error("not implemented");
      },
      py::arg("label_id"), py::arg("xs"), py::arg("ys"), py::arg("neg"),
      py::arg("pos"),
      "Plots vertical error bar. The label_id should be the same as the "
      "label_id of the associated line or bar plot.");
  m.def(
      "plot_error_bars_h",
      [](const char* label_id, const py::buffer& xs, const py::buffer& ys,
         const py::buffer& neg, const py::buffer& pos) {
        // TODO implement ValueGetter for 4 components
        throw std::runtime_error("not implemented");
      },
      py::arg("label_id"), py::arg("xs"), py::arg("ys"), py::arg("neg"),
      py::arg("pos"),
      "Plots horizontal error bars. The label_id should be the same as the "
      "label_id of the associated line or bar plot.");

  m.def(
      "plot_stems",
      [](const char* label_id, const py::buffer& values) {
        // TODO no getter (G) version
        throw std::runtime_error("not implemented");
      },
      py::arg("label_id"), py::arg("values"), "Plots vertical stems.");
  m.def(
      "plot_stems",
      [](const char* label_id, const py::buffer& xs, const py::buffer& ys) {
        // TODO no getter (G) version
        throw std::runtime_error("not implemented");
      },
      py::arg("label_id"), py::arg("xs"), py::arg("ys"),
      "Plots vertical stems.");

  m.def(
      "plot_pie_chart",
      [](const std::vector<const char*>& label_ids, const py::buffer& values,
         double x, double y, double radius, bool normalize,
         const char* label_fmt, double angle0) {
        // TODO ImPlot::PlotPieChart has no getter overload
        throw std::runtime_error("not implemented");
      },
      py::arg("label_ids"), py::arg("values"), py::arg("x"), py::arg("y"),
      py::arg("radius"), py::arg("normalize") = false,
      py::arg("label_fmt") = "%.1f", py::arg("angle0") = 90,
      "Plots a pie chart. If the sum of values > 1 or normalize is true, each "
      "value will be normalized. Center and radius are in plot units. "
      "#label_fmt can be set to NULL for no labels.");

  m.def(
      "plot_heatmap",
      []() {
        // TODO
        throw std::runtime_error("not implemented");
      },
      "Plots a 2D heatmap chart. Values are expected to be in row-major order. "
      "#label_fmt can be set to NULL for no labels.");

  m.def(
      "plot_digital",
      [](const char* label_id, const py::buffer& xs, const py::buffer& ys) {
        auto value_getter = ValueGetter(xs, ys);
        py::gil_scoped_release release;
        ImPlot::PlotDigitalG(label_id, value_getter.get_getter_func(),
                             &value_getter, value_getter.count());
      },
      py::arg("label_id"), py::arg("xs"), py::arg("ys"),
      "Plots digital data. Digital plots do not respond to y drag or zoom, and "
      "are always referenced to the bottom of the plot.");

  m.def(
      "plot_image",
      []() {
        // No ImTexture support
        throw std::runtime_error("not implemented");
      },
      "Plots an axis-aligned image. #bounds_min/bounds_max are in plot "
      "coordinatse (y-up) and #uv0/uv1 are in texture coordinates (y-down).");

  m.def("plot_text",
        py::overload_cast<const char*, double, double, bool, const ImVec2&>(
            &ImPlot::PlotText),
        py::arg("text"), py::arg("x"), py::arg("y"),
        py::arg("vertical") = false, py::arg("pixel_offset") = ImVec2(0, 0),
        "Plots a centered text label at point x,y with optional pixel offset. "
        "Text color can be changed with "
        "ImPlot::PushStyleColor(ImPlotCol_InlayText, ...).");

  m.def("plot_dummy", &ImPlot::PlotDummy, py::arg("label_id"),
        "Plots an dummy item (i.e. adds a legend entry colored by "
        "ImPlotCol_Line)");

  //-----------------------------------------------------------------------------
  // Plot Utils
  //-----------------------------------------------------------------------------

  m.def("set_next_plot_limits", &ImPlot::SetNextPlotLimits, py::arg("x_min"),
        py::arg("x_max"), py::arg("y_min"), py::arg("y_max"),
        py::arg("cond") = ImGuiCond_Once,
        "Set the axes range limits of the next plot. Call right before "
        "BeginPlot(). If ImGuiCond_Always is used, the axes limits will be "
        "locked.");
  m.def("set_next_plot_limits_x", &ImPlot::SetNextPlotLimitsX, py::arg("x_min"),
        py::arg("x_max"), py::arg("cond") = ImGuiCond_Once,
        "Set the X axis range limits of the next plot. Call right before "
        "BeginPlot(). If ImGuiCond_Always is used, the X axis limits will be "
        "locked.");
  m.def("set_next_plot_limits_y", &ImPlot::SetNextPlotLimitsY, py::arg("y_min"),
        py::arg("y_max"), py::arg("cond") = ImGuiCond_Once,
        py::arg("y_axis") = ImPlotYAxis_(0),
        "Set the Y axis range limits of the next plot. Call right before "
        "BeginPlot(). If ImGuiCond_Always is used, the Y axis limits will be "
        "locked.");
  // TODO m.def("link_next_plot_limits", &ImPlot::LinkNextPlotLimits);
  m.def("fit_next_plot_axes", &ImPlot::FitNextPlotAxes, py::arg("x") = true,
        py::arg("y") = true, py::arg("y2") = true, py::arg("y3") = true,
        "Fits the next plot axes to all plotted data if they are unlocked "
        "(equivalent to double-clicks).");

  // TODO SetNextPlotTicks (string ownership)

  m.def("set_plot_y_axis", &ImPlot::SetPlotYAxis, py::arg("y_axis"),
        "Select which Y axis will be used for subsequent plot elements. The "
        "default is ImPlotYAxis_1, or the first (left) Y axis. Enable 2nd and "
        "3rd axes with ImPlotFlags_YAxisX.");
  m.def("hide_next_item", &ImPlot::HideNextItem, py::arg("hidden") = true,
        py::arg("cond") = ImGuiCond_Once,
        "Hides or shows the next plot item (i.e. as if it were toggled from "
        "the legend). Use ImGuiCond_Always if you need to forcefully set this "
        "every frame.");

  m.def(
      "pixels_to_plot",
      py::overload_cast<const ImVec2&, int>(&ImPlot::PixelsToPlot),
      py::arg("pix"), py::arg("y_axis") = IMPLOT_AUTO,
      "Convert pixels to a position in the current plot's coordinate system. A "
      "negative y_axis uses the current value of SetPlotYAxis (ImPlotYAxis_1 "
      "initially).");
  m.def(
      "pixels_to_plot",
      py::overload_cast<float, float, int>(&ImPlot::PixelsToPlot), py::arg("x"),
      py::arg("y"), py::arg("y_axis") = IMPLOT_AUTO,
      "Convert pixels to a position in the current plot's coordinate system. A "
      "negative y_axis uses the current value of SetPlotYAxis (ImPlotYAxis_1 "
      "initially).");
  m.def(
      "plot_to_pixels",
      py::overload_cast<const ImPlotPoint&, int>(&ImPlot::PlotToPixels),
      py::arg("plt"), py::arg("y_axis") = IMPLOT_AUTO,
      "Convert a position in the current plot's coordinate system to pixels. A "
      "negative y_axis uses the current value of SetPlotYAxis (ImPlotYAxis_1 "
      "initially).");
  m.def(
      "plot_to_pixels",
      py::overload_cast<double, double, int>(&ImPlot::PlotToPixels),
      py::arg("x"), py::arg("y"), py::arg("y_axis") = IMPLOT_AUTO,
      "Convert a position in the current plot's coordinate system to pixels. A "
      "negative y_axis uses the current value of SetPlotYAxis (ImPlotYAxis_1 "
      "initially).");
  m.def("get_plot_pos", &ImPlot::GetPlotPos,
        "Get the current Plot position (top-left) in pixels.");
  m.def("get_plot_size", &ImPlot::GetPlotSize,
        "Get the curent Plot size in pixels.");
  m.def("is_plot_hovered", &ImPlot::IsPlotHovered,
        "Returns true if the plot area in the current plot is hovered.");
  m.def("is_plot_x_axis_hovered", &ImPlot::IsPlotXAxisHovered,
        "Returns true if the XAxis plot area in the current plot is hovered.");
  m.def(
      "is_plot_y_axis_hovered", &ImPlot::IsPlotYAxisHovered,
      py::arg("y_axis") = ImPlotYAxis_(0),
      "Returns true if the YAxis[n] plot area in the current plot is hovered.");
  m.def("get_plot_mouse_pos", &ImPlot::GetPlotMousePos,
        py::arg("y_axis") = IMPLOT_AUTO,
        "Returns the mouse position in x,y coordinates of the current plot. A "
        "negative y_axis uses the current value of SetPlotYAxis (ImPlotYAxis_1 "
        "initially).");
  m.def("get_plot_limits", &ImPlot::GetPlotLimits,
        py::arg("y_axis") = IMPLOT_AUTO,
        "Returns the current plot axis range. A negative y_axis uses the "
        "current value of SetPlotYAxis (ImPlotYAxis_1 initially).");

  m.def("is_plot_queried", &ImPlot::IsPlotQueried,
        "Returns true if the current plot is being queried. Query must be "
        "enabled with ImPlotFlags_Query.");
  m.def("get_plot_query", &ImPlot::GetPlotQuery,
        py::arg("y_axis") = IMPLOT_AUTO,
        "Returns the current plot query bounds. Query must be enabled with "
        "ImPlotFlags_Query.");

  //----------------------------------------------------------------------------
  // Plot Tools
  //----------------------------------------------------------------------------

  m.def(
      "annotate",
      [](double x, double y, const ImVec2& pix_offset, const char* annotation) {
        ImPlot::Annotate(x, y, pix_offset, "%s", annotation);
      },
      py::arg("x"), py::arg("y"), py::arg("pix_offset"), py::arg("annotation"),
      "Shows an annotation callout at a chosen point.");
  m.def(
      "annotate",
      [](double x, double y, const ImVec2& pix_offset, const ImVec4& color,
         const char* annotation) {
        ImPlot::Annotate(x, y, pix_offset, color, "%s", annotation);
      },
      py::arg("x"), py::arg("y"), py::arg("pix_offset"), py::arg("color"),
      py::arg("annotation"), "Shows an annotation callout at a chosen point.");
  m.def(
      "annotate_clamped",
      [](double x, double y, const ImVec2& pix_offset, const char* annotation) {
        ImPlot::AnnotateClamped(x, y, pix_offset, "%s", annotation);
      },
      py::arg("x"), py::arg("y"), py::arg("pix_offset"), py::arg("annotation"),
      "Shows an annotation callout at a chosen point always be clamped to stay "
      "inside the plot area.");
  m.def(
      "annotate_clamped",
      [](double x, double y, const ImVec2& pix_offset, const ImVec4& color,
         const char* annotation) {
        ImPlot::AnnotateClamped(x, y, pix_offset, color, "%s", annotation);
      },
      py::arg("x"), py::arg("y"), py::arg("pix_offset"), py::arg("color"),
      py::arg("annotation"),
      "Shows an annotation callout at a chosen point always be clamped to stay "
      "inside the plot area.");

  m.def(
      "drag_line_x",
      [](const char* id, Double& x_value, bool show_label, const ImVec4& col,
         float thickness) {
        return ImPlot::DragLineX(id, &x_value.value, show_label, col,
                                 thickness);
      },
      py::arg("id"), py::arg("x_value"), py::arg("show_label") = true,
      py::arg("col") = IMPLOT_AUTO_COL, py::arg("thickness") = 1.0f,
      "Shows a draggable vertical guide line at an x-value. #col defaults to "
      "ImGuiCol_Text.");
  m.def(
      "drag_line_y",
      [](const char* id, Double& y_value, bool show_label, const ImVec4& col,
         float thickness) {
        return ImPlot::DragLineY(id, &y_value.value, show_label, col,
                                 thickness);
      },
      py::arg("id"), py::arg("y_value"), py::arg("show_label") = true,
      py::arg("col") = IMPLOT_AUTO_COL, py::arg("thickness") = 1.0f,
      "Shows a draggable horizontal guide line at a y-value. #col defaults to "
      "ImGuiCol_Text.");
  ;
  m.def(
      "drag_point",
      [](const char* id, Double& x, Double& y, bool show_label,
         const ImVec4& col, float radius) {
        return ImPlot::DragPoint(id, &x.value, &y.value, show_label, col,
                                 radius);
      },
      py::arg("id"), py::arg("x"), py::arg("y"), py::arg("show_label") = true,
      py::arg("col") = IMPLOT_AUTO_COL, py::arg("radius") = 4.0f,
      "Shows a draggable point at x,y. #col defaults to ImGuiCol_Text.");

  //----------------------------------------------------------------------------
  // Legend Utils and Tools
  //----------------------------------------------------------------------------

  // The following functions MUST be called between Begin/EndPlot!
  m.def(
      "set_legend_location",
      [](ImPlotLocation_ location, ImPlotOrientation_ orientation,
         bool outside) {
        ImPlot::SetLegendLocation(location, orientation, outside);
      },
      py::arg("location"), py::arg("orientation") = ImPlotOrientation_Vertical,
      py::arg("outside") = false,
      "Set the location of the current plot's legend.");
  m.def(
      "set_mouse_pos_location",
      [](ImPlotLocation_ location) { ImPlot::SetMousePosLocation(location); },
      py::arg("location"),
      "Set the location of the current plot's mouse position text (default = "
      "South|East).");
  m.def("is_legend_entry_hovered", &ImPlot::IsLegendEntryHovered,
        py::arg("label_id"),
        "Returns true if a plot item legend entry is hovered.");
  m.def("begin_legend_drag_drop_source", &ImPlot::BeginLegendDragDropSource,
        py::arg("label_id"), py::arg("flags") = ImGuiDragDropFlags(0),
        "Begin a drag and drop source from a legend entry. The only supported "
        "flag is SourceNoPreviewTooltip");
  m.def("end_legend_drag_drop_source", &ImPlot::EndLegendDragDropSource,
        "End legend drag and drop source.");
  m.def("begin_legend_popup", &ImPlot::BeginLegendPopup, py::arg("label_id"),
        py::arg("mouse_button") = ImGuiMouseButton(1),
        "Begin a popup for a legend entry.");
  m.def("end_legend_popup", &ImPlot::EndLegendPopup,
        "End a popup for a legend entry.");

  //----------------------------------------------------------------------------
  // Plot and Item Styling
  //----------------------------------------------------------------------------

  m.def(
      "get_style",
      []() -> leaked_ptr<ImPlotStyle> {
        return leaked_ptr<ImPlotStyle>(&ImPlot::GetStyle());
      },
      "Provides access to plot style structure for permanant modifications to "
      "colors, sizes, etc.");

  m.def("style_colors_auto", &ImPlot::StyleColorsAuto,
        py::arg("style") = nullptr,
        "Style colors for current ImGui style (default).");
  m.def("style_colors_classic", &ImPlot::StyleColorsClassic,
        py::arg("style") = nullptr, "Style colors for ImGui \"Classic\".");
  m.def("style_colors_dark", &ImPlot::StyleColorsDark,
        py::arg("style") = nullptr, "Style colors for ImGui \"Dark\".");
  m.def("style_colors_light", &ImPlot::StyleColorsLight,
        py::arg("style") = nullptr, "Style colors for ImGui \"Light\".");

  m.def("push_style_color",
        py::overload_cast<ImPlotCol, const ImVec4&>(&ImPlot::PushStyleColor),
        py::arg("idx"), py::arg("col"),
        "Temporarily modify a plot color. Don't forget to call PopStyleColor!");
  m.def("push_style_color",
        py::overload_cast<ImPlotCol, ImU32>(&ImPlot::PushStyleColor),
        py::arg("idx"), py::arg("col"),
        "Temporarily modify a plot color. Don't forget to call PopStyleColor!");
  m.def("pop_style_color", &ImPlot::PopStyleColor, py::arg("count") = 1,
        "Undo temporary color modification. Undo multiple pushes at once by "
        "increasing count.");

  m.def("push_style_var",
        py::overload_cast<ImPlotStyleVar, float>(&ImPlot::PushStyleVar),
        py::arg("idx"), py::arg("val"),
        "Temporarily modify a style variable of float type. Don't forget to "
        "call PopStyleVar!");
  m.def("push_style_var",
        py::overload_cast<ImPlotStyleVar, int>(&ImPlot::PushStyleVar),
        py::arg("idx"), py::arg("val"),
        "Temporarily modify a style variable of int type. Don't forget to call "
        "PopStyleVar!");
  m.def("push_style_var",
        py::overload_cast<ImPlotStyleVar, const ImVec2&>(&ImPlot::PushStyleVar),
        py::arg("idx"), py::arg("val"),
        "Temporarily modify a style variable of ImVec2 type. Don't forget to "
        "call PopStyleVar!");
  m.def("pop_style_var", &ImPlot::PopStyleVar, py::arg("count") = 1,
        "Undo temporary style modification. Undo multiple pushes at once by "
        "increasing count.");

  m.def("set_next_line_style", &ImPlot::SetNextLineStyle,
        py::arg("col") = IMPLOT_AUTO_COL, py::arg("weight") = IMPLOT_AUTO,
        "Set the line color and weight for the next item only.");
  m.def("set_next_fill_style", &ImPlot::SetNextFillStyle,
        py::arg("col") = IMPLOT_AUTO_COL, py::arg("alpha_mod") = IMPLOT_AUTO,
        "Set the fill color for the next item only.");
  m.def("set_next_marker_style", &ImPlot::SetNextMarkerStyle,
        py::arg("marker") = IMPLOT_AUTO, py::arg("size") = IMPLOT_AUTO,
        py::arg("fill") = IMPLOT_AUTO_COL, py::arg("weight") = IMPLOT_AUTO,
        py::arg("outline") = IMPLOT_AUTO_COL,
        "et the marker style for the next item only.");
  m.def("set_next_error_bar_style", &ImPlot::SetNextErrorBarStyle,
        py::arg("col") = IMPLOT_AUTO_COL, py::arg("size") = IMPLOT_AUTO,
        py::arg("weight") = IMPLOT_AUTO,
        "Set the error bar style for the next item only.");

  m.def("get_last_item_color", &ImPlot::GetLastItemColor,
        "Gets the last item primary color (i.e. its legend icon color)");

  m.def("get_style_color_name", &ImPlot::GetStyleColorName, py::arg("idx"),
        "Returns the null terminated string name for an ImPlotCol.");
  m.def("get_marker_name", &ImPlot::GetMarkerName, py::arg("idx"),
        "Returns the null terminated string name for an ImPlotMarker.");

  //----------------------------------------------------------------------------
  // Colormaps
  //----------------------------------------------------------------------------

  m.def(
      "push_colormap",
      [](ImPlotColormap colormap) { ImPlot::PushColormap(colormap); },
      py::arg("colormap"),
      "Temporarily switch to one of the built-in colormaps.");
  m.def(
      "push_colormap",
      [](const std::vector<ImVec4>& colormap) {
        ImPlot::PushColormap(colormap.data(),
                             static_cast<int>(colormap.size()));
      },
      py::arg("colormap"),
      "Temporarily switch to your custom colormap. The pointer data must "
      "persist until the matching call to PopColormap!");
  m.def("pop_colormap", &ImPlot::PopColormap, py::arg("count") = 1,
        "Undo temporary colormap modification.");

  m.def(
      "set_colormap",
      [](const std::vector<ImVec4>& colors) {
        ImPlot::SetColormap(colors.data(), static_cast<int>(colors.size()));
      },
      py::arg("colors"),
      "Permanently sets a custom colormap. The colors will be copied to "
      "internal memory. Prefer PushColormap instead of calling this each "
      "frame.");
  m.def(
      "set_colormap",
      [](ImPlotColormap colormap, int samples) {
        ImPlot::SetColormap(colormap, samples);
      },
      py::arg("colormap"), py::arg("samples") = 0,
      "Permanently switch to one of the built-in colormaps. If samples is "
      "greater than 1, the map will be linearly resampled. Don't call this "
      "each frame.");

  m.def("get_colormap_size", &ImPlot::GetColormapSize,
        "Returns the size of the current colormap.");
  m.def("get_colormap_color", &ImPlot::GetColormapColor,
        "Returns a color from the Color map given an index >= 0 (modulo will "
        "be performed).");
  m.def("lerp_colormap", &ImPlot::LerpColormap, py::arg("t"),
        "Linearly interpolates a color from the current colormap given t "
        "between 0 and 1.");
  m.def("next_colormap_color", &ImPlot::NextColormapColor,
        "Returns the next unused colormap color and advances the colormap. Can "
        "be used to skip colors if desired.");

  m.def("show_colormap_scale", &ImPlot::ShowColormapScale, py::arg("scale_min"),
        py::arg("scale_max"), py::arg("height"),
        "Renders a vertical color scale using the current color map. Call this "
        "outside of Begin/EndPlot.");

  m.def("get_colormap_name", &ImPlot::GetColormapName, py::arg("colormap"),
        "Returns a null terminated string name for a built-in colormap.");

  //---------------------------------------------------------------------------
  // Miscellaneous
  //---------------------------------------------------------------------------

  // TODO GetInputMap()
  // GetPlotDrawList() is unsupported since DrawList is unsupported for now
  m.def("push_plot_clip_rect", &ImPlot::PushPlotClipRect,
        "Push clip rect for rendering to current plot area.");
  m.def("pop_plot_clip_rect", &ImPlot::PopPlotClipRect, "Pop plot clip rect.");

  m.def("show_style_selector", &ImPlot::ShowStyleSelector, py::arg("label"),
        "Shows ImPlot style selector dropdown menu.");
  m.def("show_colormap_selector", &ImPlot::ShowColormapSelector,
        py::arg("label"), "Shows ImPlot colormap selector dropdown menu.");
  m.def("show_style_editor", &ImPlot::ShowStyleEditor,
        py::arg("style") = nullptr,
        "Shows ImPlot style editor block (not a window).");
  m.def("show_user_guide", &ImPlot::ShowUserGuide,
        "Add basic help/info block (not a window): how to manipulate ImPlot as "
        "an end-user.");
  m.def(
      "show_metrics_window",
      [](Bool& popen) {
        ImPlot::ShowMetricsWindow(popen.null ? nullptr : &popen.value);
      },
      py::arg("popen") = null);

  //---------------------------------------------------------------------------
  // Demo
  //---------------------------------------------------------------------------

  // m.def("show_demo_window", py::overload_cast<>(&ImPlot::ShowDemoWindow));
  // m.def("show_demo_window", []() { ImPlot::ShowDemoWindow(); });
  m.def(
      "show_demo_window",
      [](Bool& open) {
        ImPlot::ShowDemoWindow((bool*)(open.null ? nullptr : &open.value));
      },
      py::arg("open") = null,
      "Shows the ImPlot demo. Pass the current ImGui context if ImPlot is a "
      "DLL.");
}
