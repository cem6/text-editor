#ifndef WINDOW_H
#define WINDOW_H

#include "settings.h"
#include "cursor.h"

// maybe keyboard should have its own class

class Window {
private:
    sf::View view;
    sf::Font font;
    sf::Text text;
    std::vector<std::string> &textVec; // i dont think this is a good idea
    Cursor cursor;
    sf::RectangleShape cursorShape;

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
        updateText(textVec);
        // text.setPosition(0, 0);
        text.setLineSpacing(1);

        // textsize 18: width 12, height 20
        cursorShape.setSize(sf::Vector2f(2, TEXTSIZE));
        cursorShape.setFillColor(sf::Color::Blue);
        cursorShape.setOrigin(sf::Vector2f(-10, -2));
    }

    void handleEvents() {
        sf::Event e;
        while (self.pollEvent(e)) {
            if (e.type == sf::Event::Closed)
                self.close();

            if (e.type == sf::Event::Resized)
                resize(e.size.width, e.size.height);

            if (e.type == sf::Event::MouseWheelScrolled && e.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel)
                scroll(e.mouseWheelScroll.delta);

            if (e.type == sf::Event::TextEntered)
                input(e.text.unicode);
            
            // cursor movement
            if (e.key.code == sf::Keyboard::Right && cursor.x < textVec[cursor.y].size())
                cursor.x++;
            if (e.key.code == sf::Keyboard::Left && cursor.x > 0)
                cursor.x--;
            if (e.key.code == sf::Keyboard::Down && cursor.y < textVec.size())
                cursor.y++;
            if (e.key.code == sf::Keyboard::Up && cursor.y > 0)
                cursor.y--;

            std::cout << cursor.x << " " << cursor.y << std::endl;
        }
    }
    
    // text is stored in vector<string> but is displayed in pure string form
    void updateText(const std::vector<std::string> &vec) {
        std::string s = vec[0];
        for (int i = 1; i < vec.size(); i++)
            s += '\n' + vec[i];
        text.setString(s);
    }

    void render() {
        self.clear(sf::Color::Black);
        self.setView(view);  // should propably be done in resize function
        self.draw(cursorShape);
        self.draw(text);
        self.display();
    }

private:
    void resize(int width, int height) {
        SCREEN_WIDTH = width;
        SCREEN_HEIGHT = height;

        // resize view instead of changing size of text
        view.reset(sf::FloatRect(0, 0, width, height));
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
    }

    void input(char c) {
        // input with mod keys
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
            return; // dont want to write c that was pressed wit mod key
        }

        // write input, CURSOR IS NOT YET VISIBLE
        if (c == '\b') { // backspace
            // bug: sometimes the whole line is deleted, i dont know why because i cant see the cursor and thinking is hard
            // delete only char at cursor.x
            if (!textVec[cursor.y].empty()) textVec[cursor.y].erase(cursor.x); // bug (vielleicht)
            // line is empty, delete empty line and go to line before
            else if (cursor.y != 0) textVec.erase(textVec.begin() + cursor.y--);
        }
        else if (c == '\n' || c == '\r') { // newline
            // cursor is at end of line, insert new line after this line
            if (cursor.x == textVec[cursor.y].size()) {
                std::cout << cursor.x << " " << textVec[cursor.y].size() << std::endl; 
                textVec.insert(textVec.begin() + cursor.y + 1, "");
                cursor.y++;
            }
            // cursor is in middle of line, insert everything right of cursor to new line after this line
            else {
                // new line
                textVec.insert(textVec.begin() + cursor.y + 1, textVec[cursor.y].substr(cursor.x));
                // this line
                textVec[cursor.y] = textVec[cursor.y].substr(0, cursor.x);
                // move cursor begin of new line
                cursor.y++;
                cursor.x = 0;
            }
        }
        else { // char
            // insert works for both, middle and back of string
            textVec[cursor.y] = textVec[cursor.y].insert(cursor.x++, 1, static_cast<char>(c));
            
        }
        updateText(textVec);
    }
};

#endif