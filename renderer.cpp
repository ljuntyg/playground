#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>

#include "renderer.h"
#include "triangle.h"
#include "matrices.h"

double Renderer::cameraYaw = 0;

double Renderer::cameraPitch = 0;

Eigen::Vector4d Renderer::lookDir(0, 0, 1, 1);

Eigen::Vector4d Renderer::cameraPos(0, 0, -50, 1);

Eigen::Vector4d Renderer::targetPos(0, 0, 0, 1);

void Renderer::onKeys(const std::string& key) {

    Eigen::Vector4d Up(0, 1, 0, 1);
    Eigen::Vector4d Forward = lookDir;
    Eigen::Vector3d Right3D = Up.head<3>().cross(Forward.head<3>());
    Eigen::Vector4d Right(Right3D.x(), Right3D.y(), Right3D.z(), 1);

    Up *= cameraSpeed;
    Right *= cameraSpeed;
    Forward *= cameraSpeed;

    if (key == "UP") {
        cameraPos += Up;
        targetPos += Up;
    }
    if (key == "DOWN") {
        cameraPos -= Up;
        targetPos -= Up;
    }
    if (key == "RIGHT") {
        cameraPos += Right;
        targetPos += Right;
    }
    if (key == "LEFT") {
        cameraPos -= Right;
        targetPos -= Right;
    }
    if (key == "FORWARD") {
        cameraPos += Forward;
        targetPos += Forward;
    }
    if (key == "BACK") {
        cameraPos -= Forward;
        targetPos -= Forward;
    }
}

void Renderer::onYawPitch() {
    // Create the rotation matrix for yaw
    Eigen::Matrix4d rotationMatrixYaw = Matrices::createRotationY(-cameraYaw);

    // Calculate the horizontal direction vector of the camera
    Eigen::Vector3d horizontalDir(lookDir.x(), 0, lookDir.z());
    horizontalDir.normalize();

    // Calculate the rotation axis by taking the cross product of the horizontal direction vector and the Up vector
    Eigen::Vector3d rotationAxis = Eigen::Vector3d(0, 1, 0).cross(horizontalDir);

    // Create the rotation matrix for pitch
    Eigen::Matrix4d rotationMatrixPitch = Matrices::createRotationCustom(rotationAxis, -cameraPitch);

    // Apply both yaw and pitch rotations to the initial look direction vector
    lookDir = rotationMatrixYaw * rotationMatrixPitch * Eigen::Vector4d(0, 0, 1, 1);

    targetPos = cameraPos + lookDir;
    lookDir.normalize();
}


void Renderer::drawObject(SDL_Renderer* renderer, std::vector<Eigen::Vector4d>& object, double rotX, double rotY, double rotZ, double scale) {
    // Create transformation matrices
    Eigen::Matrix4d scaleMatrix = Matrices::createScale(scale, scale, scale);
    Eigen::Matrix4d rotXMatrix = Matrices::createRotationX(rotX);
    Eigen::Matrix4d rotYMatrix = Matrices::createRotationY(rotY);
    Eigen::Matrix4d rotZMatrix = Matrices::createRotationZ(rotZ);
    Eigen::Matrix4d translationMatrix = Matrices::createTranslation(0, 0, 0);

    // Create view matrix
    Eigen::Matrix4d viewMatrix = Matrices::createViewMatrix(cameraPos, targetPos, Eigen::Vector4d(0, 1, 0, 1));

    // Create projection matrix
    Eigen::Matrix4d projectionMatrix = Matrices::createPerspectiveProjection(FOV, WINDOW_WIDTH / WINDOW_HEIGHT, NEAR, FAR);

    // Combine model (rotate, scale, translate), view and projection into transform matrix;
    Eigen::Matrix4d transformMatrix = projectionMatrix * viewMatrix * translationMatrix * scaleMatrix * rotXMatrix * rotYMatrix * rotZMatrix;

    for (size_t i = 0; i < object.size(); i += 3) {
        Eigen::Vector4d v1 = object[i];
        Eigen::Vector4d v2 = object[i+1];
        Eigen::Vector4d v3 = object[i+2];

        // Apply combined transform matrix
        Eigen::Vector4d projectedV1 = transformMatrix * v1;
        Eigen::Vector4d projectedV2 = transformMatrix * v2;
        Eigen::Vector4d projectedV3 = transformMatrix * v3;

        // Convert to screen coordinates
        projectedV1.x() /= projectedV1.w();
        projectedV1.y() /= projectedV1.w();
        projectedV1.x() = (projectedV1.x() + 1.0) * WINDOW_WIDTH / 2.0;
        projectedV1.y() = (1.0 - projectedV1.y()) * WINDOW_HEIGHT / 2.0;

        projectedV2.x() /= projectedV2.w();
        projectedV2.y() /= projectedV2.w();
        projectedV2.x() = (projectedV2.x() + 1.0) * WINDOW_WIDTH / 2.0;
        projectedV2.y() = (1.0 - projectedV2.y()) * WINDOW_HEIGHT / 2.0;

        projectedV3.x() /= projectedV3.w();
        projectedV3.y() /= projectedV3.w();
        projectedV3.x() = (projectedV3.x() + 1.0) * WINDOW_WIDTH / 2.0;
        projectedV3.y() = (1.0 - projectedV3.y()) * WINDOW_HEIGHT / 2.0;

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

std::vector<Eigen::Vector4d> Renderer::loadObj(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open the .obj file: " + filename);
    }

    std::vector<Eigen::Vector4d> vertices;
    std::vector<unsigned int> indices;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "v") {
            double x, y, z;
            iss >> x >> y >> z;
            vertices.push_back(Eigen::Vector4d(x, y, z, 1));
        } else if (token == "f") {
            unsigned int i1, i2, i3;
            iss >> i1 >> i2 >> i3;
            indices.push_back(i1 - 1);
            indices.push_back(i2 - 1);
            indices.push_back(i3 - 1);
        }
    }

    file.close();

    std::vector<Eigen::Vector4d> orderedVertices;
    for (auto index : indices) {
        orderedVertices.push_back(vertices[index]);
    }

    return orderedVertices;
}