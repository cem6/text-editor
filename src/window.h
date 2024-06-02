#ifndef WINDOW_H
#define WINDOW_H

#include <limits>
#include "settings.h"
#include "cursor.h"
#include "mouse.h"

// TODOTODO: rewrite again (use classes (input, cursor, ...), window is too crowded) !
//
// TODO: mousepointer (select), copy / paste, scrollbars

class Window {
private:
    sf::View view;
    sf::Font font;
    sf::Text text;
    std::vector<std::string> &textVec; // i dont think this is a good idea
    int scrollX = 0, scrollY = 0;

    sf::Text lineNumbers;
    sf::RectangleShape lineNumberBar;

    Cursor cursor;
    sf::RectangleShape cursorShape;
    sf::Clock clock;
    sf::Time cursorMoveDelay = sf::milliseconds(10);
    Mouse mouse;

public:
    sf::RenderWindow self;
    int SCREEN_WIDTH = 800;
    int SCREEN_HEIGHT = 600;
    int TEXTSIZE = 18;

    Window(std::vector<std::string> &vec) : textVec{vec} {
        // std::cout << "window constructor" << std::endl;

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

        updateDisplayText();

        // textsize 18: width 12, height 22 (?)
        cursorShape.setSize(sf::Vector2f(2, TEXTSIZE));
        cursorShape.setFillColor(sf::Color(0, 200, 0));
        cursorShape.setOrigin(sf::Vector2f(0, -2));
    }

