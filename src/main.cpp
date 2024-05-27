
#include <SFML/Graphics/Color.hpp>
#include <iostream>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>


#define SCREEN_WIDTH 600
# define SCREEN_HEIGHT 600

int main() {
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "sfml fenster", sf::Style::Default);

    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            switch (e.type) {
                case sf::Event::Closed: window.close();
                    break;
                case sf::Event::TextEntered: std::cout << e.text.unicode << std::endl;
                    break;
            }
        }

        window.clear(sf::Color(0, 0, 0));
        window.display();
    }
}