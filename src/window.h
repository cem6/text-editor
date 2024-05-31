#ifndef WINDOW_H
#define WINDOW_H

#include "settings.h"
#include "cursor.h"

// TODO: mousepointer (set cursor, select), ui
// maybe keyboard and cursor should have their own class

class Window {
private:
    sf::View view;
    sf::Font font;
    sf::Text text;
    std::vector<std::string> &textVec; // i dont think this is a good idea

    Cursor cursor;
    sf::RectangleShape cursorShape;
    sf::Clock clock;
    sf::Time cursorMoveDelay = sf::milliseconds(10);

public:
    sf::RenderWindow self;
    int SCREEN_WIDTH = 800;
    int SCREEN_HEIGHT = 600;
    int TEXTSIZE = 18;

    Window(std::vector<std::string> &vec) : textVec{vec} {
        std::cout << "window constructor" << std::endl;

        self.create(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "sfml fenster", sf::Style::Default);
        view.reset(sf::FloatRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));

        if (!font.loadFromFile("res/JetBrainsMonoNerdFont-Regular.ttf"))
            std::cerr << "font loading error" << std::endl;
        text.setFont(font);
        text.setCharacterSize(TEXTSIZE);
        text.setFillColor(sf::Color::White);
        updateDisplayText(textVec);
        // text.setPosition(0, 0);
        text.setLineSpacing(1);

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
        }
    }
    
    // text is stored in vector<string> but is displayed in pure string form
    void updateDisplayText(const std::vector<std::string> &vec) {
        std::string s = vec[0];
        for (int i = 1; i < vec.size(); i++)
            s += '\n' + vec[i];
        text.setString(s);
    }

    void render() {
        self.clear(sf::Color::Black);
        self.setView(view);  // should propably be done in resize function, maybe?
        self.draw(cursorShape);
        self.draw(text);
        self.display();
    }

private:
    void resizeWindow(int width, int height) {
        SCREEN_WIDTH = width;
        SCREEN_HEIGHT = height;
        // resize view, else text will be transformed
        view.reset(sf::FloatRect(0, 0, width, height));
        updateCursorShape();
    }

    void scroll(int delta) {
        // horizontal scroll
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
            if (delta == -1) text.move(-20, 0);
            if (delta == 1 && text.getPosition().x < 0) text.move(20, 0);
        }
        // vertical scroll
        else {
            if (delta == -1) text.move(0, -20);
            if (delta == 1 && text.getPosition().y < 0) text.move(0, 20);
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
            // if cursor it at end of line, move next line to end of this line (if next line exists)
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
            // zoom in
            if (c == '=' && text.getCharacterSize() < 96) {
                TEXTSIZE++;
                text.setCharacterSize(TEXTSIZE);
            }
            // zoom out
            if (c == '-' && text.getCharacterSize() > 6) {
                TEXTSIZE--;
                text.setCharacterSize(TEXTSIZE);
            }
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
        updateDisplayText(textVec);
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
            
        std::cout << cursor.x << " " << cursor.y << std::endl;
        clock.restart();
        updateCursorShape();
    }

/* -------------------- chatgpt code alert -------------------- */
    void updateCursorShape() {
        float x = text.getPosition().x + text.findCharacterPos(cursor.x + getTextOffset()).x;
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
/* -------------------- chatgpt code alert -------------------- */

};

#endif