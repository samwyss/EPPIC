#ifndef CORE_EMENGINE_H
#define CORE_EMENGINE_H

#include <expected>
#include <stdexcept>
#include <string>

#include "config.h"
#include "vector.h"

class EMEngine {
public:
  /*!
   * EMEngine static factory method
   * @param config configuration structure
   * @return
   */
  static std::expected<EMEngine, std::string> create(const Config &config);

  /*!
   * EMEngine destructor
   */
  ~EMEngine() = default;

private:
  /*!
   * EMEngine constructor
   * @param config configuration structure
   */
  explicit EMEngine(const Config &config);

  /// (V/m) electric field vector
  Vector3<double> e;

  /// (A/m) magnetic field vector
  Vector3<double> h;

  /// (s) elapsed time
  double time = 0.0;
};

#endif // CORE_EMENGINE_H
