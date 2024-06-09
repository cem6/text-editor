#include "window.h"
#include "filehandling.h"

// leftclick doesnt work after horizontal scroll
// zooming scrolls ?
//
// keyboardSelect() and mouseSelect() in Cursor can be 1 function
// updateKeyboardSelectionShape() and updateMouseSelectionShape() in Window can be 1 function
//
// selection cant be modified when shift isnt released (weird behavior in: handleTextEntered() handleKeyPressed())
// 
// TODO: move selected, copy / paste, scrollbars

int main() {
    Filehandling file("readme.txt");
    Window window;

    while (window.self.isOpen()) {
        window.handleEvents();
        window.render();
    }

    file.updateFile();
    return 0;
}