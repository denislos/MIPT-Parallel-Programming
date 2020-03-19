
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <cstdio>
#include <thread>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>


#include <omp.h>

#include "matrix.h"
#include "support.h"

int inline check_number(long number) { return number >= 0; }


static constexpr const std::size_t NUM_ROWS = 70;
static constexpr const std::size_t NUM_COLS = 120;

static constexpr const std::size_t FPS_TEXT_X_POS = 620;
static constexpr const std::size_t FPS_TEXT_Y_POS = 8;


static constexpr const std::size_t FPS_BOX_STRING_SIZE = 10;


int main(int argc, char** argv)
{
    if (argc != 2)
        raise_error("Bad arguments");

    const unsigned long num_threads = get_number_from_arguments(argc, argv, 1, check_number);

    omp_set_num_threads(num_threads);

    Matrix matrix(NUM_ROWS, NUM_COLS);

    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Life Simulator", sf::Style::Fullscreen);
  
    sf::Text fps_box;
    
    sf::Font font;
    if (!font.loadFromFile("../Consolas.ttf"))
        raise_error("Bad create font");

    fps_box.setFillColor(sf::Color::Black);
    fps_box.setFont(font);
    fps_box.setPosition(sf::Vector2f(FPS_TEXT_X_POS, FPS_TEXT_Y_POS));
    fps_box.setString("0 fps");

    char* fps_box_string = new(std::nothrow) char[FPS_BOX_STRING_SIZE];
    if (fps_box_string == nullptr)
        raise_error("Bad alloc");

    std::size_t frame_counter = 0;

    //const auto start_time = std::chrono::system_clock::now();

    double total_time = 0;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed || 
                (event.type == sf::Event::KeyPressed && sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)))
            {
                window.close();
            }
		}

        const auto start = std::chrono::system_clock::now();
        matrix.clock();
        const auto end = std::chrono::system_clock::now();

        window.clear(sf::Color::Green);
        matrix.draw(window);

        frame_counter++;
        //const auto measure = std::chrono::system_clock::now();
        //const double duration = std::chrono::duration_cast<std::chrono::nanoseconds>(measure - start_time).count();
        total_time += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count(); 
        const double fps = frame_counter * 1000000000 / total_time;

        sprintf(fps_box_string, "%.2f fps", fps);
        fps_box.setString(fps_box_string);

        window.draw(fps_box);
        window.display();
    }

    delete[] fps_box_string;

    return EXIT_SUCCESS;
}