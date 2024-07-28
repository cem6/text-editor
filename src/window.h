#ifndef WINDOW_H
#define WINDOW_H

#include "settings.h"
#include <iostream>
#include "cursor.h"
#include "filehandling.h"

class Window {
private:
    sf::View view;
    sf::Font font;

    sf::Text lineNumbers;
    sf::RectangleShape lineNumberBar;
    std::string lineNumberStr = "";

    Cursor cursor;
    sf::RectangleShape cursorShape; // everything that is rendered can be done in window class
    std::vector<sf::RectangleShape> selectionShapes;

public:
    sf::RenderWindow self;
    Filehandling *file;

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
        for (int i = 1; i <= 999; i++) {
            std::string s = std::to_string(i);
            switch(s.size()) {
                case 2: s = " " + s; break;
                case 1: s = "  " + s; break;
                default: break;
            }
            lineNumberStr += s + '\n';
        }

        updateText();

        cursorShape.setSize(sf::Vector2f(2, TEXTSIZE));
        cursorShape.setFillColor(sf::Color(0, 200, 0));
        cursorShape.setOrigin(sf::Vector2f(0, -2));
        selectionShapes.push_back(sf::RectangleShape());
    }

    void handleEvents() {
        sf::Event e;
        while (self.pollEvent(e)) {
            if (e.type == sf::Event::Closed) self.close();
            if (e.type == sf::Event::Resized) resizeWindow(e.size.width, e.size.height);

            // keyboard
            if (e.type == sf::Event::TextEntered) handleTextEntered(e.text.unicode);
            if (e.type == sf::Event::KeyPressed) handleKeypressed(e);
            // mousewheel
            if (e.type == sf::Event::MouseWheelScrolled && e.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel)
                handleMousewheel(e.mouseWheelScroll.delta);


            // leftclick press
            if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
                cursor.resetSelection(selectionShapes); // fixes stickSelectionShapes() bug
                cursor.leftclickPressed(self, text.getPosition());
                updateCursorShape();
                // updateSelectionShape();
            }
            // leftclick release
            if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left) {
                cursor.leftclickReleased(self, text.getPosition());
                updateCursorShape();
                // updateSelectionShape();
                stickSelectionShapes();

            }
            // executes while leftclick is pressed and not released
            if (cursor.leftclick) {
                cursor.selectionEndMouse = sf::Mouse::getPosition(self);
                updateSelectionShapes();
                std::cout << "leftclick pressed\n";
            }

            // shift press
            if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::LShift) {
                cursor.shiftPressed();
            }
            // shift release
            if (e.type == sf::Event::KeyReleased && e.key.code == sf::Keyboard::LShift) {
                cursor.shiftReleased();
            }
            // executes while shift is pressed and not released
            if (cursor.shift) {
                cursor.selectionEnd = {cursor.y, cursor.x};
                updateSelectionShapes();
                std::cout << "shift pressed\n";
            }

        }
    }
    void render() {
        self.clear(sf::Color::Black); // 30, 20, 40
        self.draw(text);
        self.draw(cursorShape);
        for (const auto &sh : selectionShapes) self.draw(sh);
        self.draw(lineNumberBar);
        self.draw(lineNumbers);
        self.display();
    }

