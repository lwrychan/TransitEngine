#pragma once

#include <string>
#include <utility>

namespace network
{
// Logical endpoint in the transit topology. It deliberately has no position.
class AbstractNode
{
  public:
  AbstractNode() = default;
  explicit AbstractNode(std::string name) : name(std::move(name)) {}

  const std::string& getName() const
  {
    return name;
  }
  void setName(std::string newName)
  {
    name = std::move(newName);
  }

  private:
  std::string name;
};
} // namespace network
