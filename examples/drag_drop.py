# Copyright 2020 Joel Linn
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software
# without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# You are under no obligation whatsoever to provide any bug fixes, patches, or
# upgrades to the features, functionality or performance of the source code
# ("Enhancements") to anyone; however, if you choose to make your Enhancements
# available either publicly, or directly to the author of this software, without
# imposing a separate written license agreement for such Enhancements, then you
# hereby grant the following license: a non-exclusive, royalty-free perpetual
# license to install, use, modify, prepare derivative works, incorporate into
# other computer software, distribute, and sublicense such enhancements or
# derivative works thereof, in binary and source code form.

from enum import Enum
import mahi_gui
from mahi_gui import imgui

class DragDropTest(mahi_gui.Application):

    def __init__(self):
        super().__init__(210, 250, "Drag&Drop test")
        imgui.get_io().ini_filename = None
        self.set_vsync(False)
        imgui.disable_viewports()

    class Mode(Enum):
        COPY = 1
        MOVE = 2
        SWAP = 3
    _mode = Mode.COPY
    _names = [
        "Bobby", "Beatrice", "Betty",
        "Brianna", "Barry", "Bernard",
        "Bibi", "Blaine", "Bryn"
    ]
    def _update(self):
        width, height = self.get_window_size()
        imgui.begin(
            "##DDTest",
            imgui.Bool(True),
            imgui.WindowFlags.NoTitleBar | imgui.WindowFlags.NoResize | imgui.WindowFlags.NoMove)
        imgui.set_window_pos(imgui.Vec2(0,0))
        imgui.set_window_size(imgui.Vec2(width, height))
        imgui.text("{:.3f} FPS".format(imgui.get_io().framerate))

        if imgui.radio_button("Copy", self._mode == self.Mode.COPY):
            self._mode = self.Mode.COPY
        imgui.same_line()
        if imgui.radio_button("Move", self._mode == self.Mode.MOVE):
            self._mode = self.Mode.MOVE
        imgui.same_line()
        if imgui.radio_button("Swap", self._mode == self.Mode.SWAP):
            self._mode = self.Mode.SWAP

        for n in range(len(self._names)):
            imgui.push_id_int(n)
            if (n % 3) != 0:
                imgui.same_line()
            imgui.button(self._names[n], imgui.Vec2(60, 60))

            # Our buttons are both drag sources and drag targets here!
            if imgui.begin_drag_drop_source(imgui.DragDropFlags.None_):
                # Set payload to carry the index of our item (could be anything)
                imgui.set_drag_drop_payload_string(str(n))

                # Display preview (could be anything, e.g. when dragging an image we could decide to display
                # the filename and a small preview of the image, etc.)
                if self._mode == self.Mode.COPY: imgui.text("Copy {}".format(self._names[n]))
                if self._mode == self.Mode.MOVE: imgui.text("Move {}".format(self._names[n]))
                if self._mode == self.Mode.SWAP: imgui.text("Swap {}".format(self._names[n]))
                imgui.end_drag_drop_source()

            if imgui.begin_drag_drop_target():
                payload = imgui.accept_drag_drop_payload_string()
                if payload:
                    payload_n = int(payload)
                    if self._mode == self.Mode.COPY:
                        self._names[n] = self._names[payload_n]
                    if self._mode == self.Mode.MOVE:
                        self._names[n] = self._names[payload_n]
                        self._names[payload_n] = ""
                    if self._mode == self.Mode.SWAP:
                        tmp = self._names[n]
                        self._names[n] = self._names[payload_n]
                        self._names[payload_n] = tmp
                imgui.end_drag_drop_target()
            imgui.pop_id()
        imgui.end()

if __name__ == "__main__":
    app = DragDropTest()
    app.run()
