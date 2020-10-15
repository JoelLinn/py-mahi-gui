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

import mahi_gui
from mahi_gui import imgui
import numpy as np

def redirect_pyglet():
    import pyglet
    # Do not create a hidden window and opengl context
    pyglet.options['shadow_window'] = False
    # Trick pyglet into using an external context
    import pyglet.gl
    ctx = pyglet.gl.Context(None)
    ctx.canvas = pyglet.canvas.base.Canvas(None)
    ctx.set_current()

class MahiRat(mahi_gui.Application):
    def __init__(self):
        conf = mahi_gui.Application.Config()
        conf.width = 800
        conf.height = 800
        conf.title = "Mahi-Gui with ratcave scene renderer"
        # pyglet uses legacy OpenGL features so we need to enable them
        # https://pyglet.readthedocs.io/en/latest/programming_guide/context.html
        conf.gl_forward_compat = False
        super(MahiRat, self).__init__(conf)
        redirect_pyglet()
        imgui.get_io().ini_filename = None
        imgui.disable_viewports()
        self._load_scene()

    def _update(self):
        self._update_imgui()
        self._update_scene()

    def _draw_opengl(self):
        import ratcave as rc
        with rc.default_shader:
            self._scene.draw()

    def _load_scene(self):
        # ratcave instantly creates gl resources so we can't import earlier
        import ratcave as rc
        # Insert filename into WavefrontReader.
        obj_filename = rc.resources.obj_primitives
        obj_reader = rc.WavefrontReader(obj_filename)
        # Create Mesh
        self._monkey = obj_reader.get_mesh("Monkey", scale=.7)
        self._monkey.position.xyz = 0, 0, 0
        self._monkey.uniforms['diffuse'] = 0., 0., 1.
        self._monkey.uniforms['spec_weight'] = 300.
        self._torus = obj_reader.get_mesh("TorusSmooth")
        self._torus.position.xyz = 0, 0, 0
        self._torus.rotation.x = 20
        self._torus.uniforms['diffuse'] = 1., 0., 0.
        self._torus.uniforms['spec_weight'] = 300.
        # Create Scene
        self._scene = rc.Scene(meshes=[self._monkey, self._torus])
        self._scene.light.position.z = 3
        self._scene.light.position.x = 3
        # Prebuild shader
        with rc.default_shader:
            pass
        # mutables
        self._torus_xpos = imgui.Float(0)
        self._cam_angle = imgui.Float(0)
        self._cam_dist = imgui.Float(3)

    def _update_imgui(self):
        imgui.set_next_window_size(imgui.Vec2(350, -1), imgui.Condition.Once)
        imgui.set_next_window_pos(imgui.Vec2(10,10), imgui.Condition.Once)
        imgui.begin("Settings")
        if(imgui.button("VSync On")):
            self.set_vsync(True)
        imgui.same_line()
        if(imgui.button("VSync Off")):
            self.set_vsync(False)
        imgui.text("{:.3f} FPS".format(imgui.get_io().framerate))
        imgui.spacing()

        imgui.slider_float("Torus x shift", self._torus_xpos, -3, 3)
        imgui.spacing()

        imgui.slider_angle("Camera rotation", self._cam_angle, -180, 180)
        imgui.slider_float("Camera distance", self._cam_dist, 0, 5)

        imgui.end()

    def _update_scene(self):
        # Torus position shift
        self._torus.position.z = self._torus_xpos.value
        # Monkey rotation
        self._monkey.rotation.y += 0.05 * self.delta_time() / 1000.
        # Camera rotation on a circle around the origin
        angle = self._cam_angle.value
        dist = self._cam_dist.value
        self._scene.camera.position.x = dist * np.sin(angle)
        self._scene.camera.position.z = dist * np.cos(angle)
        self._scene.camera.look_at(0, 0, 0)

if __name__ == "__main__":
    app = MahiRat()
    app.run()
