/*
Copyright (c) 2017-2018 Stanislav Pidhorskyi
Copyright (c) 2019-2020 Joel Linn

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _IMGUI_HELPER_H
#define _IMGUI_HELPER_H

struct Bool {
  Bool() : value(false) {}
  Bool(bool v) : value(v) {}

  bool value;
  bool null = false;
};

static const Bool null = []() -> Bool {
  Bool b;
  b.null = true;
  return b;
}();

struct Float {
  Float() : value(0.0f) {}
  Float(float v) : value(v) {}

  float value;
};

struct Double {
  Double() : value(0.0) {}
  Double(double v) : value(v) {}

  double value;
};

struct Int {
  Int() : value(0) {}
  Int(int v) : value(v) {}

  int value;
};

struct String {
  String() : value("") {}
  String(const std::string& v) : value(v) {}

  std::string value;
};

#endif
