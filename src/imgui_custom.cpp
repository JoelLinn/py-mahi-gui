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

#include <Mahi/Gui/imgui_custom.hpp>
#include <pybind11/pybind11.h>

#include "imgui_helper.hpp"

namespace py = pybind11;

void py_init_module_imgui_custom(py::module& m) {
  m.def("enable_viewports", &ImGui::EnableViewports);
  m.def("disable_viewports", &ImGui::DisableViewports);

  m.def("enable_docking", &ImGui::EnableDocking);
  m.def("disable_docking", &ImGui::DisableDocking);

  m.def("begin_fixed", &ImGui::BeginFixed);

  m.def("hover_tooltip", &ImGui::HoverTooltip);

  m.def("begin_disabled", &ImGui::BeginDisabled);
  m.def("end_disabled", &ImGui::EndDisabled);

  m.def("toggle_button", &ImGui::ToggleButton);
  m.def("button_colored", &ImGui::ButtonColored);
  m.def("mode_selector",
        [](Int& currentMode, const std::vector<std::string>& modes,
           bool horizontal) -> bool {
          return ImGui::ModeSelector(&currentMode.value, modes, horizontal);
        });

  // double is standard in python

  m.def("style_colors_mahi_dark_1", &ImGui::StyleColorsMahiDark1);
  m.def("style_colors_mahi_dark_2", &ImGui::StyleColorsMahiDark2);
  m.def("style_colors_mahi_dark_3", &ImGui::StyleColorsMahiDark3);
  m.def("style_colors_mahi_dark_4", &ImGui::StyleColorsMahiDark4);
}
