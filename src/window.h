#ifndef WINDOW_H
#define WINDOW_H

#include "settings.h"
#include <iostream>
#include "cursor.h"

// leftclick doesnt work after horizontal scroll
//
// TODO: mousepointer (select), copy / paste, scrollbars

class Window {
private:
    sf::View view;
    sf::Font font;
    // std::vector<std::string> &textVec; // i dont think this is a good idea

    sf::Text lineNumbers;
    sf::RectangleShape lineNumberBar;

    Cursor cursor;
    sf::RectangleShape cursorShape; // everything that is rendered can be done in window class

public:
    sf::RenderWindow self;

    Window() {
        self.create(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "sfml fenster", sf::Style::Default);
        view.reset(sf::FloatRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));

        if (!font.loadFromFile("res/JetBrainsMonoNerdFont-Regular.ttf"))
            std::cerr << "font loading error" << std::endl;
        text.setFont(font);
        text.setCharacterSize(TEXTSIZE);
        text.setFillColor(sf::Color::White);
        text.setOrigin(-TEXTSIZE * 4, 0);

        lineNumbers.setFont(font);
        lineNumbers.setCharacterSize(TEXTSIZE);
        lineNumbers.setFillColor(sf::Color(100, 100, 100));
        lineNumbers.setOrigin(-TEXTSIZE, 0);
        lineNumberBar.setSize(sf::Vector2f(TEXTSIZE * 3.8, SCREEN_HEIGHT));
        lineNumberBar.setFillColor(sf::Color::Black); // 20, 10, 30

        updateText();

        cursorShape.setSize(sf::Vector2f(2, TEXTSIZE));
        cursorShape.setFillColor(sf::Color(0, 200, 0));
        cursorShape.setOrigin(sf::Vector2f(0, -2));
    }

    void handleEvents() {
        sf::Event e;
        while (self.pollEvent(e)) {
            if (e.type == sf::Event::Closed) self.close();
            if (e.type == sf::Event::Resized) resizeWindow(e.size.width, e.size.height);
            
            // keyboard
            if (e.type == sf::Event::TextEntered) handleTextEntered(e.text.unicode);
            if (e.type == sf::Event::KeyPressed) handleKeypressed(e);
            // scroll
            if (e.type == sf::Event::MouseWheelScrolled && e.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel)
                scroll(e.mouseWheelScroll.delta);
            // leftclick
            if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
                cursor.setPosToMouse(self, text.getPosition());
                updateCursorRender();
            }
        }
    }
    void render() {
        self.clear(sf::Color::Black); // 30, 20, 40
        self.draw(cursorShape);
        self.draw(text);
        self.draw(lineNumberBar);
        self.draw(lineNumbers);
        self.display();
    }

