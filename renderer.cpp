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
Eigen::Vector4d Renderer::cameraPos(0, 0, -15, 1);
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

    // Define the clip planes
    std::vector<ClipPlane> clipPlanes = {
        ClipPlane(Eigen::Vector4d(1, 0, WINDOW_WIDTH / WINDOW_HEIGHT / tan(FOV / 2), 0), 0), // Left
        ClipPlane(Eigen::Vector4d(-1, 0, WINDOW_WIDTH / WINDOW_HEIGHT / tan(FOV / 2), 0), 0), // Right
        ClipPlane(Eigen::Vector4d(0, 1, 1 / tan(FOV / 2), 0), 0), // Top
        ClipPlane(Eigen::Vector4d(0, -1, 1 / tan(FOV / 2), 0), 0), // Bottom
        ClipPlane(Eigen::Vector4d(0, 0, 1, -NEAR), NEAR), // Near
        ClipPlane(Eigen::Vector4d(0, 0, -1, FAR), -FAR) // Far
    };

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

        lightDir.normalize();
        double normLightDotProd = normal.dot(lightDir.head<3>());

        // CULLING VALS END (SKIP IF BACK FACE)
        if (dotProduct > 0) {
            continue;
        }

        // Transform and project the triangle
        Eigen::Vector4d projectedV1 = projectionMatrix * viewMatrix * transformMatrixNoViewNoProjection * v1;
        Eigen::Vector4d projectedV2 = projectionMatrix * viewMatrix * transformMatrixNoViewNoProjection * v2;
        Eigen::Vector4d projectedV3 = projectionMatrix * viewMatrix * transformMatrixNoViewNoProjection * v3;

        Triangle projectedTriangle = Triangle(projectedV1, projectedV2, projectedV3, normLightDotProd);

        // Clip the triangle against the view frustum (all planes)
        std::vector<Triangle> clippedTriangles = {projectedTriangle};
        for (const ClipPlane& clipPlane : clipPlanes) {
            std::vector<Triangle> newTriangles;
            for (const Triangle& clippedTriangle : clippedTriangles) {
                std::vector<Triangle> clipped = clipTriangle(clippedTriangle, clipPlane);
                newTriangles.insert(newTriangles.end(), clipped.begin(), clipped.end());
            }
            clippedTriangles = newTriangles;
        }

        // Draw the clipped triangles
        for (const Triangle& clippedTriangle : clippedTriangles) {
            Eigen::Vector4d screenV1 = clippedTriangle.v1;
            Eigen::Vector4d screenV2 = clippedTriangle.v2;
            Eigen::Vector4d screenV3 = clippedTriangle.v3;

            // Convert to screen space coordinates
            screenV1.x() /= screenV1.w();
            screenV1.y() /= screenV1.w();
            screenV1.x() = (screenV1.x() + 1.0) * scaleFactor;
            screenV1.y() = (1.0 - screenV1.y()) * scaleFactor;

            screenV2.x() /= screenV2.w();
            screenV2.y() /= screenV2.w();
            screenV2.x() = (screenV2.x() + 1.0) * scaleFactor;
            screenV2.y() = (1.0 - screenV2.y()) * scaleFactor;

            screenV3.x() /= screenV3.w();
            screenV3.y() /= screenV3.w();
            screenV3.x() = (screenV3.x() + 1.0) * scaleFactor;
            screenV3.y() = (1.0 - screenV3.y()) * scaleFactor;

            drawTriangle(renderer, Triangle(screenV1, screenV2, screenV3), getShadingColor(clippedTriangle.shadeVal));
        }
    }

    // Clear depth buffer
    std::fill(depthBuffer.begin(), depthBuffer.end(), std::numeric_limits<double>::max());
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
    // Sort vertices by y-coordinate
    std::array<Eigen::Vector4d, 3> sortedTriangle = {triangle.v1, triangle.v2, triangle.v3};
    std::sort(sortedTriangle.begin(), sortedTriangle.end(),
              [](const Eigen::Vector4d& a, const Eigen::Vector4d& b) { return a.y() < b.y(); });

    const Eigen::Vector4d& v0 = sortedTriangle[0];
    const Eigen::Vector4d& v1 = sortedTriangle[1];
    const Eigen::Vector4d& v2 = sortedTriangle[2];

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    for (int y = static_cast<int>(std::ceil(v0.y())); y < static_cast<int>(std::ceil(v2.y())); y++) {
        if (y < 0 || y >= WINDOW_HEIGHT) {
            continue;
        }

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
            if (x < 0 || x >= WINDOW_WIDTH) {
                continue;
            }

            // Compute barycentric coordinates
            double area = ((v1.y() - v2.y()) * (v0.x() - v2.x()) + (v2.x() - v1.x()) * (v0.y() - v2.y()));
            double w0 = ((v1.y() - v2.y()) * (x - v2.x()) + (v2.x() - v1.x()) * (y - v2.y())) / area;
            double w1 = ((v2.y() - v0.y()) * (x - v2.x()) + (v0.x() - v2.x()) * (y - v2.y())) / area;
            double w2 = 1.0 - w0 - w1;

            // Compute depth value at this pixel
            double z = w0 * v0.z() + w1 * v1.z() + w2 * v2.z();

            // Check if the current fragment is closer to the camera than the one in the depth buffer
            int depthBufferIdx = y * WINDOW_WIDTH + x;
            if (z < depthBuffer[depthBufferIdx]) {
                // Update the depth buffer
                depthBuffer[depthBufferIdx] = z;

                // Draw the fragment
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
}

std::vector<Triangle> Renderer::clipTriangle(const Triangle& triangle, const ClipPlane& clipPlane) {
    std::vector<Eigen::Vector4d> insideVertices;
    std::vector<Eigen::Vector4d> outsideVertices;
    std::vector<Triangle> clippedTriangles;

    // Check which vertices are inside and outside the clip plane
    for (const Eigen::Vector4d& vertex : {triangle.v1, triangle.v2, triangle.v3}) {
        if (clipPlane.normal.dot(vertex) + clipPlane.distance >= 0) {
            insideVertices.push_back(vertex);
        } else {
            outsideVertices.push_back(vertex);
        }
    }

    if (insideVertices.size() == 0) {
        // Completely outside the clip plane; discard the triangle
        return clippedTriangles;
    }

    if (insideVertices.size() == 3) {
        // Completely inside the clip plane; return the original triangle
        clippedTriangles.push_back(triangle);
        return clippedTriangles;
    }

    if (insideVertices.size() == 1) {
        // One vertex inside the clip plane; create a new triangle
        Eigen::Vector4d A = insideVertices[0];
        Eigen::Vector4d B = outsideVertices[0];
        Eigen::Vector4d C = outsideVertices[1];

        Eigen::Vector4d AB_intersection = A + (B - A) * (-clipPlane.distance - clipPlane.normal.dot(A)) / clipPlane.normal.dot(B - A);
        Eigen::Vector4d AC_intersection = A + (C - A) * (-clipPlane.distance - clipPlane.normal.dot(A)) / clipPlane.normal.dot(C - A);

        clippedTriangles.push_back(Triangle(A, AB_intersection, AC_intersection, triangle.shadeVal));
        return clippedTriangles;
    }

    if (insideVertices.size() == 2) {
        // Two vertices inside the clip plane; create a new quad (two triangles)
        Eigen::Vector4d A = insideVertices[0];
        Eigen::Vector4d B = insideVertices[1];
        Eigen::Vector4d C = outsideVertices[0];

        Eigen::Vector4d BC_intersection = B + (C - B) * (-clipPlane.distance - clipPlane.normal.dot(B)) / clipPlane.normal.dot(C - B);
        Eigen::Vector4d AC_intersection = A + (C - A) * (-clipPlane.distance - clipPlane.normal.dot(A)) / clipPlane.normal.dot(C - A);

        clippedTriangles.push_back(Triangle(A, B, BC_intersection, triangle.shadeVal));
        clippedTriangles.push_back(Triangle(A, BC_intersection, AC_intersection, triangle.shadeVal));
        return clippedTriangles;
    }

    // Should never reach this point
    return clippedTriangles;
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

void Renderer::visualizeDepthBuffer(SDL_Renderer* renderer) {
    for (int y = 0; y < WINDOW_HEIGHT; y++) {
        for (int x = 0; x < WINDOW_WIDTH; x++) {
            int index = x + y * WINDOW_WIDTH;
            double depth = depthBuffer[index];

            // Normalize the depth value to the range [0, 1]
            double normalizedDepth = (depth - NEAR) / (FAR - NEAR);

            // Clamp the normalized depth to the range [0, 1]
            normalizedDepth = std::clamp(normalizedDepth, 0.0, 1.0);

            // Convert the normalized depth to a grayscale color
            uint8_t grayscaleValue = static_cast<uint8_t>(normalizedDepth * 255);

            SDL_SetRenderDrawColor(renderer, grayscaleValue, grayscaleValue, grayscaleValue, 255);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}