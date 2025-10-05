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

#ifndef CORE_IO_H
#define CORE_IO_H

#include <hdf5.h>
#include <utility>

/*!
 * RAII HDF5 object wrapper
 * @tparam CloseFunc function that closes HDF5 object
 */
template <typename CloseFunc> class HDF5Mgr {
public:
  /*!
   * HDF5 object wrapper constructor
   * @param handle HDF5 object handle
   * @param release HDF5 object release function
   */
  HDF5Mgr(const hid_t handle, const CloseFunc release) : handle(handle), close(release) {}

  /*!
   * HDF5 object wrapper default constructor
   */
  HDF5Mgr() : handle(-1), close(nullptr) {};

  /*!
   * HDF5 object wrapper destructor
   */
  ~HDF5Mgr() noexcept {
    if (handle >= 0) {
      close(handle);
    }
  }

  /*!
   * deleted HDF5 object wrapper copy constructor
   */
  HDF5Mgr(const HDF5Mgr &) = delete;

  /*!
   * deleted HDF5 object wrapper copy assignment operator
   */
  HDF5Mgr &operator=(const HDF5Mgr &) = delete;

  /*!
   * HDF5 object wrapper move constructor
   * @param other other HDF5 object wrapper
   */
  HDF5Mgr(HDF5Mgr &&other) noexcept : handle(std::exchange(other.handle, -1)), close(std::move(other.close)) {};

  /*!
   * HDF5 object wrapper move assignment operator
   * @param other other HDF5 object wrapper
   * @return HDF5 object wrapper
   */
  HDF5Mgr &operator=(HDF5Mgr &&other) noexcept {
    if (this != &other) {
      if (handle >= 0) {
        close(handle);
      }

      handle = std::exchange(other.handle, -1);
      close = std::move(other.close);
    }
    return *this;
  }

  /*!
   * HDF5 object handle getter
   * @return HDF5 object handle
   */
  [[nodiscard]] hid_t get() const noexcept { return handle; }

private:
  /// HDF5 object handle
  hid_t handle;

  /// HDF5 object release function
  CloseFunc close;
};

/// type alias for HDF5Mgr
using HDF5Obj = HDF5Mgr<herr_t (*)(hid_t)>;

/*!
 * dataspace container for writable data
 */
struct Dataspaces {
  /// dataspace for scalar data
  HDF5Obj scalar;

  /// dataspace for electric field data
  HDF5Obj e;

  /// dataspace for magnetic field data
  HDF5Obj h;
};

/*!
 * dataset container for writable data
 */
struct Datasets {
  /// time dataspace
  HDF5Obj time;

  /// step dataspace
  HDF5Obj step;

  /// electric field x-component dataset
  HDF5Obj ex;

  /// electric field y-component dataset
  HDF5Obj ey;

  /// electric field z-component dataset
  HDF5Obj ez;

  /// magnetic field x-component dataset
  HDF5Obj hx;

  /// magnetic field y-component dataset
  HDF5Obj hy;

  /// magnetic field z-component dataset
  HDF5Obj hz;
};

#endif // CORE_IO_H
