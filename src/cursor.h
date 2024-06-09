#ifndef CURSOR_H
#define CURSOR_H

#include "settings.h"
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include <limits>

class Cursor {
private:
    sf::Clock clock;
    sf::Time cursorMoveDelay = sf::milliseconds(10);
public:
    int y = 0; // line
    int x = 0; // char
    bool leftclick = false;
    bool shift = false;
    sf::Vector2i selectionStartMouse;
    sf::Vector2i selectionEndMouse;
    // array<int, 2> is propably better (no sf::Vector2i bc line,char is y,x)
    std::pair<int, int> selectionStart, selectionEnd; // {line, char}
    bool selectionActive = false;

    Cursor() = default;




    // this is very badly written
    void deleteSelection(std::vector<sf::RectangleShape> &selectionShapes) {
        // no selection
        if (selectionStart == selectionEnd) {
            resetSelection(selectionShapes);
        }
        // single line
        else if (selectionStart.first == selectionEnd.first) {
            textVec[selectionStart.first].erase(selectionStart.second, selectionEnd.second - selectionStart.second);
        }
        // multiple lines
        else {
            // delete selectionStart from startChar to end of line
            textVec[selectionStart.first].erase(selectionStart.second);
            // add last selectionEnd (beginnning from endChar) to first selectionStart
            textVec[selectionStart.first] += textVec[selectionEnd.first].substr(selectionEnd.second); 
            // delete everything after selectionStart
            textVec.erase(textVec.begin() + selectionStart.first + 1, textVec.begin() + selectionEnd.first + 1);
        }
        resetSelection(selectionShapes);
        y = selectionStart.first;
        x = selectionStart.second; 
    }
    void resetSelection(std::vector<sf::RectangleShape> &selectionShapes) {
        selectionEnd = selectionStart;
        selectionShapes.clear();
        selectionActive = false;
    }
    void copySelection() {
        // TODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODO
    }

    void leftclickPressed(const sf::Window &win, const sf::Vector2f &scrollOffset) {
        leftclick = true;
        // set cursor pos
        setPos(getMousePos(win, scrollOffset));
        // start selection
        selectionStartMouse = sf::Mouse::getPosition(win);
        selectionEndMouse = selectionStartMouse;
        std::cout << "\nleftclick pressed\n";
    }
    void leftclickReleased(const sf::Window &win, const sf::Vector2f &scrollOffset) {
        leftclick = false;
        // set cursorPos
        setPos(getMousePos(win, scrollOffset, selectionEndMouse));
        // do selection
        mouseSelect(win, scrollOffset);
        std::cout << "leftclick released" << std::endl;
    }

    void shiftPressed() {
        shift = true;
        selectionActive = true; // ??????????????????????????????
        selectionStart = {y, x};
        selectionEnd = selectionStart;
        std::cout << "\nshift pressed\n";
    }
    void shiftReleased() {
        keyboardSelect();
        shift = false;
        std::cout << "shift released" << std::endl;
        
    }

    // arrow keys
    void handleMovement(const sf::Event &e) {
        if (clock.getElapsedTime() < cursorMoveDelay) return;
        // skip to next non alnum or begin/end of line
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
            bool select = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

            // if select:
            // get cursor start and cursor end and select range

            switch (e.key.code) {
                case sf::Keyboard::Right:
                    if (x < textVec[y].size()) {
                        do { x++; } while (x < textVec[y].size() && std::isalnum(textVec[y][x]));
                    }
                    break;
                case sf::Keyboard::Left:
                    if (x > 0) {
                        x--;
                        do { x--; } while (x >= 0 && std::isalnum(textVec[y][x]));
                        x++; 
                    }
                    break;
            }
        }
        // move cursor by 1
        else {
            switch (e.key.code) {
                case sf::Keyboard::Right:
                    if (x < textVec[y].size()) x++;
                    break;
                case sf::Keyboard::Left:
                    if (x > 0) x--;
                    break;
                case sf::Keyboard::Down:
                    if (y < textVec.size() - 1) {
                        y++;
                        x = std::min(x, (int) textVec[y].size());
                    }
                    break;
                case sf::Keyboard::Up:
                    if (y > 0) {
                        y--;
                        x = std::min(x, (int) textVec[y].size());
                    }
                    break;
            }
        }
        clock.restart();
        std::cout << "cursor: line: " << y + 1 << ", char: " << x << std::endl;
    }

    ////////// should propably be done somewhere else //////////
    int getTextOffset() const {
        int offset = 0;
        for (int i = 0; i < y; ++i) {
            offset += textVec[i].length() + 1; // +1 for newline character
        }
        return offset;
    }

