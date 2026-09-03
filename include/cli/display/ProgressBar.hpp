#pragma once

namespace cli
{
namespace display
{
    class ProgressBar
    {
    public:
        ProgressBar(int maxWidth = 3, std::string rightText = "", std::string topText = "");

        int getMaxWidth();
        int getCurrentWidth();

        void setProgress(double progress);
        void setRightText(std::string& text);
        void setTopText(std::string& text);

        void print();

        void finishStaticPrint();

    private:
        double progress;
        int currentWidth;
        int maxWidth;
        std::string rightText;
        std::string topText;
    };
} // namespace display
} // namespace cli