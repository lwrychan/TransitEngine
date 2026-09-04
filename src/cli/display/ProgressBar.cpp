#include <cmath>
#include <iostream>
#include <string>

#include "cli/Terminal.hpp"
#include "cli/display/ProgressBar.hpp"

namespace cli
{
namespace display
{
  ProgressBar::ProgressBar(int maxWidth, std::string rightText, std::string topText)
      : maxWidth(maxWidth), rightText(rightText), topText(topText)
  {
  }
  int ProgressBar::getMaxWidth()
  {
    return this->maxWidth;
  }
  int ProgressBar::getCurrentWidth()
  {
    return this->currentWidth;
  }
  void ProgressBar::setProgress(double progress)
  {
    this->progress = progress;
    this->currentWidth = std::round(this->maxWidth * progress);
  }

  void ProgressBar::print()
  {
    if (this->topText != "")
    {
      std::cout << Terminal::CLEAR_SCREEN << Terminal::CURSOR_START << topText << std::endl;
    }

    std::cout << Terminal::CLEAR_LINE << Terminal::CURSOR_START;

    std::cout << "[";

    int filled = this->currentWidth;
    int empty = this->maxWidth - this->currentWidth;

    for (int i = 0; i < filled; i++)
    {
      std::cout << "█";
    }
    for (int i = 0; i < empty; i++)
    {
      std::cout << "░";
    }

    std::cout << "]  ";
    std::cout << progress * 100 << "%";

    if (this->rightText != "")
    {
      std::cout << " || " << rightText;
    }
    std::cout << std::flush;
  }
} // namespace display
} // namespace cli