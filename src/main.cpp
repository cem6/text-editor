#include "settings.h"

#include "window.h"
#include "filehandling.h"

int main() {
    Filehandling file("readme.txt");
    Window window(file.textVec);

    while (window.self.isOpen()) {
        window.handleEvents();
        window.render();
    }

    // file.updateFile();
    return 0;
}