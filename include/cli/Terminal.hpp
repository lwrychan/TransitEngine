namespace cli
{
    namespace Terminal
    {
        constexpr const char* CLEAR_LINE = "\033[2K";
        constexpr const char* CURSOR_HOME = "\033[H";
        constexpr const char* CURSOR_START = "\r";
        constexpr const char* DISABLE_WRAP = "\033[?7l";
        constexpr const char* ENABLE_WRAP = "\033[?7h";
        constexpr const char* CLEAR_SCREEN = "\033[2J";

        void clearScreen();

        void moveCursorToStart();

        void moveCursorUp(int lineCount);

        void moveCursorDown(int lineCount);

        void finishStaticPrint();
    }
}