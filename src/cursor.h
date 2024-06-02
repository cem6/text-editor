#ifndef CURSOR_H
#define CURSOR_H

#include "settings.h"
#include <iostream>
#include <limits>

class Cursor {
private:
    sf::Clock clock;
    sf::Time cursorMoveDelay = sf::milliseconds(10);

public:
    int x = 0;
    int y = 0;

    Cursor() {

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
        std::cout << "cursor: line: " << y << ", char: " << x << std::endl;
    }

    // mouse
    void setPosToMouse(const sf::Window &win, const sf::Vector2f &scrollOffset) {
        auto [mouseX, mouseY] = sf::Mouse::getPosition(win);
        auto [line, character] = getCursorFromMousePos(mouseX - scrollOffset.x, mouseY - scrollOffset.y); // x doesnt work
        
        y = line;
        x = character;
    }
    std::pair<int, int> getCursorFromMousePos(int x, int y) {
        // convert mouse y to line number
        int line = y / (TEXTSIZE + TEXTSIZE / 3);
        line = std::min(std::max(line, 0), (int)textVec.size() - 1); // line can only be where text is
        // convert mouse x to character pos in line
        int character = 0;
        float minDistance = std::numeric_limits<float>::max();
        for (int i = 0; i <= textVec[line].size(); i++) {
            float charX = text.findCharacterPos(i + getTextOffset()).x; // adjust character position by text's moved position
            float distance = std::abs(charX - x);
            if (distance < minDistance) {
                minDistance = distance;
                character = i;
            }
        }
        return {line, character};
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