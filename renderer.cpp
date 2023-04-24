#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>

#include "renderer.h"
#include "matrices.h"

std::vector<double> Renderer::depthBuffer(WINDOW_WIDTH * WINDOW_HEIGHT, std::numeric_limits<double>::max());

double Renderer::cameraYaw = 0;
double Renderer::cameraPitch = 0;

Eigen::Vector4d Renderer::lookDir(0, 0, 1, 1);
Eigen::Vector4d Renderer::cameraPos(0, 0, -20, 1);
Eigen::Vector4d Renderer::targetPos(0, 0, 0, 1);
Eigen::Vector4d Renderer::lightDir(0, 0, -1, 1);

void Renderer::onKeys(const std::string& key) {
    Eigen::Vector4d Up(0, 1, 0, 1);
    Eigen::Vector4d Forward = lookDir; // Should already be normalized
    Eigen::Vector3d Right3D = Up.head<3>().cross(Forward.head<3>()).normalized(); // Normalize for regular speed across pitch range
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

void Renderer::onYawPitch(const double& dx, const double& dy) {
    // Check if there is any yaw (dx) or any pitch (dy) and only update the respective movements if they are non-zero
    if (dx != 0) {
        cameraYaw += dx;

        // Clamp yaw to make yaw value independent of number of rotations
        if (cameraYaw > M_PI) {
            cameraYaw = 0;
        } else if (cameraYaw < 0) {
            cameraYaw= M_PI;
        }

        // Create the rotation matrix for yaw
        Eigen::Matrix4d rotationMatrixYaw = Matrices::createRotationY(-dx);

        // Apply yaw rotation to the current look direction vector
        lookDir = rotationMatrixYaw * lookDir;
    }

    if (dy != 0) {
        cameraPitch += dy;

        // Clamp pitch to avoid gimbal lock, don't need to update cam if outside range, exit method
        if (cameraPitch > M_PI_2) {
            cameraPitch = M_PI_2;
            return;
        } else if (cameraPitch < -M_PI_2) {
            cameraPitch = -M_PI_2;
            return;
        }

        // Calculate the "Right" vector by taking the cross product of the Up vector and the current look direction vector
        Eigen::Vector3d rightVector = Eigen::Vector3d(0, 1, 0).cross(lookDir.head<3>());

        // Normalize the "Right" vector
        rightVector.normalize();

        // Create the rotation matrix for pitch
        Eigen::Matrix4d rotationMatrixPitch = Matrices::createRotationCustom(rightVector, -dy);

        // Apply pitch rotation to the current look direction vector
        lookDir = rotationMatrixPitch * lookDir;
    }

    // Update the target position
    targetPos = cameraPos + lookDir;

    // Normalize the look direction vector
    lookDir.normalize();
}

void Renderer::drawObject(SDL_Renderer* renderer, std::vector<Triangle>& object, double rotX, double rotY, double rotZ, double scale) {
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

    // Combine model (rotate, scale, translate) into transform matrix (without view and projection);
    Eigen::Matrix4d transformMatrixNoViewNoProjection = translationMatrix * scaleMatrix * rotXMatrix * rotYMatrix * rotZMatrix;

    for (const Triangle& triangle : object) {
        Eigen::Vector4d v1 = triangle.v1;
        Eigen::Vector4d v2 = triangle.v2;
        Eigen::Vector4d v3 = triangle.v3;

        // CULLING VALS START

        Eigen::Vector4d translatedV1 = transformMatrixNoViewNoProjection * v1;
        Eigen::Vector4d translatedV2 = transformMatrixNoViewNoProjection * v2;
        Eigen::Vector4d translatedV3 = transformMatrixNoViewNoProjection * v3;

        Eigen::Vector3d line1 = (translatedV2 - translatedV1).head<3>();
        Eigen::Vector3d line2 = (translatedV3 - translatedV1).head<3>();
        Eigen::Vector3d normal = line1.cross(line2);
        normal.normalize();

        // Calculate the view direction vector from the camera position to any vertex of the triangle
        Eigen::Vector3d viewDir = (translatedV1.head<3>() - cameraPos.head<3>()).normalized();

        // Calculate the dot product of the normal and view direction vectors
        double dotProduct = normal.dot(viewDir);

        // CULLING VALS END
        if (dotProduct < 0) {
            lightDir.normalize();
            double normLightDotProd = normal.dot(lightDir.head<3>());

            // Apply combined transform matrix
            Eigen::Vector4d projectedV1 = projectionMatrix * viewMatrix * translatedV1;
            Eigen::Vector4d projectedV2 = projectionMatrix * viewMatrix * translatedV2;
            Eigen::Vector4d projectedV3 = projectionMatrix * viewMatrix * translatedV3;

            // Convert to screen coordinates
            projectedV1.x() /= projectedV1.w();
            projectedV1.y() /= projectedV1.w();
            projectedV1.x() = (projectedV1.x() + 1.0) * scaleFactor;
            projectedV1.y() = (1.0 - projectedV1.y()) * scaleFactor;

            projectedV2.x() /= projectedV2.w();
            projectedV2.y() /= projectedV2.w();
            projectedV2.x() = (projectedV2.x() + 1.0) * scaleFactor;
            projectedV2.y() = (1.0 - projectedV2.y()) * scaleFactor;

            projectedV3.x() /= projectedV3.w();
            projectedV3.y() /= projectedV3.w();
            projectedV3.x() = (projectedV3.x() + 1.0) * scaleFactor;
            projectedV3.y() = (1.0 - projectedV3.y()) * scaleFactor;

            drawTriangle(renderer, Triangle(projectedV1, projectedV2, projectedV3), getShadingColor(normLightDotProd));
        }
    }
}

void Renderer::drawTriangle(SDL_Renderer* renderer, const Triangle& triangle) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Draw point v1 to v2
    SDL_RenderDrawLine(renderer, triangle.v1.x(), triangle.v1.y(), triangle.v2.x(), triangle.v2.y());

    // Draw point v2 to v3
    SDL_RenderDrawLine(renderer, triangle.v2.x(), triangle.v2.y(), triangle.v3.x(), triangle.v3.y());

    // Draw point v3 to v1
    SDL_RenderDrawLine(renderer, triangle.v3.x(), triangle.v3.y(), triangle.v1.x(), triangle.v1.y());
}