private:
    // set mouse pos to {line, char}
    void setPos(std::pair<int, int> p) {
        y = p.first;
        x = p.second;
    }


    // !!!!!!!!!!!! MOUSESELECT AND KEYBOARDSELECT CAN BE A SINGLE SELECT FUNCTION !!!!!!!!!!!!

    // converts selections mouse positions (start, end) to {line, char} and gets the selected text from textVec
    void mouseSelect(const sf::Window &win, const sf::Vector2f &scrollOffset) {
        // no need to execute all this when nothing was selected
        if (selectionStartMouse == selectionEndMouse) {
            std::cout << "---\nnot a selection\n---" << std::endl;
            return;
        }

        selectionActive = true;

        // get {line, char} from selectionStartMouse and selectionEndMouse (are mouse coords, not window coords)
        selectionStart = getMousePos(win, scrollOffset, selectionStartMouse);
        selectionEnd = getMousePos(win, scrollOffset, selectionEndMouse);
        
        // ensure start is top left and end is bottom right
        if (selectionStart.first > selectionEnd.first || (selectionStart.first == selectionEnd.first && selectionStart.second > selectionEnd.second)) {
            std::swap(selectionStart, selectionEnd);
        }

        auto [startLine, startChar] = selectionStart;
        auto [endLine, endChar] = selectionEnd;

        /* print selected text */
        std::cout << "---\nstart: " << startLine + 1 << " " << startChar << "\n";
        std::cout << "end: " << endLine + 1 << " " << endChar << "\n";
        if (startLine == endLine) {
            std::cout << textVec[startLine].substr(startChar, endChar - startChar) << "\n";
        }
        else {
            for (int line = startLine; line <= endLine; line ++) {
                if (line == startLine) std::cout << textVec[startLine].substr(startChar) << '\n';
                else if (line == endLine && startLine != endLine) std::cout << textVec[endLine].substr(0, endChar) << "\n---" << std::endl;
                else std::cout << textVec[line] << "\n";
            }
        }
        /* print selected text */
    }

    // converts cursor starting position and cursor ending position (keyboard movement) to selection
    void keyboardSelect() {
        // no need to execute all this when nothing was selected
        if (selectionStart == selectionEnd) {
            std::cout << "---\nnot a selection\n---" << std::endl;
            return;
        }

        selectionActive = true;

        // ensure start is top left and end is bottom right
        if (selectionStart.first > selectionEnd.first || (selectionStart.first == selectionEnd.first && selectionStart.second > selectionEnd.second)) {
            std::swap(selectionStart, selectionEnd);
        }

        auto [startLine, startChar] = selectionStart;
        auto [endLine, endChar] = selectionEnd;

        /* print selected text */
        std::cout << "---\nstart: " << startLine + 1 << " " << startChar << "\n";
        std::cout << "end: " << endLine + 1 << " " << endChar << "\n";
        if (startLine == endLine) {
            std::cout << textVec[startLine].substr(startChar, endChar - startChar) << "\n";
        }
        else {
            for (int line = startLine; line <= endLine; line ++) {
                if (line == startLine) std::cout << textVec[startLine].substr(startChar) << '\n';
                else if (line == endLine && startLine != endLine) std::cout << textVec[endLine].substr(0, endChar) << "\n---" << std::endl;
                else std::cout << textVec[line] << "\n";
            }
        }
        /* print selected text */
    }
    


    // get current mouse pos in {line, char} form
    std::pair<int, int> getMousePos(const sf::Window &win, const sf::Vector2f &scrollOffset) {
        auto [mouseX, mouseY] = sf::Mouse::getPosition(win);
        return getCursorFromMousePos(mouseX - scrollOffset.x, mouseY - scrollOffset.y);
    }
    // get saved mouse pos in {line, char} form
    std::pair<int, int> getMousePos(const sf::Window &win, const sf::Vector2f &scrollOffset, const sf::Vector2i &mousePos) {
        auto [mouseX, mouseY] = mousePos;
        return getCursorFromMousePos(mouseX - scrollOffset.x, mouseY - scrollOffset.y);
    }
    // convert mouse pos to line, char in textVec
    std::pair<int, int> getCursorFromMousePos(int mouseX, int mouseY) {
        // convert mouse y to line number
        int line = mouseY / (TEXTSIZE + TEXTSIZE / 3);
        line = std::min(std::max(line, 0), (int)textVec.size() - 1); // line can only be where text is
        // convert mouse x to character pos in line
        int character = 0;
        float minDistance = std::numeric_limits<float>::max();
        for (int i = 0; i <= textVec[line].size(); i++) {
            float charX = text.findCharacterPos(i + getTextOffset()).x; // adjust character position by text's moved position
            float distance = std::abs(charX - mouseX);
            if (distance < minDistance) {
                minDistance = distance;
                character = i;
            }
        }
        return {line, character};
    }

};

#endif