    void handleEvents() {
        sf::Event e;
        while (self.pollEvent(e)) {
            if (e.type == sf::Event::Closed)
                self.close();

            if (e.type == sf::Event::Resized)
                resizeWindow(e.size.width, e.size.height);

            if (e.type == sf::Event::MouseWheelScrolled && e.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel)
                scroll(e.mouseWheelScroll.delta);

            if (e.type == sf::Event::TextEntered)
                handleTextEntered(e.text.unicode);
            
            if (e.type == sf::Event::KeyPressed)
                handleKeypressed(e);

            // leftclick
            if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
                mouseCursor();
            }
            
        }
    }
    
    // text is stored in vector<string> but is displayed in pure string form
    void updateDisplayText() {
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

    void render() {
        self.clear(sf::Color::Black); // 30, 20, 40
        self.setView(view);  // should propably be done in resize function, maybe?
        self.draw(cursorShape);
        self.draw(text);
        self.draw(lineNumberBar);
        self.draw(lineNumbers);
        self.display();
    }

private:
    void resizeWindow(int width, int height) {
        SCREEN_WIDTH = width;
        SCREEN_HEIGHT = height;
        // resize view, else text will be transformed
        view.reset(sf::FloatRect(0, 0, width, height));
        lineNumberBar.setSize(sf::Vector2f(TEXTSIZE * 3.8, height));
        updateCursorShape();
    }

    void scroll(int delta) {
        // horizontal scroll
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
            if (delta == -1) {
                text.move(-20, 0);
                scrollX -= 20;
            }
            if (delta == 1 && text.getPosition().x < 0) {
                text.move(20, 0);
                scrollY += 20;
            } 
        }
        // vertical scroll
        else {
            int v = 3 * (TEXTSIZE + TEXTSIZE / 3); // size of 3 * 1line with cur textsize
            if (delta == -1) {
                text.move(0, -v);
                lineNumbers.move(0, -v);
                scrollY -= v;
            }
            if (delta == 1 && text.getPosition().y < 0) {
                text.move(0, v);
                lineNumbers.move(0, v);
                scrollY += v;
            }
        }
        updateCursorShape(); // doesnt work correctly
    }

    // for everything that is not Event::TextEntered
    void handleKeypressed(const sf::Event &e) {
        // cursor movement with arrow keys
        if (e.key.code == sf::Keyboard::Right || e.key.code == sf::Keyboard::Left ||
            e.key.code == sf::Keyboard::Down || e.key.code == sf::Keyboard::Up) {
            cursorMovement(e);
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
            // zoom
            if (c == '=' && text.getCharacterSize() < 96) TEXTSIZE++;
            if (c == '-' && text.getCharacterSize() > 6) TEXTSIZE--;

            // beautiful !
            text.setCharacterSize(TEXTSIZE);
            text.setOrigin(-TEXTSIZE * 4, 0);
            lineNumbers.setCharacterSize(TEXTSIZE);
            lineNumbers.setOrigin(-TEXTSIZE, 0);
            lineNumberBar.setSize(sf::Vector2f(TEXTSIZE * 3.8, SCREEN_HEIGHT));

            updateCursorShape();
            return; // dont want to write c that was pressed with mod key
        }

        // backspace, TODO: REFACTOR
        if (c == '\b') {
            // delete only char at cursor.x
            if (!textVec[cursor.y].empty() && cursor.x > 0)  {
                textVec[cursor.y].erase(cursor.x - 1, 1);
                cursor.x--;
            }
            // cursor it at begin of line, delete this line, append this lines text to line before
            else if (cursor.y != 0) {
                cursor.x = textVec[cursor.y - 1].size();
                textVec[cursor.y - 1] += textVec[cursor.y];
                textVec.erase(textVec.begin() + cursor.y--);
            }
            // updateCursorShape(); // not neccessary, vielleicht
        }
        // newline
        else if (c == '\n' || c == '\r') { 
            // cursor at end of line
            if (cursor.x == textVec[cursor.y].size()) {
                // insert new line after this line
                textVec.insert(textVec.begin() + cursor.y + 1, "");
            }
            // cursor in middle of line
            else {
                // insert substring on right of cursor to new line after this line 
                textVec.insert(textVec.begin() + cursor.y + 1, textVec[cursor.y].substr(cursor.x));
                // remove substring from this line
                textVec[cursor.y] = textVec[cursor.y].substr(0, cursor.x);    
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
            // insert works for both, middle and back of string
            textVec[cursor.y] = textVec[cursor.y].insert(cursor.x++, 1, static_cast<char>(c));
        }
        updateDisplayText();
        updateCursorShape();
    }

    void cursorMovement(const sf::Event &e) {
        if (clock.getElapsedTime() < cursorMoveDelay) return;

        // skip to next non alnum or begin/end of line
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
            switch (e.key.code) {
                case sf::Keyboard::Right:
                    if (cursor.x < textVec[cursor.y].size()) {
                        do { cursor.x++; } while (cursor.x < textVec[cursor.y].size() && std::isalnum(textVec[cursor.y][cursor.x]));
                    }
                    break;

                case sf::Keyboard::Left:
                    if (cursor.x > 0) {
                        cursor.x--;
                        do { cursor.x--; } while (cursor.x >= 0 && std::isalnum(textVec[cursor.y][cursor.x]));
                        cursor.x++; 
                    }
                    break;
            }
        }
        // move cursor by 1
        else {
            switch (e.key.code) {
                case sf::Keyboard::Right:
                    if (cursor.x < textVec[cursor.y].size()) cursor.x++;
                    break;
                case sf::Keyboard::Left:
                    if (cursor.x > 0) cursor.x--;
                    break;
                case sf::Keyboard::Down:
                    if (cursor.y < textVec.size() - 1) {
                        cursor.y++;
                        cursor.x = std::min(cursor.x, (int) textVec[cursor.y].size());
                    }
                    break;
                case sf::Keyboard::Up:
                    if (cursor.y > 0) {
                        cursor.y--;
                        cursor.x = std::min(cursor.x, (int) textVec[cursor.y].size());
                    }
                    break;
            }
        }
            
        clock.restart();
        updateCursorShape();
        std::cout << "cursor: line: " << cursor.y << ", char: " << cursor.x << std::endl;
    }

    // move cursor on leftclick, doesnt work after scrolling
    void mouseCursor() {
        auto [x, y] = sf::Mouse::getPosition(self);
        auto [line, character] = getCursorFromMousePos(x - scrollX, y - scrollY);
        cursor.y = line;
        cursor.x = character;

        updateCursorShape();
        std::cout << "mouse: " << x << " " << y << std::endl;

    }
    std::pair<int, int> getCursorFromMousePos(int x, int y) {
        // convert mouse y to line number
        int line = y / (TEXTSIZE + TEXTSIZE / 3);
        line = std::min(std::max(line, 0), (int) textVec.size() - 1); // line can only be where thext is

        // convert mouse x to character pos in line
        int character = 0;
        float minDistance = std::numeric_limits<float>::max();
        for (int i = 0; i <= textVec[line].size(); i++) {
            float charX = text.findCharacterPos(i + getTextOffset()).x; // sfml OP
            float distance = std::abs(charX - x);
            if (distance < minDistance) {
                minDistance = distance;
                character = i;
            }
        }

        return {line, character};
    }

    // update cursorShape pos based on cursor.x and cursor.y in text
    void updateCursorShape() {
        float x = text.getPosition().x + text.findCharacterPos(cursor.x + getTextOffset()).x; // sfml OP
        float y = text.getPosition().y + cursor.y * (TEXTSIZE + TEXTSIZE / 3);
        cursorShape.setPosition(x, y);
        cursorShape.setSize(sf::Vector2f(2, TEXTSIZE));
    }
    int getTextOffset() const {
        int offset = 0;
        for (int i = 0; i < cursor.y; ++i) {
            offset += textVec[i].length() + 1; // +1 for newline character
        }
        return offset;
    }

};

#endif