#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>

#include "renderer.h"
#include "triangle.h"
#include "matrix.h"

void Renderer::drawObject(SDL_Renderer* renderer, std::vector<Vertex>& object, double rotX, double rotY, double rotZ, double scale) {
    // Create transformation matrices
    Matrix scaleMatrix = Matrix::createScale(scale, scale, scale);
    Matrix rotXMatrix = Matrix::createRotationX(rotX);
    Matrix rotYMatrix = Matrix::createRotationY(rotY);
    Matrix rotZMatrix = Matrix::createRotationZ(rotZ);
    Matrix translationMatrix = Matrix::createTranslation(0, 0, 5);

    // Create view matrix TODO

    // Create projection matrix
    double fov = M_PI / 3.0; // 60 degree fov
    double aspectRatio = Renderer::WINDOW_WIDTH / Renderer::WINDOW_HEIGHT;
    double near = Renderer::NEAR;
    double far = Renderer::FAR;
    Matrix projectionMatrix = Matrix::createPerspectiveProjection(fov, aspectRatio, near, far);

    // Combine model (rotate, scale, translate) and projection into transform matrix;
    Matrix transformMatrix = projectionMatrix * translationMatrix * scaleMatrix * rotXMatrix * rotYMatrix * rotZMatrix;

    for (size_t i = 0; i < object.size(); i += 3) {
        Vertex v1 = object[i];
        Vertex v2 = object[i+1];
        Vertex v3 = object[i+2];

        // Apply combined transform matrix
        Vertex projectedV1 = transformMatrix * v1;
        Vertex projectedV2 = transformMatrix * v2;
        Vertex projectedV3 = transformMatrix * v3;

         // Convert to screen coordinates
        projectedV1.setX((projectedV1.x() + 1.0) * Renderer::WINDOW_WIDTH / 2.0);
        projectedV1.setY((projectedV1.y() + 1.0) * Renderer::WINDOW_HEIGHT / 2.0);
        projectedV2.setX((projectedV2.x() + 1.0) * Renderer::WINDOW_WIDTH / 2.0);
        projectedV2.setY((projectedV2.y() + 1.0) * Renderer::WINDOW_HEIGHT / 2.0);
        projectedV3.setX((projectedV3.x() + 1.0) * Renderer::WINDOW_WIDTH / 2.0);
        projectedV3.setY((projectedV3.y() + 1.0) * Renderer::WINDOW_HEIGHT / 2.0);

        drawTriangle(renderer, Triangle(projectedV1, projectedV2, projectedV3));
    }
}

void Renderer::drawTriangle(SDL_Renderer* renderer, Triangle triangle) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Draw point v1 to v2
    SDL_RenderDrawLine(renderer, triangle.v1().x(), triangle.v1().y(), triangle.v2().x(), triangle.v2().y());

    // Draw point v2 to v3
    SDL_RenderDrawLine(renderer, triangle.v2().x(), triangle.v2().y(), triangle.v3().x(), triangle.v3().y());

    // Draw point v3 to v1
    SDL_RenderDrawLine(renderer, triangle.v3().x(), triangle.v3().y(), triangle.v1().x(), triangle.v1().y());
}

std::vector<Vertex> Renderer::loadObj(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open the .obj file: " + filename);
    }

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "v") {
            double x, y, z;
            iss >> x >> y >> z;
            vertices.push_back(Vertex(x, y, z, 1));
        } else if (token == "f") {
            unsigned int i1, i2, i3;
            iss >> i1 >> i2 >> i3;
            indices.push_back(i1 - 1);
            indices.push_back(i2 - 1);
            indices.push_back(i3 - 1);
        }
    }

    file.close();

    std::vector<Vertex> orderedVertices;
    for (auto index : indices) {
        orderedVertices.push_back(vertices[index]);
    }

    return orderedVertices;
}
