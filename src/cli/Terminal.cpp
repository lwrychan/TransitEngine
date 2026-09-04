#include <iostream>

#include "cli/Terminal.hpp"

void cli::Terminal::moveCursorToStart()
{
  std::cout << cli::Terminal::CURSOR_START;
}

void cli::Terminal::moveCursorUp(int lineCount)
{
  std::cout << "\033[" << lineCount << "A";
}

void cli::Terminal::moveCursorDown(int lineCount)
{
  std::cout << "\033[" << lineCount << "B";
}

void cli::Terminal::clearScreen()
{
  std::cout << Terminal::CLEAR_SCREEN << Terminal::CURSOR_START << std::endl;
}

void cli::Terminal::finishStaticPrint()
{
  std::cout << std::endl;
}