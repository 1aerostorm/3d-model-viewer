#include "renders/OpenGLWindow.h"
#include <FL/Fl.H>
#include <stdexcept>
#include <iostream>

int main(int argc, char** argv) {
    try {
        OpenGLWindow window(800, 600);
        window.show();
        return Fl::run();
    } catch (const std::exception& e) {
        Fl::error("Error: %s", e.what());
        return 1;
    }
}