private:
    // .setString() cant be used with vector
    void updateText() {
        // there should always be an extra empty line after the last line
        if (textVec.back() != "")
            textVec.push_back("");

        // convert vector to string with '\n'
        std::string s = textVec[0];
        for (int i = 1; i < textVec.size(); i++)
            s += '\n' + textVec[i];
        text.setString(s);

        // textVec.size() == how much of lineNumberStr is rendered
        int maxLines = std::min(textVec.size(), size_t(999)); // limit to 999 lines
        int i = 0, lines = 0;
        while (lines < maxLines && i < lineNumberStr.size()) {
            if (lineNumberStr[i] == '\n') lines++;
            i++;
        }
        lineNumbers.setString(lineNumberStr.substr(0, i)); // ????
    }

    // update cursorShape pos and size
    void updateCursorShape() {
        float x = text.getPosition().x + text.findCharacterPos(cursor.x + cursor.getTextOffset()).x; // sfml OP
        float y = text.getPosition().y + cursor.y * (TEXTSIZE + TEXTSIZE / 3);
        cursorShape.setPosition(x, y);
        cursorShape.setSize(sf::Vector2f(2, TEXTSIZE));
    }

    void updateSelectionShapes() {
        selectionShapes.clear();

        sf::Vector2f startPos;
        sf::Vector2f endPos;

        if (cursor.leftclick) {
            startPos = self.mapPixelToCoords(cursor.selectionStartMouse);
            endPos = self.mapPixelToCoords(cursor.selectionEndMouse);
        }
        else if (cursor.shift) {
            startPos.x = text.getPosition().x + text.findCharacterPos(cursor.selectionStart.second + cursor.getTextOffset()).x;
            startPos.y = text.getPosition().y + cursor.selectionStart.first * (TEXTSIZE + TEXTSIZE / 3);
            endPos.x = text.getPosition().x + text.findCharacterPos(cursor.selectionEnd.second + cursor.getTextOffset()).x;
            endPos.y = text.getPosition().y + cursor.selectionEnd.first * (TEXTSIZE + TEXTSIZE / 3);
        }

        // ensure start is top left and end is bottom right
        if (startPos.y > endPos.y || (startPos.y == endPos.y && startPos.x > endPos.x))
            std::swap(startPos, endPos);

        // rectangleShape for every line
        int lineHeight = TEXTSIZE + TEXTSIZE / 3;
        int startLine = startPos.y / lineHeight;
        int endLine = endPos.y / lineHeight;
        for (int line = startLine; line <= endLine; line++) {
            sf::RectangleShape selectionShape;

            float y = line * lineHeight;
            float xStart = (line == startLine ? startPos.x : text.getPosition().x);
            // doesnt end on lines end but on same x for every line, ??? use findCharacterPos() oder so
            float xEnd = (line == endLine ? endPos.x : text.getPosition().x + text.getGlobalBounds().width);

            selectionShape.setPosition(xStart, y);
            selectionShape.setSize(sf::Vector2f(xEnd - xStart, lineHeight));
            selectionShape.setFillColor(sf::Color(100, 100, 255, 100));

            selectionShapes.push_back(selectionShape);
        }
    }
    // TEMP! for doing on updateSelectionShapes() with cursor positions after mouseSelect() to get "stick" to chars effect
    void stickSelectionShapes() {
        selectionShapes.clear();

        sf::Vector2f startPos;
        sf::Vector2f endPos;
        startPos.x = text.getPosition().x + text.findCharacterPos(cursor.selectionStart.second + cursor.getTextOffset()).x;
        startPos.y = text.getPosition().y + cursor.selectionStart.first * (TEXTSIZE + TEXTSIZE / 3);
        endPos.x = text.getPosition().x + text.findCharacterPos(cursor.selectionEnd.second + cursor.getTextOffset()).x;
        endPos.y = text.getPosition().y + cursor.selectionEnd.first * (TEXTSIZE + TEXTSIZE / 3);

        if (startPos.y > endPos.y || (startPos.y == endPos.y && startPos.x > endPos.x))
            std::swap(startPos, endPos);

        int lineHeight = TEXTSIZE + TEXTSIZE / 3;
        int startLine = startPos.y / lineHeight;
        int endLine = endPos.y / lineHeight;
        for (int line = startLine; line <= endLine; line++) {
            sf::RectangleShape selectionShape;
            float y = line * lineHeight;
            float xStart = (line == startLine ? startPos.x : text.getPosition().x);
            float xEnd = (line == endLine ? endPos.x : text.getPosition().x + text.getGlobalBounds().width);
            selectionShape.setPosition(xStart, y);
            selectionShape.setSize(sf::Vector2f(xEnd - xStart, lineHeight));
            selectionShape.setFillColor(sf::Color(100, 100, 255, 100));
            selectionShapes.push_back(selectionShape);
        }
    }

    // for everything that is not Event::TextEntered
    void handleKeypressed(const sf::Event &e) {
        // cursor movement with arrow keys
        if (e.key.code == sf::Keyboard::Right || e.key.code == sf::Keyboard::Left ||
            e.key.code == sf::Keyboard::Down || e.key.code == sf::Keyboard::Up) {
            cursor.handleMovement(e);
            if (cursor.selectionActive) cursor.resetSelection(selectionShapes);
            updateCursorShape();
        }
        // del
        else if (e.key.code == sf::Keyboard::Delete) {
            // delete selection
            if (cursor.selectionActive) {
                cursor.deleteSelection(selectionShapes);
            }
            // delete char at cursor.x + 1 (right of cursor)
            else if (cursor.x < textVec[cursor.y].size()) {
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
        // fix weird shift behavior, maybe
        if (cursor.shift) cursor.shiftReleased(); // ?????????????????????

        // input with mod keys (can best be handled in here bc = and - are are Event::TextEntered)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
            // std::cout << "ctrl pressed " << static_cast<int>(c) << '\n';
            switch (c) {
                case 19: file->updateFile(); break; // s
                case '=': zoom(+1); break;
                case '-': zoom(-1); break;
                default: break;
            }
            return; // dont want to write char that was pressed with mod key
        }

        // check if theres a selection that needs to be deleted before modifying text
        bool deletedSelection = false;
        if (cursor.selectionActive) {
            cursor.deleteSelection(selectionShapes);
            deletedSelection = true;
        }

        // backspace
        if (c == '\b' && !deletedSelection) {
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
        // tab, TODO: tab selection
        else if (c == '\t') {
            textVec[cursor.y].insert(cursor.x, "    ");
            cursor.x += 4;
        }
        // char
        else if (isprint(static_cast<unsigned char>(c))) {
            /* textVec[cursor.y] = */ textVec[cursor.y].insert(cursor.x++, 1, static_cast<char>(c)); // insert works fine
        }
        updateText();
        updateCursorShape();
    }

    void resizeWindow(int width, int height) {
        SCREEN_WIDTH = width;
        SCREEN_HEIGHT = height;

        // resize view, else text will be transformed
        view.reset(sf::FloatRect(0, 0, width, height));
        self.setView(view);
        // resize lineNumberBar and cursor
        lineNumberBar.setSize(sf::Vector2f(TEXTSIZE * 3.8, height));
        updateCursorShape();
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

        updateText(); // ?
        updateCursorShape();
        if (cursor.selectionActive) cursor.resetSelection(selectionShapes); // ?
    }

    void handleMousewheel(int delta) {
        // zoom
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
            if (delta == -1) {
                zoom(-1);
            }
            if (delta == 1) {
                zoom(+1);
            }
        }
        // horizontal scroll !!! breaks cursor !!!
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
            if (delta == -1) {
                text.move(-20, 0);
                // for (auto &sh : selectionShapes) sh.move(-20, 0);
            }
            if (delta == 1 && text.getPosition().x < 0) {
                text.move(20, 0);
                // for (auto &sh : selectionShapes) sh.move(20, 0);
            }
            updateCursorShape();
            updateSelectionShapes();
        }
        // vertical scroll
        else {
            int lineHeight = TEXTSIZE + TEXTSIZE / 3;
            int scrollHeight = 3 * lineHeight; // size of 3 lines with cur textsize
            if (delta == -1 && (text.getPosition().y - scrollHeight) * -1 < textVec.size() * lineHeight) {
                text.move(0, -scrollHeight);
                lineNumbers.move(0, -scrollHeight);
                for (auto &sh : selectionShapes) sh.move(0, -scrollHeight);
            }
            if (delta == 1 && text.getPosition().y < 0) {
                text.move(0, scrollHeight);
                lineNumbers.move(0, scrollHeight);
                for (auto &sh : selectionShapes) sh.move(0, scrollHeight);
            }
        }
        updateCursorShape();
    }

};

#endif
