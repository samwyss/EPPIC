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