private:
    // .setString() cant be used with vector
    void updateText() {
        // there should always be an extra empty line after the last line
        // updateDisplayText() propably isnt the right place for this
        if (textVec.back() != "")
            textVec.push_back("");
        
        // convert vector to string with '\n'
        std::string s = textVec[0];
        for (int i = 1; i < textVec.size(); i++)
            s += '\n' + textVec[i];
        text.setString(s);

        // VERY NICE AND GREAT im sure theres no better way
        std::string lineNumberStr = "";
        size_t maxLines = std::min(textVec.size(), size_t(999)); // limit to 999 lines
        for (size_t i = 1; i <= maxLines; i++) {
            std::string s = std::to_string(i);
            while (s.size() < 3) s = ' ' + s;
            lineNumberStr += s + '\n';
        }
        lineNumbers.setString(lineNumberStr); // ????
    }

    // update cursorShape pos based on cursor.x and cursor.y in text
    void updateCursorRender() {
        float x = text.getPosition().x + text.findCharacterPos(cursor.x + cursor.getTextOffset()).x; // sfml OP
        float y = text.getPosition().y + cursor.y * (TEXTSIZE + TEXTSIZE / 3);
        cursorShape.setPosition(x, y);
        cursorShape.setSize(sf::Vector2f(2, TEXTSIZE));
    }



    // for everything that is not Event::TextEntered
    void handleKeypressed(const sf::Event &e) {
        // cursor movement with arrow keys
        if (e.key.code == sf::Keyboard::Right || e.key.code == sf::Keyboard::Left ||
            e.key.code == sf::Keyboard::Down || e.key.code == sf::Keyboard::Up) {
            cursor.handleMovement(e);
            updateCursorRender();
        }
        // del
        else if (e.key.code == sf::Keyboard::Delete) {
            // delete char at cursor.x + 1 (right of cursor)
            if (cursor.x < textVec[cursor.y].size()) {
                textVec[cursor.y].erase(cursor.x, 1);
            }
            // if cursor is at end of line, move next line to end of this line (if next line exists)
            else if (cursor.y + 1 < textVec.size()) {
                textVec[cursor.y] += textVec[cursor.y + 1];
                textVec.erase(textVec.begin() + cursor.y + 1);
            }
        }
    }

    // for everything that is Event::TextEntered
    void handleTextEntered(char c) {
        // input with mod keys (can best be handled in here bc = and - are are Event::TextEntered)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
            switch (c) {
                case '=': zoom(+1); break;
                case '-': zoom(-1); break;

                default: break;
            }
            return; // dont want to write c that was pressed with mod key
        }

        // backspace
        if (c == '\b') {
            // cursor somewhere in line except begin
            if (!textVec[cursor.y].empty() && cursor.x > 0)  {
                textVec[cursor.y].erase(cursor.x - 1, 1); // delete char left of cursor
                cursor.x--;
            }
            // cursor it at begin of line
            else if (cursor.y != 0) {
                cursor.x = textVec[cursor.y - 1].size();     // set x to end of line before
                textVec[cursor.y - 1] += textVec[cursor.y];  // append this line to line before
                textVec.erase(textVec.begin() + cursor.y--); // delet this line
            }
        }
        // newline
        else if (c == '\n' || c == '\r') { 
            // cursor at end of line
            if (cursor.x == textVec[cursor.y].size()) {
                textVec.insert(textVec.begin() + cursor.y + 1, ""); // insert new line after this line
            }
            // cursor in middle of line
            else {
                textVec.insert(textVec.begin() + cursor.y + 1, textVec[cursor.y].substr(cursor.x)); // insert substring on right of cursor to new line after this line 
                textVec[cursor.y] = textVec[cursor.y].substr(0, cursor.x); // remove substring from this line
            }
            cursor.y++;
            cursor.x = 0;
        }
        // tab
        else if (c == '\t') {
            textVec[cursor.y].insert(cursor.x, "    ");
            cursor.x += 4;
        }
        // char
        else if (isprint(static_cast<unsigned char>(c))) { 
            textVec[cursor.y] = textVec[cursor.y].insert(cursor.x++, 1, static_cast<char>(c)); // insert works fine
        }
        updateText();
        updateCursorRender();
    }



    void resizeWindow(int width, int height) {
        SCREEN_WIDTH = width;
        SCREEN_HEIGHT = height;

        // resize view, else text will be transformed
        view.reset(sf::FloatRect(0, 0, width, height));
        self.setView(view);
        // resize lineNumberBar and cursor
        lineNumberBar.setSize(sf::Vector2f(TEXTSIZE * 3.8, height));
        updateCursorRender();
    }

    void zoom(int mode) {
        if (mode == +1 && text.getCharacterSize() < 96) TEXTSIZE++;
        if (mode == -1 && text.getCharacterSize() > 6) TEXTSIZE--;

        // beautiful !
        text.setCharacterSize(TEXTSIZE);
        text.setOrigin(-TEXTSIZE * 4, 0);
        lineNumbers.setCharacterSize(TEXTSIZE);
        lineNumbers.setOrigin(-TEXTSIZE, 0);
        lineNumberBar.setSize(sf::Vector2f(TEXTSIZE * 3.8, SCREEN_HEIGHT));

        updateCursorRender();
    }

    void scroll(int delta) {
        // horizontal scroll
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
            if (delta == -1) {
                text.move(-20, 0);
            }
            if (delta == 1 && text.getPosition().x < 0) {
                text.move(20, 0);
            } 
        }
        // vertical scroll
        else {
            int v = 3 * (TEXTSIZE + TEXTSIZE / 3); // size of 3 * 1line with cur textsize
            if (delta == -1) {
                text.move(0, -v);
                lineNumbers.move(0, -v);
            }
            if (delta == 1 && text.getPosition().y < 0) {
                text.move(0, v);
                lineNumbers.move(0, v);
            }
        }
        updateCursorRender();
    }

};

#endif