#include <Mahi/Util/Timing/Time.hpp>
#include <pybind11/pybind11.h>

namespace pybind11 {
namespace detail {

// Cast mahi::util::Time as a 64 bit signed integer
template <> class type_caster<mahi::util::Time> {
  using py_type = int64_t;
  using util_type = std::result_of<decltype (
      &mahi::util::Time::as_microseconds)(mahi::util::Time)>::type;
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
