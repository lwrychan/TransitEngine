#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace cli
{
class View
{
  public:
  View(std::string_view initContent);

  View(const std::vector<std::string>& initContent);

  View(int lineCount);

  // lineNumber = index + 1
  void setLine(size_t lineNumber, std::string text);
  std::string getLine(size_t lineNumber);

  int getLineCount();

  void print();

  private:
  int lineCount;
  std::vector<std::string> content;

  bool validLineNumber(size_t lineNumber);
};
} // namespace cli