void Renderer::drawTriangle(SDL_Renderer* renderer, const Triangle& triangle, SDL_Color color) {
    auto edgeFunction = [](const Eigen::Vector4d& a, const Eigen::Vector4d& b, const Eigen::Vector4d& c) {
        return (c.x() - a.x()) * (b.y() - a.y()) - (c.y() - a.y()) * (b.x() - a.x());
    };

    // Sort vertices by y-coordinate
    std::array<Eigen::Vector4d, 3> sortedTriangle = {triangle.v1, triangle.v2, triangle.v3};
    std::sort(sortedTriangle.begin(), sortedTriangle.end(),
              [](const Eigen::Vector4d& a, const Eigen::Vector4d& b) { return a.y() < b.y(); });

    const Eigen::Vector4d& v0 = sortedTriangle[0];
    const Eigen::Vector4d& v1 = sortedTriangle[1];
    const Eigen::Vector4d& v2 = sortedTriangle[2];

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // Calculate the area of the triangle
    double area = edgeFunction(v0, v1, v2);

    for (int y = static_cast<int>(std::ceil(v0.y())); y < static_cast<int>(std::ceil(v2.y())); y++) {
        bool topHalf = y < v1.y();

        // Calculate the x-coordinate of the intersection points of the scanline with the triangle edges
        double x0 = v0.x() + (y - v0.y()) * (v2.x() - v0.x()) / (v2.y() - v0.y());
        double x1 = topHalf
                        ? v0.x() + (y - v0.y()) * (v1.x() - v0.x()) / (v1.y() - v0.y())
                        : v1.x() + (y - v1.y()) * (v2.x() - v1.x()) / (v2.y() - v1.y());

        // Ensure x0 is always on the left and x1 on the right
        if (x0 > x1) {
            std::swap(x0, x1);
        }

        for (int x = static_cast<int>(std::ceil(x0)); x < static_cast<int>(std::ceil(x1)); x++) {
            // Calculate the barycentric coordinates of the pixel
            double w0 = edgeFunction(v1, v2, Eigen::Vector4d(x, y, 0, 1)) / area;
            double w1 = edgeFunction(v2, v0, Eigen::Vector4d(x, y, 0, 1)) / area;
            double w2 = edgeFunction(v0, v1, Eigen::Vector4d(x, y, 0, 1)) / area;

            // Use the barycentric coordinates to interpolate the z (depth) value for the current pixel
            double z = v0.z() * w0 + v1.z() * w1 + v2.z() * w2;

            // Check if the current pixel is within the triangle and update the depth buffer
            int index = x + y * WINDOW_WIDTH;
            if (index >= 0 && index < WINDOW_WIDTH * WINDOW_HEIGHT && z < depthBuffer[index]) {
                depthBuffer[index] = z;
                // Draw the pixel since it passes the Z-buffer test
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
}

SDL_Color Renderer::getShadingColor(double intensity) {
    intensity = std::clamp(intensity, 0.1, 1.0);
    Uint8 grayValue = static_cast<Uint8>(intensity * 255);

    SDL_Color color;
    color.r = grayValue;
    color.g = grayValue;
    color.b = grayValue;
    color.a = 255;

    return color;
}

std::vector<Triangle> Renderer::loadObj(const std::string& filename) {
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
            if (!(iss >> x >> y >> z)) {
                throw std::runtime_error("Invalid vertex format in .obj file: " + filename);
            }
            vertices.push_back(Eigen::Vector4d(x, y, z, 1));
        } else if (token == "f") {
            unsigned int i1, i2, i3;
            if (!(iss >> i1 >> i2 >> i3)) {
                throw std::runtime_error("Invalid face format in .obj file: " + filename);
            }
            indices.push_back(i1 - 1);
            indices.push_back(i2 - 1);
            indices.push_back(i3 - 1);
        }
    }

    file.close();

    if (indices.size() % 3 != 0) {
        throw std::runtime_error("Invalid number of indices in .obj file: " + filename);
    }

    std::vector<Triangle> triangles;
    for (size_t i = 0; i < indices.size(); i += 3) {
        triangles.push_back(Triangle(vertices[indices[i]], vertices[indices[i + 1]], vertices[indices[i + 2]]));
    }

    return triangles;
}