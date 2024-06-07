#ifndef CURSOR_H
#define CURSOR_H

#include "settings.h"
#include <iostream>
#include <limits>

class Cursor {
public:
    int y = 0; // line
    int x = 0; // char
    bool leftclick = false;
    sf::Vector2i selectionStartMouse;
    sf::Vector2i selectionEndMouse;
    std::pair<int, int> selectionStart, selectionEnd; // {line, char}
    bool selectionActive = false;

    Cursor() = default;




    // this is very badly written
    void deleteSelection(std::vector<sf::RectangleShape> &selectionShapes) {
        // no selection
        if (selectionStart == selectionEnd) {
            resetSelection(selectionShapes);
            return;
        }

        // same line
        if (selectionStart.first == selectionEnd.first) {
            textVec[selectionStart.first].erase(selectionStart.second, selectionEnd.second - selectionStart.second);
        }
        // everything else
        else {
            for (int line = selectionStart.first; line <= selectionEnd.first; line++) {
                if (line == selectionStart.first) textVec[line].erase(selectionStart.second, textVec[line].size() - selectionStart.second);
                else if (line == selectionEnd.first) textVec[line].erase(0, selectionEnd.second);
                else textVec[line].erase();
            }
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



private:
    sf::Clock clock;
    sf::Time cursorMoveDelay = sf::milliseconds(10);

    // set mouse pos to {line, char}
    void setPos(std::pair<int, int> p) {
        y = p.first;
        x = p.second;
    }

    // converts selections mouse positions (start, end) to {line, char} and gets the selected text from textVec
    // THIS TAKES SOME TIME TO EXECUTE AND SHOULD ONLY BE CALLED ___WHEN SELECTED TEXT IS MODIFIED___
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
        // TODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODOTODO
        std::cout << "---\nstart: " << startLine + 1 << " " << startChar << "\n";
        std::cout << "end: " << endLine + 1 << " " << endChar << "\n";
        for (int line = startLine; line <= endLine; line ++) {
            if (line == startLine) std::cout << textVec[startLine].substr(startChar) << '\n';
            else if (line == endLine && startLine != endLine) std::cout << textVec[endLine].substr(0, endChar) << "\n---" << std::endl;
            else std::cout << textVec[line] << "\n";
        }
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

public:
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

    // arrow keys
    void handleMovement(const sf::Event &e) {
        if (clock.getElapsedTime() < cursorMoveDelay) return;
        // skip to next non alnum or begin/end of line
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
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

};

#endif