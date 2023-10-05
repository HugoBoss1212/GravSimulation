#include <cmath>
#include <vector>
#include <functional>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <random>

#include "Particle.h"
#include "Quadtree.h"
#include "HugoStable.h"
#include "Timer.h"


// Assume these constants are defined appropriately
double MASS1 = 5;
double W1 = 5;
double MASS2 = 10;
double W2 = 10;
double MASS3 = 10;
double W3 = 10;
double TYPE1_COUNT = 5;
double TYPE2_COUNT = 5;
double TYPE3_COUNT = 1; //only one!
double TAIL_CUTOFF = 5;
double RENDER_GRAVITY_RADIUS = 5;
double SHOW_GRAV = 1;
double TIME_SCALE = 0.9;
double k = 0.01; // 0.01 is an example value for k, adjust as needed

double result[HUGO_STABLE][HUGO_STABLE];

double getRandomDouble(double min, double max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

std::vector<Particle> randomizeParticles(int count, double A, double W, 
                                         double x_min, double x_max, 
                                         double y_min, double y_max, 
                                         double vx_min, double vx_max, 
                                         double vy_min, double vy_max,
                                         ParticleType particleType) {
    std::vector<Particle> particles;
    for (int i = 0; i < count; i++) {
        double x_offset = getRandomDouble(x_min, x_max);
        double y_offset = getRandomDouble(y_min, y_max);
        double velocity_x = getRandomDouble(vx_min, vx_max);
        double velocity_y = getRandomDouble(vy_min, vy_max);
        particles.push_back(Particle(A, W, x_offset, y_offset, velocity_x, velocity_y, particleType));
    }
    return particles;
}

bool readSettingsFromFile(const std::string& filename,
                          double& TIME_SCALE, double& k,
                          double& A1, double& W1,
                          double& A2, double& W2,
                          double& A3, double& W3) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open settings file." << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        double value;
        if (std::getline(iss, key, '=') && iss >> value) {
            if (key == "TIME_SCALE") TIME_SCALE = value;
            else if (key == "k") k = value;
            else if (key == "MASS1") MASS1 = value;
            else if (key == "W1") W1 = value;
            else if (key == "MASS2") MASS2 = value;
            else if (key == "W2") W2 = value;
            else if (key == "MASS3") MASS3 = value;
            else if (key == "W3") W3 = value;
            else if (key == "TYPE3_COUNT") TYPE3_COUNT = value;
            else if (key == "TYPE2_COUNT") TYPE2_COUNT = value;
            else if (key == "TYPE1_COUNT") TYPE1_COUNT = value;
            else if (key == "TAIL_CUTOFF") TAIL_CUTOFF = value;
            else if (key == "RENDER_GRAVITY_RADIUS") RENDER_GRAVITY_RADIUS = value;
            else if (key == "SHOW_GRAV") SHOW_GRAV = value;
        }
    }
    return true;
}

std::filesystem::file_time_type getLastModifiedTime(const std::string& filename) {
    return std::filesystem::last_write_time(filename);
}


