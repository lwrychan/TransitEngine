#include "Resources.hpp"
Resources::Resources() {
#ifdef _WIN32
    this->baseDirectory = "resources/";
#elif __APPLE__
    this->baseDirectory = "../Resources/";
#endif
}