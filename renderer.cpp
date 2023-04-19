#include <iostream>
#include <cmath>

#include "renderer.h"
#include "triangle.h"
#include "matrix.h"

void Renderer::testDraw(SDL_Renderer* renderer, std::array<Triangle, 12> cube, double rotX, double rotY, double rotZ) {
    // Scaling matrix
    Matrix scaleMatrix = Matrix::createScale(0.5, 0.5, 0.5);

    // Create rotation matrices
    Matrix rotXMatrix = Matrix::createRotationX(rotX);
    Matrix rotYMatrix = Matrix::createRotationY(rotY);
    Matrix rotZMatrix = Matrix::createRotationZ(rotZ);

    // Translation matrix
    Matrix translMatrix = Matrix::createTranslation(-0.5, -0.5, 0.5);

    // Projection matrix
    Matrix projMatrix = Matrix::createPerspectiveProjection(M_PI_2, static_cast<double>(Renderer::WINDOW_WIDTH / Renderer::WINDOW_HEIGHT), Renderer::NEAR, Renderer::FAR);


    for (auto triangle : cube) {
        Vertex v1 = triangle.v1();
        Vertex v2 = triangle.v2();
        Vertex v3 = triangle.v3();

        // Scale and rotate vertices
        Vertex scaledRotatedV1 = rotZMatrix * rotYMatrix * rotXMatrix * scaleMatrix * v1;
        Vertex scaledRotatedV2 = rotZMatrix * rotYMatrix * rotXMatrix * scaleMatrix * v2;
        Vertex scaledRotatedV3 = rotZMatrix * rotYMatrix * rotXMatrix * scaleMatrix * v3;

        // Translate vertices
        Vertex translatedV1 = translMatrix * scaledRotatedV1;
        Vertex translatedV2 = translMatrix * scaledRotatedV2;
        Vertex translatedV3 = translMatrix * scaledRotatedV3;

        // Project vertices
        Vertex projectedV1 = projMatrix * translatedV1;
        Vertex projectedV2 = projMatrix * translatedV2;
        Vertex projectedV3 = projMatrix * translatedV3;

        // Perspective division
        v1 = projectedV1 / projectedV1.w();
        v2 = projectedV2 / projectedV2.w();
        v3 = projectedV3 / projectedV3.w();

        // Map the coordinates from clip space to screen space
        int x1 = static_cast<int>((v1.x() + 1) * 0.5 * Renderer::WINDOW_WIDTH);
        int y1 = static_cast<int>((v1.y() + 1) * 0.5 * Renderer::WINDOW_HEIGHT);
        int x2 = static_cast<int>((v2.x() + 1) * 0.5 * Renderer::WINDOW_WIDTH);
        int y2 = static_cast<int>((v2.y() + 1) * 0.5 * Renderer::WINDOW_HEIGHT);
        int x3 = static_cast<int>((v3.x() + 1) * 0.5 * Renderer::WINDOW_WIDTH);
        int y3 = static_cast<int>((v3.y() + 1) * 0.5 * Renderer::WINDOW_HEIGHT);

        // Create final 2D triangle and draw it
        Triangle projectedTriangle = Triangle(Vertex(x1, y1, 0, 1), Vertex(x2, y2, 0, 1), Vertex(x3, y3, 0, 1));
        drawTriangle(renderer, projectedTriangle);
        
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
