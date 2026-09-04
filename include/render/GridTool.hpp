#pragma once

namespace render
{
enum class Tool
{
  Pointer,
  Node,
  Route,
  Geometry
};

constexpr const char* toolName(Tool tool)
{
  switch (tool)
  {
  case Tool::Pointer:
    return "Pointer";
  case Tool::Node:
    return "Node";
  case Tool::Route:
    return "Route";
  case Tool::Geometry:
    return "Geometry";
  }
  return "Unknown";
}
} // namespace render
