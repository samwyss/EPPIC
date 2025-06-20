#ifndef CORE_WORLD_H
#define CORE_WORLD_H

#include <expected>
#include <stdexcept>
#include <string>

class World {
public:
  static std::expected<World, std::string> create();

  ~World() = default;

  static std::expected<void, std::string> adv_by();

  static std::expected<void, std::string> adv_to();

private:
  World();
};

#endif // CORE_WORLD_H
