#include "window.h"
#include "filehandling.h"

// cmake --build .

// ISSUES:
// leftclick doesnt work after horizontal scroll
// horizontal scroll moves cursor
// zooming scrolls ?
// cant execute ./editor whithout arguments
//
// ???
// selection cant be modified when shift isnt released (weird behavior in: handleTextEntered() handleKeyPressed())
// 
// TODOTODO: better updateSelectionShapes logic (get start only once, stick to chars (mouse), ...)
// TODO: move selected, copy / paste, line break, scrollbars 

int main(int argc, char *argv[]) {
    Filehandling file(argv[1]);
    Window window;
    window.file = &file;

    while (window.self.isOpen()) {
        window.handleEvents();
        window.render();
    }

    return 0;
}