void generateData(double t, double result[HUGO_STABLE][HUGO_STABLE], std::vector<Particle>& peaks, double tk) {
    //Timer functionTimer2;
    std::fill(&result[0][0], &result[0][0] + HUGO_STABLE * HUGO_STABLE, 0.0);
    bool computed[HUGO_STABLE][HUGO_STABLE] = { false };  // This array will keep track of which pixels have been computed

    for (Particle& peak : peaks) {

        double minX, maxX, minY, maxY;
        minX = peak.getX() - RENDER_GRAVITY_RADIUS;
        maxX = peak.getX() + RENDER_GRAVITY_RADIUS;
        minY = peak.getY() - RENDER_GRAVITY_RADIUS;
        maxY = peak.getY() + RENDER_GRAVITY_RADIUS;

        for (double x = minX; x <= maxX; ++x) {
            for (double y = minY; y <= maxY; ++y) {
                int i = static_cast<int>(x + HUGO_STABLE / 2.0);
                int j = static_cast<int>(y + HUGO_STABLE / 2.0);

                if (i >= 0 && i < HUGO_STABLE && j >= 0 && j < HUGO_STABLE && !computed[i][j]) {
                    result[i][j] = Particle::g0(x, y, peaks, t);
                    computed[i][j] = true;  // Mark the pixel as computed
                }
            }
        }

    }
    //std::cout << "g0 gravity loop in: " << functionTimer2.elapsed() << " microseconds." << std::endl;

    //Timer functionTimer1;
    std::vector<Particle*> A1particles;  // List of pointers to A1 type particles    
    std::vector<Particle*> peakPtrs;
    for (Particle& peak : peaks) {
        peakPtrs.push_back(&peak);  // Add the address of each Particle to the peakPtrs vector

        if (peak.getType() == ::A1) {
            A1particles.push_back(&peak);  // Add the address of A1 type Particle to the A1particles vector
        }
    }

    // Update peak positions
    for (Particle& peak : peaks) {
        peak.updatePosition(peakPtrs, t, k, result, tk);  // Pass peakPtrs instead of peaks
    }
    //std::cout << "particles vectorisation,update position completed in: " << functionTimer1.elapsed() << " microseconds." << std::endl;
}

// Define a structure to hold both position and color
struct DotInfo {
    int x, y;
    sf::Color color;
};

// This function visualizes the data
void visualizeData(double t, double result[HUGO_STABLE][HUGO_STABLE], const std::vector<Particle>& peaks, sf::RenderWindow& window) {
    //Timer functionTimer3;
    // Calculate the max_value using STL
    double max_value = *std::max_element(&result[0][0], &result[0][0] + HUGO_STABLE * HUGO_STABLE);
    
    if (max_value == 0.0) {
        return;  // or handle this error in another way
    }
    //std::cout << "max_vlaue completed in: " << functionTimer3.elapsed() << " microseconds." << std::endl;

    // Create an SFML image
    sf::Image image;
    image.create(HUGO_STABLE, HUGO_STABLE);

    const sf::Color RED_COLOR = sf::Color::Red;
    const sf::Color GREEN_COLOR = sf::Color::Green;
    const sf::Color BLUE_COLOR = sf::Color::Blue;

    // Timer functionTimer4;
    // Normalize the result array to range 0 - 255 and set image pixels
    if (SHOW_GRAV == 1) {
        for (int i = 0; i < HUGO_STABLE; ++i) {
            for (int j = 0; j < HUGO_STABLE; ++j) {
                unsigned char value = static_cast<unsigned char>((result[i][j] / max_value) * 255);
                sf::Color grayscale(value/4, value/4, value/4);
                image.setPixel(static_cast<unsigned int>(i), static_cast<unsigned int>(j), grayscale);
            }
        }
    }
    // std::cout << "normalize completed in: " << functionTimer4.elapsed() << " microseconds." << std::endl;

    //Timer functionTimer5;
    for (const Particle& peak : peaks) {
        const auto& history = peak.getHistory();
        size_t history_size = history.size() > TAIL_CUTOFF ? TAIL_CUTOFF : history.size();

        for (size_t i = 0; i < history_size; i++) {
            const auto& pos = history[history.size() - 1 - i]; // Access history in reverse

            // Calculate fade factor (from 0.2 at the start to 1.0 at the most recent position)
            double fade_factor = 0.2 + (i / static_cast<double>(history_size)) * 0.8;

            // Calculate faded alpha
            sf::Uint8 faded_alpha = static_cast<sf::Uint8>(255 * fade_factor);

            sf::Color faded_color = RED_COLOR;
            if (peak.getType() == ParticleType::A1) {
                faded_color = RED_COLOR;
            } else if (peak.getType() == ParticleType::A2) {
                faded_color = GREEN_COLOR;
            } else if (peak.getType() == ParticleType::A3) {
                faded_color = BLUE_COLOR;
            }
            faded_color.a = faded_alpha;  // Adjusting only the alpha for transparency

            int x = static_cast<int>(HUGO_STABLE / 2 + pos.first);
            int y = static_cast<int>(HUGO_STABLE / 2 + pos.second);
            image.setPixel(x, y, faded_color);
        }
    }



    // Populate the list of red dots and their respective colors
    std::vector<DotInfo> dot_infos;
    for (const Particle& peak : peaks) {
        DotInfo info;
        info.x = static_cast<int>(HUGO_STABLE / 2 + peak.getX());
        info.y = static_cast<int>(HUGO_STABLE / 2 + peak.getY());

        // Determine the color based on peak type
        if (peak.getType() == ParticleType::A1) {
            info.color = RED_COLOR;
        } else if (peak.getType() == ParticleType::A2) {
            info.color = GREEN_COLOR;
        } else if (peak.getType() == ParticleType::A3) {
            info.color = BLUE_COLOR; // Default color
        }

        dot_infos.push_back(info);
    }

    // Add dots to the image using their respective colors
    for (const DotInfo& info : dot_infos) {
        image.setPixel(info.x, info.y, info.color);
    }
    //std::cout << "red dot tail,position,pixel completed in: " << functionTimer5.elapsed() << " microseconds." << std::endl;

    //Timer functionTimer6;
    // Create an SFML texture from the image
    sf::Texture texture;
    if (!texture.loadFromImage(image)) {
        std::cerr << "Error: Could not create texture from image." << std::endl;
        return;
    }

    // Create an SFML sprite from the texture
    sf::Sprite sprite;
    sprite.setTexture(texture);

    // Draw the sprite to the window
    window.clear();
    window.draw(sprite);
    window.display();
    //std::cout << "texture and dislpay completed in: " << functionTimer6.elapsed() << " microseconds." << std::endl;
}

