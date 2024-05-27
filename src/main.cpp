#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/WindowStyle.hpp>

#include <iostream>
#include <fstream>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>


#define SCREEN_WIDTH 800
# define SCREEN_HEIGHT 600

void updateFile(const std::string &content) {
    std:: ofstream outfile("src/file.txt");
    if (!outfile.is_open()) {
        std::cout << "failed to open outfile" << std::endl;
        return;
    }
    outfile << content;
    outfile.close();
}

int main() {
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "sfml fenster", sf::Style::Default);

    // read file
    std::ifstream infile("src/file.txt");
    if (!infile.is_open())
        std::cout << "failed to open infile" << std::endl;
    std::string displayString; // this string will be modified when outfile is modified
    std::string line;
    while (std::getline(infile, line))
        displayString += line + "\n";
    infile.close();
    
    // font
    sf::Font font;
    font.loadFromFile("res/JetBrainsMonoNerdFont-Regular.ttf");
    // text
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(12);
    text.setFillColor(sf::Color::White);
    text.setString(displayString);

    while (window.isOpen()) {
        sf::Event e;

        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) {
                window.close();
            }
            if (e.type == sf::Event::TextEntered) {
                if (e.text.unicode == '\b') {
                    if (!displayString.empty())
                        displayString.pop_back();
                } else if (e.text.unicode == '\r' || e.text.unicode == '\n')  {
                    displayString += "\n";
                } else {
                    char enteredChar = static_cast<char>(e.text.unicode);
                    displayString += enteredChar;
                }
                text.setString(displayString);
                updateFile(displayString);
            }
        }

        window.clear(sf::Color(0, 0, 0));
        window.draw(text);
        window.display();
    }

    std::cout << "ende" << std::endl;
}