/*
 * Copyright (C) 2025 Samuel Wyss
 *
 * This file is part of EPPIC.
 *
 * EPPIC is free software: you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * EPPIC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with EPPIC. If not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef CORE_TYPE_H
#define CORE_TYPE_H

#include <H5Tpublic.h>
#include <string>
#include <type_traits>

/// floating point type (e.g., double or float)
#if EPPIC_USE_FLOAT
using fp_t = float;
#else
using fp_t = double;
#endif

// ensure fp_t is either double or float to use correct HDF5 and MPI type aliases
static_assert(std::is_same_v<fp_t, double> || std::is_same_v<fp_t, float>);

/// unsigned integer type (e.g., uint64_t or unit32_t)
#if EPPIC_USE_UINT32_T
using ui_t = uint32_t;
#else
using ui_t = uint64_t;
#endif

/// HDF5 floating point type
template <typename T> hid_t h5_fp_t();

// ensure ui_t is either uint64_t or uint32_t to use correct HDF5 and MPI type aliases
static_assert(std::is_same_v<ui_t, uint64_t> || std::is_same_v<ui_t, uint32_t>);

/// HDF5 floating point type template specialization for double
template <> inline hid_t h5_fp_t<double>() { return H5T_NATIVE_DOUBLE; }

/// HDF5 floating point type template specialization for float
template <> inline hid_t h5_fp_t<float>() { return H5T_NATIVE_FLOAT; }

/// HDF5 unsigned integer type
template <typename T> hid_t h5_ui_t();

/// HDF5 unsigned integer type template specialization for uint64_t
template <> inline hid_t h5_ui_t<uint64_t>() { return H5T_NATIVE_UINT64; }

/// HDF5 unsigned integer type template specialization for uint32_t
template <> inline hid_t h5_ui_t<uint32_t>() { return H5T_NATIVE_UINT32; }

/*!
 * returns typename of T as a std::string
 * @tparam T type to get name of
 * @note defaults to mangled typename if demangling fails
 * @return typename of T as a std::string
 */
template <typename T> [[nodiscard]] std::string type_name() noexcept {
  SPDLOG_TRACE("enter type_name");

  using TR = std::remove_reference_t<T>;

  int status = 0;
  const std::unique_ptr<char, void (*)(void *)> name_char_arr(
      abi::__cxa_demangle(typeid(TR).name(), nullptr, nullptr, &status), std::free);

  std::string name = (status == 0) ? name_char_arr.get() : typeid(TR).name();

  if (std::is_const_v<T>) {
    name += " const";
  }

  if (std::is_volatile_v<T>) {
    name += " volatile";
  }

  if (std::is_lvalue_reference_v<T>) {
    name += "&";
  } else if (std::is_rvalue_reference_v<T>) {
    name += "&&";
  }

  SPDLOG_TRACE("exit type_name");
  return name;
}

#endif // CORE_TYPE_H