int main() {
    std::string settingsFile = "/home/hugo/GravitySymulation/src/resources/properties.txt";
    readSettingsFromFile(settingsFile, TIME_SCALE, k, MASS1, W1, MASS2, W2, MASS3, W3);

    sf::RenderWindow window(sf::VideoMode(HUGO_STABLE, HUGO_STABLE), "GravitySimulation");

    std::vector<Particle> particles1 = randomizeParticles(TYPE1_COUNT, MASS1, W1, -5, -4, -5, -4, -0.1, 0.1, -0.1, 0.1, ParticleType::A1);
    std::vector<Particle> particles2 = randomizeParticles(TYPE2_COUNT, MASS2, W2, -5, -4, -5, -4, -0.1, 0.1, -0.1, 0.1, ParticleType::A2);
    std::vector<Particle> particles3 = randomizeParticles(TYPE3_COUNT, MASS3, W3, -10, 10, -10, 10, 0, 0, 0, 0, ParticleType::A3);
    particles1.insert(particles1.end(), 
             std::make_move_iterator(particles2.begin()), 
             std::make_move_iterator(particles2.end()));
    particles1.insert(particles1.end(), 
             std::make_move_iterator(particles3.begin()), 
             std::make_move_iterator(particles3.end()));

    double tk = 0;
    std::cout << "magnitude;type;tk" << std::endl;

    double t = 0;
    bool oscillating = false; 
    bool increasing = true;

    while (true) {
        if (t >= 0.3 && !oscillating) {
            oscillating = true;
            increasing = false;
        }

        if(oscillating) {
            if (t <= 0.29) {
                increasing = true;
            } else if (t >= 0.3) {
                increasing = false;
            }
        }

        // your loop contents
        tk += TIME_SCALE;
        
        //Timer functionTimer7;
        generateData(t, result, particles1, tk);
        visualizeData(t, result, particles1, window);

        double max_value = *std::max_element(&result[0][0], &result[0][0] + HUGO_STABLE * HUGO_STABLE);
        //std::cout << "Loop t=" << t << ", Max value of result: " << max_value << std::endl;

        // Check for close event
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return 0;
            }
        }
        //std::cout << t << std::endl;

        if(increasing) {
            t += TIME_SCALE;
        } else {
            t -= TIME_SCALE;
        }
    }
    return 0;

}