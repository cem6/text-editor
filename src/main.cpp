#include "window.h"
#include "filehandling.h"

// ISSUES:
// leftclick doesnt work after horizontal scroll
// zooming scrolls ?
// horizontal scroll moves cursor
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

    while (window.self.isOpen()) {
        window.handleEvents();
        window.render();
    }

    // file.updateFile();
    return 0;
}