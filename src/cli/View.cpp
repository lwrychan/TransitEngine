#include <iostream>
#include <ranges>

#include "cli/Terminal.hpp"
#include "cli/View.hpp"

namespace cli
{
View::View(std::string_view initContent)
{
  if (initContent.empty())
  {
    lineCount = 0;
    return;
  }

  std::vector<std::string> splitContent;
  size_t start = 0;

  while (true)
  {
    size_t next = initContent.find('\n', start);
    if (next == std::string_view::npos)
    {
      splitContent.emplace_back(initContent.substr(start));
      break;
    }

    splitContent.emplace_back(initContent.substr(start, next - start));
    start = next + 1;
  }

  this->content = std::move(splitContent);
  this->lineCount = static_cast<int>(content.size());
}

View::View(const std::vector<std::string>& initContent)
{
  this->content = initContent;
  this->lineCount = static_cast<int>(content.size());
}

View::View(int lineCount)
{
  this->content.resize(lineCount);
  this->lineCount = lineCount;
}

bool View::validLineNumber(size_t lineNumber)
{
  return lineNumber > 0 && lineNumber <= this->content.size();
}

std::string View::getLine(size_t lineNumber)
{
  if (this->validLineNumber(lineNumber))
  {
    return this->content[lineNumber - 1];
  }
  return "";
}

void View::setLine(size_t lineNumber, std::string text)
{
  if (this->validLineNumber(lineNumber))
  {
    this->content[lineNumber - 1] = text;
  }
}

int View::getLineCount()
{
  return this->lineCount;
}

void View::print()
{
  for (const auto& line : content)
  {
    std::cout << cli::Terminal::CLEAR_LINE << cli::Terminal::CURSOR_START;
    std::cout << line;
    if (line.empty() || line.back() != '\n')
    {
      std::cout << '\n';
    }
  }
  std::cout << std::flush;

  if (this->lineCount > 0)
  {
    std::cout << "\033[" << lineCount << "A" << cli::Terminal::CURSOR_START;
  }
}
} // namespace cli