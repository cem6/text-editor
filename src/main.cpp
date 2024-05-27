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

// read input file and save it to string for instant modification
std::string getFile(const std::string path) {
    std::ifstream infile(path);
    if (!infile.is_open()) {
        std::cout << "failed to open infile" << std::endl;
        return nullptr;
    }
    std::string readString;
    std::string line;
    while (std::getline(infile, line))
        readString += line + "\n";
    infile.close();

    return readString;
}

// save input string (propably modified) to file
void updateFile(const std::string path, const std::string &content) {
    std::ofstream outfile(path);
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
    std::string displayString = getFile("README.md");
    
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
                    displayString += static_cast<char>(e.text.unicode);
                }
                text.setString(displayString);
            }
        }

        window.clear(sf::Color(0, 0, 0));
        window.draw(text);
        window.display();
    }

    updateFile("README.md", displayString);
    std::cout << "ende" << std::endl;
}