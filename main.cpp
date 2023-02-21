#include "imgui.h" // necessary for ImGui::*, imgui-SFML.h doesn't include imgui.h
#include "imgui-SFML.h" // for ImGui::SFML::* functions and SFML-specific overloads

#include <SFML/Graphics.hpp>

#include <unordered_map>
#include <random>
#include <chrono>
#include "mazegen/mazegen.hpp"

// const int TILE_SIZE = 16;
// const int HEIGHT = 61;
// const int WIDTH = 115;

// const int TILE_SIZE = 8;
// const int HEIGHT = 121;
// const int WIDTH = 231;
// const int ROOMS = 700;
// const int ROOM_SIZE_MIN = 7;
// const int ROOM_SIZE_MAX = 11;

// const int TILE_SIZE = 1;
// const int HEIGHT = 961;
// const int WIDTH = 1841;
// const int ROOMS = 1000;
// const int ROOM_SIZE_MIN = 15;
// const int ROOM_SIZE_MAX = 59;

const int TILE_SIZE = 32;
const int HEIGHT = 27;
const int WIDTH = 43;
const int ROOMS = 15;
const int ROOM_SIZE_MIN = 5;
const int ROOM_SIZE_MAX = 7;



sf::Texture floor_texture;

void prepare() {
    if (!floor_texture.loadFromFile("assets/cell.png"))
        return;
    floor_texture.setRepeated(true);
}


template <typename T>
std::unordered_map<int, sf::Color> get_random_region_colors(
    const std::vector<T>& regions, bool use_seed = false, int seed = 0) {

    std::mt19937 rng;
    if (use_seed) {
        rng.seed(seed);
    } else {
        std::random_device rd;
        rng.seed(rd());
    }

    std::unordered_map<int, sf::Color> color_map;

    std::uniform_int_distribution<uint8_t> distribution(0, 255);
    for (auto& region: regions) {
        color_map[region.id] = sf::Color(distribution(rng), distribution(rng), distribution(rng));
    }

    return color_map;
}

float chance = 0.0;
bool trigger = false;


sf::VertexArray generate_maze() {
    mazegen::EXTRA_CONNECTION_CHANCE = 0.0;
    mazegen::WIGGLE_CHANCE = 0.3;
    mazegen::DEADEND_CHANCE = chance;

    mazegen::ROOM_NUMBER = ROOMS;
    mazegen::ROOM_SIZE_MIN = ROOM_SIZE_MIN;
    mazegen::ROOM_SIZE_MAX = ROOM_SIZE_MAX;

    mazegen::RECONNECT_DEADENDS_CHANCE = 0.0;
    
    chance += 0.1;

    int SEED = 101;

    auto gen = mazegen::Generator();
    gen.set_seed(SEED);
    mazegen::PointSet constraints {{1, 1}, {WIDTH - 2, HEIGHT - 2}};
    auto grid = gen.generate(WIDTH, HEIGHT, constraints);
    auto doors = gen.get_doors();

    auto hall_colors = get_random_region_colors(gen.get_halls(), true, SEED);

    sf::VertexArray map_vertices;
    map_vertices.setPrimitiveType(sf::Quads);
    map_vertices.resize(HEIGHT * WIDTH * 4);
    for (int y = 0; y < grid.size(); y++) {
         for (int x = 0; x < grid[0].size(); x++) {
            int id = grid[y][x];
            if (id  != mazegen::NOTHING_ID) {
                sf::Vertex* quad = &map_vertices[(x + y * WIDTH) * 4];

                quad[0].position = sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE);
                quad[1].position = sf::Vector2f((x + 1) * TILE_SIZE, y * TILE_SIZE);
                quad[2].position = sf::Vector2f((x + 1) * TILE_SIZE, (y + 1) * TILE_SIZE);
                quad[3].position = sf::Vector2f(x  * TILE_SIZE, (y + 1) * TILE_SIZE);

                quad[0].texCoords = sf::Vector2f(0, 0);
                quad[1].texCoords = sf::Vector2f(TILE_SIZE, 0);
                quad[2].texCoords = sf::Vector2f(TILE_SIZE, TILE_SIZE);
                quad[3].texCoords = sf::Vector2f(0, TILE_SIZE);
                
                if (hall_colors.find(id) != hall_colors.end()) {
                    quad[0].color = hall_colors[id];
                    quad[1].color = hall_colors[id];
                    quad[2].color = hall_colors[id];
                    quad[3].color = hall_colors[id];
                }
            }
        }
    }
    return map_vertices;
}


int main()
{
    prepare();
    sf::RenderWindow window(sf::VideoMode(WIDTH * TILE_SIZE, HEIGHT * TILE_SIZE), "Map");
    // render_game(window);

    bool imgui_ok = ImGui::SFML::Init(window);
    sf::Clock deltaClock;

    sf::VertexArray maze_vertices = generate_maze();

    while (window.isOpen())
    {
        bool is_rebuild_needed = false;
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(window, event);
            switch (event.type)
            {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Escape) {
                        window.close();
                    } else {
                        is_rebuild_needed = true;
                    }
                    break;
            }
        }

        if (is_rebuild_needed) {
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            maze_vertices = generate_maze();
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::cout << "Generated a " << WIDTH << "x" << HEIGHT << " maze in " 
                << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() 
                << " ms" << std::endl;
        }

        ImGui::SFML::Update(window, deltaClock.restart());
        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::End();

        window.clear();
        window.draw(maze_vertices, &floor_texture);
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}
