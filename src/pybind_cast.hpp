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

#ifndef _PYBIND_CAST_HPP
#define _PYBIND_CAST_HPP

#include <Mahi/Util/Timing/Time.hpp>
#include <pybind11/pybind11.h>

namespace pybind11 {
namespace detail {

// Cast mahi::util::Time as a 64 bit signed integer
template <> class type_caster<mahi::util::Time> {
  using py_type = int64_t;
  using util_type = mahi::util::int64;
  static_assert(sizeof(py_type) == sizeof(util_type));
  static_assert(std::is_signed<util_type>::value);

public:
  bool load(handle src, bool convert) {
    py_type py_value;

    if (!src)
      return false;

    py_value = sizeof(py_type) <= sizeof(long)
                   ? (py_type)PyLong_AsLong(src.ptr())
                   : (py_type)PYBIND11_LONG_AS_LONGLONG(src.ptr());

    if ((py_value == (py_type)-1) && PyErr_Occurred()) {
      bool type_error = PyErr_ExceptionMatches(
#if PY_VERSION_HEX < 0x03000000 && !defined(PYPY_VERSION)
          PyExc_SystemError
#else
          PyExc_TypeError
#endif
      );
      PyErr_Clear();
      if (type_error && convert && PyNumber_Check(src.ptr())) {
        auto tmp = reinterpret_steal<object>(PyNumber_Long(src.ptr()));
        PyErr_Clear();
        return load(tmp, false);
      }
      return false;
    }
    value = mahi::util::microseconds(py_value);
    return true;
  }

  static handle cast(mahi::util::Time src, return_value_policy /* policy */,
                     handle /* parent */) {
    return sizeof(py_type) <= sizeof(long)
               ? PYBIND11_LONG_FROM_SIGNED((long)src.as_microseconds())
               : PyLong_FromLongLong((long long)src.as_microseconds());
  }

  PYBIND11_TYPE_CASTER(mahi::util::Time, _("mahi_util_time"));
};

} // namespace detail
} // namespace pybind11

#endif
