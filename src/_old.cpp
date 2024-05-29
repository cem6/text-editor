#include <iostream>
#include <fstream>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

// TODO: rewrite
//       better data structure for text: array of lines: left buffer + gap + right buffer
//       cursor: modifies gap
//       ui

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;
int TEXTSIZE = 18;

// read input file and save it to string for instant modification
std::pair<std::string, int> getFile(const std::string path) {
    std::ifstream infile(path);
    if (!infile.is_open()) {
        std::cout << "failed to open infile" << std::endl;
        return {};
    }
    std::string readString;
    std::string line;
    int lineCnt = 0;
    while (std::getline(infile, line)) {
        readString += line + '\n';
        lineCnt++;
    }
    infile.close();

    return {readString, lineCnt};
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

// std::string generateDisplayString(const std::vector<std::string> &vec) {
//     std::string displayString = vec[0];
//     for (int i = 1; i < vec.size(); i++)
//         displayString += '\n' + vec[i];
//     return displayString;
// }

// void insertChar(int x, int y, char c, std::vector<std::string> &vec) {
//     vec[y] = vec[y].substr(0, x) + c + vec[y].substr(x + 1, vec.size() - x);
// }

int main() {
    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "sfml fenster", sf::Style::Default);
    sf::View view(sf::FloatRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));

    // read file
    auto [displayString, y] = getFile("readme.txt");
    std::cout << "lines: " << y << std::endl;

    // font
    sf::Font font;
    font.loadFromFile("res/JetBrainsMonoNerdFont-Regular.ttf");
    // text
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(TEXTSIZE);
    text.setFillColor(sf::Color::White);
    text.setString(displayString);
    text.setOrigin(-(TEXTSIZE * 1.5 + TEXTSIZE / 5), 0); // offset for line numbers

    // ui
    // sf::RectangleShape sideBar(sf::Vector2f(50, SCREEN_HEIGHT));
    // sf::RectangleShape topBar(sf::Vector2f(SCREEN_WIDTH, 50.f));

    // line numbers
    sf::RectangleShape lineNumberBar(sf::Vector2f(TEXTSIZE * 1.5, SCREEN_HEIGHT));
    lineNumberBar.setFillColor(sf::Color(15, 5, 25));
    sf::Text lineNumbers;
    lineNumbers.setFont(font);
    lineNumbers.setCharacterSize(TEXTSIZE);
    lineNumbers.setFillColor(sf::Color(100, 100, 100));
    lineNumbers.setString(" 1\n 2\n 3\n 4\n 5\n 6\n 7\n 8\n 9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30");

    while (window.isOpen()) {
        sf::Event e;

        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) {
                window.close();
            }
            // dynamic resizing
            if (e.type == sf::Event::Resized) {
                SCREEN_WIDTH = (int) e.size.width;
                SCREEN_HEIGHT = (int) e.size.height;
                
                view.reset(sf::FloatRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
                lineNumberBar.setSize(sf::Vector2f(TEXTSIZE * 1.5, SCREEN_HEIGHT));
            }
            
            // mousewheel scroll
            if (e.type == sf::Event::MouseWheelScrolled && e.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                // horizontal scroll
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                    if (e.mouseWheelScroll.delta == -1)
                        text.move(-20, 0);
                    if (e.mouseWheelScroll.delta == 1 && text.getPosition().x < 0)
                        text.move(20, 0);
                }
                // vertical scroll
                else {
                    if (e.mouseWheelScroll.delta == -1) {
                        text.move(0, -20);
                        lineNumbers.move(0, -20);
                    }
                    if (e.mouseWheelScroll.delta == 1 && text.getPosition().y < 0) {
                        text.move(0, 20);
                        lineNumbers.move(0, 20);
                    }
                }
            }

            if (e.type == sf::Event::TextEntered) {
                // keyboard input (mod keys)
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
                    if (e.text.unicode == '=' && text.getCharacterSize() < 96) {
                        TEXTSIZE++;
                        text.setCharacterSize(TEXTSIZE);
                        text.setOrigin(-(TEXTSIZE * 1.5 + TEXTSIZE / 5), 0);
                        lineNumbers.setCharacterSize(TEXTSIZE);
                        lineNumberBar.setSize(sf::Vector2f(TEXTSIZE * 1.5, SCREEN_HEIGHT));
                    }
                    if (e.text.unicode == '-' && text.getCharacterSize() > 6) {
                        TEXTSIZE--;
                        text.setCharacterSize(TEXTSIZE);
                        text.setOrigin(-(TEXTSIZE * 1.5 + TEXTSIZE / 5), 0);
                        lineNumbers.setCharacterSize(TEXTSIZE);
                        lineNumberBar.setSize(sf::Vector2f(TEXTSIZE * 1.5, SCREEN_HEIGHT));
                    }
                    continue; // dont want to write '=' or '-'
                }
                // text input
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

        window.clear(sf::Color(30, 20, 40));

        window.setView(view);

        window.draw(text);
        window.draw(lineNumberBar);
        window.draw(lineNumbers);
        window.display();
    }


    updateFile("readme.txt", displayString);
    std::cout << "ende" << std::endl;
}