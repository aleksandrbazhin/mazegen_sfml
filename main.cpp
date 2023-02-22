#include "imgui.h" // necessary for ImGui::*, imgui-SFML.h doesn't include imgui.h
#include "imgui-SFML.h" // for ImGui::SFML::* functions and SFML-specific overloads

#include <SFML/Graphics.hpp>

#include <unordered_map>
#include <random>
#include <chrono>
#include "mazegen/mazegen.hpp"

int TILE_SIZE = 32;
int HEIGHT = 31;
int WIDTH = 57;
bool USE_FIXED_SEED = false;
int SEED = 0;


template <typename T>
std::unordered_map<int, sf::Color> get_random_region_colors(
    const std::vector<T>& regions, int seed = 0) {
    std::mt19937 rng;
    if (USE_FIXED_SEED) {
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


sf::VertexArray generate_maze() {
    auto gen = mazegen::Generator();
    if (USE_FIXED_SEED) {
        gen.set_seed(SEED);
    } else {
        // SEED = gen.get_seed();
    }
    mazegen::PointSet constraints {{1, 1}, {WIDTH - 2, HEIGHT - 2}};
    auto grid = gen.generate(WIDTH, HEIGHT, constraints);
    auto doors = gen.get_doors();

    auto hall_colors = get_random_region_colors(gen.get_halls(), SEED);

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
    sf::Texture floor_texture;
    if (!floor_texture.loadFromFile("assets/cell.png")) {
        std::cout << "Error: Can not load floor texture!" << std::endl;
        return 0;
    }
    floor_texture.setRepeated(true);
    sf::VideoMode screen_mode = sf::VideoMode::getDesktopMode(); 
    sf::RenderWindow window(screen_mode, "Mazegen SFML + ImGui demo");
    sf::RenderTexture renderTexture;
    renderTexture.create(screen_mode.width, screen_mode.height);
    sf::Sprite maze_sprite;

    bool imgui_ok = ImGui::SFML::Init(window);

    sf::Clock deltaClock;
    sf::VertexArray maze_vertices = generate_maze();
    bool is_rebuild_needed = true;
    float generation_time = 0.0;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed || 
                (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                    window.close();
                    break;
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
                is_rebuild_needed = true;
            }
            ImGui::SFML::ProcessEvent(window, event);
        }

        ImGui::SFML::Update(window, deltaClock.restart());
        ImGui::SetNextWindowSize({0.0f, 0.0f});

        ImGui::Begin("Mazegen settings");

        ImGui::Text("Size params");
        ImGui::SliderInt("Maze width", &WIDTH, 9, 1841);
        ImGui::SliderInt("Maze height", &HEIGHT, 9, 961);
        ImGui::SliderInt("Room attempts", &mazegen::ROOM_NUMBER, 1, 2000);
        ImGui::SliderInt("Room size min", &mazegen::ROOM_SIZE_MIN, 3, 51);
        ImGui::SliderInt("Room size max", &mazegen::ROOM_SIZE_MAX, 3, 51);

        ImGui::Text("Render params");
        ImGui::SliderInt("Tile size", &TILE_SIZE, 1, 32);

        ImGui::Text("Generation params");
        ImGui::Checkbox("Fixed seed", &USE_FIXED_SEED);

        ImGui::InputInt("Seed", &SEED);
        ImGui::SliderFloat("Deadends chance", &mazegen::DEADEND_CHANCE, 0.0f, 1.0f);
        ImGui::SliderFloat("Extra connection chance", &mazegen::EXTRA_CONNECTION_CHANCE, 0.0f, 1.0f);
        ImGui::SliderFloat("Wiggle chance", &mazegen::WIGGLE_CHANCE, 0.0f, 1.0f);
        ImGui::SliderFloat("Reconnect chance", &mazegen::RECONNECT_DEADENDS_CHANCE, 0.0f, 1.0f);

        if (ImGui::Button("Generate maze! (ENTER)")) {
            is_rebuild_needed = true;
        }
        ImGui::Text("Generated maze in %fms", generation_time);
        ImGui::End();

        if (is_rebuild_needed) {
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            maze_vertices = generate_maze();
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            generation_time = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000.0;
            renderTexture.clear();
            renderTexture.draw(maze_vertices, &floor_texture);
            renderTexture.display();
            maze_sprite.setTexture(renderTexture.getTexture());
            maze_sprite.setPosition(0, 0);
            is_rebuild_needed = false;
        }

        window.clear();
        window.draw(maze_sprite);
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}
