#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>

#include "renderer.h"
#include "matrices.h"
#include "main.h"

RenderMode Renderer::RENDER_MODE = MODE_NORMAL;

std::string Renderer::objFolder = "res"; // Must be in working directory
std::vector<std::string> Renderer::allObjNames = Main::getObjFiles(objFolder);
std::string Renderer::targetFile = "3d_city_sample_terrain.obj"; // Don't forget .obj extension
std::vector<Mesh> Renderer::targetObj = getTargetObj();
std::vector<Mesh> Renderer::getTargetObj() {
    objl::Loader loader;
    loader.LoadFile(objFolder + "/" + targetFile);
    return Main::objlMeshToCustomMesh(loader.LoadedMeshes);
}

std::vector<double> Renderer::depthBuffer(WINDOW_WIDTH * WINDOW_HEIGHT, std::numeric_limits<double>::max());
std::vector<ClipPlane> Renderer::clipPlanes = {
    ClipPlane({0, 0, 0, 1}, {0, 1, 0, 1}), // Top
    ClipPlane({0, (double)WINDOW_HEIGHT - 1, 0, 1}, {0, -1, 0, 1}), // Bottom
    ClipPlane({0, 0, 0, 1}, {1, 0, 0, 1}), // Left
    ClipPlane({(double)WINDOW_WIDTH - 1, 0, 0, 1}, {-1, 0, 0, 1}), // Right
    ClipPlane({0, 0, -NEAR, 1}, {0, 0, -1, 1}), // Near, FLIPPED Z TO NEG, HAVE TO FLIP NEAR AS WELL
    ClipPlane({0, 0, -FAR, 1}, {0, 0, 1, 1}) // Far, FLIPPED Z AND FAR DUE TO ABOVE NEAR CHANGE, NO FAR CLIPPING IMPLEMENTED
};

Uint32 Renderer::frameCounter = 0;
Uint32 Renderer::frameTimeUpdateTime = 0;
double Renderer::frameTime = 0;

double Renderer::cameraYaw = 0;
double Renderer::cameraPitch = 0;

Eigen::Vector4d Renderer::lookDir(0, 0, 1, 1);
Eigen::Vector4d Renderer::cameraPos(0, 0, -10, 1); // Change to change camera start position
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

void Renderer::drawObject(SDL_Renderer* renderer, std::vector<Mesh>& object, double rotX, double rotY, double rotZ, double scale) {
    // Create transformation matrices
    Eigen::Matrix4d scaleMatrix = Matrices::createScale(scale, scale, scale);
    Eigen::Matrix4d rotXMatrix = Matrices::createRotationX(rotX);
    Eigen::Matrix4d rotYMatrix = Matrices::createRotationY(rotY);
    Eigen::Matrix4d rotZMatrix = Matrices::createRotationZ(rotZ);
    Eigen::Matrix4d translationMatrix = Matrices::createTranslation(0, 0, 0);

    // Model
    Eigen::Matrix4d matWorld = translationMatrix * scaleMatrix * rotXMatrix * rotYMatrix * rotZMatrix;

    // Create view matrix
    Eigen::Matrix4d viewMatrix = Matrices::createViewMatrix(cameraPos, targetPos, Eigen::Vector4d(0, 1, 0, 1));

    // Create projection matrix
    Eigen::Matrix4d projectionMatrix = Matrices::createPerspectiveProjection(FOV, WINDOW_WIDTH / WINDOW_HEIGHT, NEAR, FAR);

    // Common vector for all triangles to draw
    std::vector<Triangle> finalTriangles;

    for (const Mesh& mesh : object) {
        for (size_t i = 0; i < mesh.indices.size(); i += 3) {
            // Create a triangle from the current set of indices
            const Eigen::Vector4d& v1 = mesh.vertices[mesh.indices[i]];
            const Eigen::Vector4d& v2 = mesh.vertices[mesh.indices[i + 1]];
            const Eigen::Vector4d& v3 = mesh.vertices[mesh.indices[i + 2]];
            Triangle triangle(v1, v2, v3);

            // World Matrix Transform
            Eigen::Vector4d triTransformed_v1 = matWorld * v1;
            Eigen::Vector4d triTransformed_v2 = matWorld * v2;
            Eigen::Vector4d triTransformed_v3 = matWorld * v3;

            // Calculate triangle Normal
            Eigen::Vector3d line1 = (triTransformed_v2 - triTransformed_v1).head<3>();
            Eigen::Vector3d line2 = (triTransformed_v3 - triTransformed_v1).head<3>();
            Eigen::Vector3d normal = line1.cross(line2);
            normal.normalize();

            // Get Ray from triangle to camera
            Eigen::Vector3d vCameraRay = (triTransformed_v1.head<3>() - cameraPos.head<3>()).normalized();

            // Skip triangle if back face
            if (normal.dot(vCameraRay) > 0) {
                continue;
            }

            // Illumination
            Eigen::Vector3d light_direction = {0, 1, -1};
            light_direction.normalize();

            // Dot product for shasing
            double dp = std::max(0.1, normal.dot(light_direction.head<3>()));
            SDL_Color shadingColor = getShadingColor(dp);

            Eigen::Vector4d triViewedV1 = viewMatrix * triTransformed_v1;
            Eigen::Vector4d triViewedV2 = viewMatrix * triTransformed_v2;
            Eigen::Vector4d triViewedV3 = viewMatrix * triTransformed_v3;

            // Clip Viewed Triangle against near plane
            int nClippedTriangles = 0;
            Triangle clipped[2];
            Triangle triViewed(triViewedV1, triViewedV2, triViewedV3);
            nClippedTriangles = clipTriangleAgainstPlane(clipPlanes[4], triViewed, clipped[0], clipped[1]);

            // Project triangles from 3D to 2D
            for (int n = 0; n < nClippedTriangles; n++) {
                Triangle triProjected(clipped[n].v1, clipped[n].v2, clipped[n].v3, shadingColor);

                // Project vertices
                triProjected.v1 = projectionMatrix * triProjected.v1;
                triProjected.v2 = projectionMatrix * triProjected.v2;
                triProjected.v3 = projectionMatrix * triProjected.v3;

                    // Convert to normalized device coordinates
                triProjected.v1 /= triProjected.v1.w();
                triProjected.v2 /= triProjected.v2.w();
                triProjected.v3 /= triProjected.v3.w();

                // Convert to screen space coordinates
                triProjected.v1.x() = (triProjected.v1.x() + 1.0) * scaleFactor;
                triProjected.v1.y() = (1.0 - triProjected.v1.y()) * scaleFactor;
                triProjected.v2.x() = (triProjected.v2.x() + 1.0) * scaleFactor;
                triProjected.v2.y() = (1.0 - triProjected.v2.y()) * scaleFactor;
                triProjected.v3.x() = (triProjected.v3.x() + 1.0) * scaleFactor;
                triProjected.v3.y() = (1.0 - triProjected.v3.y()) * scaleFactor;

                finalTriangles.push_back(triProjected);
            }
                
        }
    }

    // Edge clip and draw triangles
    for (const Triangle &triangle : finalTriangles) {

        Triangle clipped[2];
        std::list<Triangle> listTriangles;

        // Add initial triangle
        listTriangles.push_back(triangle);
        int nNewTriangles = 1;

        for (int p = 0; p < 4; p++) {
            int nTrisToAdd = 0;
            while (nNewTriangles > 0) {

                Triangle test = listTriangles.front();
                listTriangles.pop_front();
                nNewTriangles--;

                switch (p) {
                    case 0:	nTrisToAdd = clipTriangleAgainstPlane(clipPlanes[p], test, clipped[0], clipped[1]); break;
                    case 1:	nTrisToAdd = clipTriangleAgainstPlane(clipPlanes[p], test, clipped[0], clipped[1]); break;
                    case 2:	nTrisToAdd = clipTriangleAgainstPlane(clipPlanes[p], test, clipped[0], clipped[1]); break;
                    case 3:	nTrisToAdd = clipTriangleAgainstPlane(clipPlanes[p], test, clipped[0], clipped[1]); break;
                }

                for (int w = 0; w < nTrisToAdd; w++)
                    listTriangles.push_back(clipped[w]);
            }
            nNewTriangles = listTriangles.size();
        }
        
        for (auto &t : listTriangles) {
            drawTriangle(renderer, t, RENDER_MODE);
        }
    }

    if (RENDER_MODE == MODE_SHOW_DEPTH) {
        visualizeDepthBuffer(renderer);
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
            double w2 = 1 - w0 - w1;

            // Compute depth value at this pixel
            double z = w0 * v0.z() + w1 * v1.z() + w2 * v2.z();

            // Check if the current fragment is closer to the camera than the one in the depth buffer
            int depthBufferIdx = y * WINDOW_WIDTH + x;
            if (z < depthBuffer[depthBufferIdx]) {
                // Update the depth buffer
                depthBuffer[depthBufferIdx] = z;

                // Draw the fragment, unless MODE_SHOW_DEPTH (it will cover any drawn triangles, so don't draw)
                if (RENDER_MODE != MODE_SHOW_DEPTH) {
                    SDL_RenderDrawPoint(renderer, x, y);
                }
            }
        }
    }
}

void Renderer::drawTriangle(SDL_Renderer* renderer, const Triangle& triangle, const RenderMode& mode) {
    switch (mode) {
    case MODE_NORMAL:
        drawTriangle(renderer, triangle, triangle.color);
        break;

    case MODE_SHOW_CLIPPING:
        // Draw with color, clip method will check mode
        drawTriangle(renderer, triangle, triangle.color);
        break;

    case MODE_SHOW_DEPTH:
        /* Call draw with color, but check in method if mode selected, 
        if so skip drawing but add to buffer, then check at end of 
        drawObject to decide whether to draw depth map or not */
        drawTriangle(renderer, triangle, triangle.color);
        break;

    case MODE_SHOW_WIREFRAME:
        // Call basic draw with just lines
        drawTriangle(renderer, triangle);
        break;
    
    default:
        std::cerr << "Error: Invalid mode" << std::endl;
        break;
    }
}

Eigen::Vector4d Renderer::linePlaneIntersection(const ClipPlane& clipPlane, const Eigen::Vector4d& lineStart, const Eigen::Vector4d& lineEnd) {
    Eigen::Vector4d planePoint {clipPlane.planePoint}, planeNormal {clipPlane.planeNormal};
    double t = planeNormal.dot(planePoint - lineStart) / planeNormal.dot(lineEnd - lineStart);
    return lineStart + t * (lineEnd - lineStart);
}

int Renderer::clipTriangleAgainstPlane(const ClipPlane& clipPlane, Triangle& triIn, Triangle& triOut1, Triangle& triOut2) {
    Eigen::Vector4d planePoint {clipPlane.planePoint}, planeNormal {clipPlane.planeNormal};
    auto dist = [&](const Eigen::Vector4d& p) {
        return planeNormal.dot(p) - planeNormal.dot(planePoint);
    };

    Eigen::Vector4d* inside_points[3];
    int nInsidePointCount = 0;
    Eigen::Vector4d* outside_points[3];
    int nOutsidePointCount = 0;

    double d0 = dist(triIn.v1);
    double d1 = dist(triIn.v2);
    double d2 = dist(triIn.v3);

    if (d0 >= 0) { inside_points[nInsidePointCount++] = &triIn.v1; } else { outside_points[nOutsidePointCount++] = &triIn.v1; }
    if (d1 >= 0) { inside_points[nInsidePointCount++] = &triIn.v2; } else { outside_points[nOutsidePointCount++] = &triIn.v2; }
    if (d2 >= 0) { inside_points[nInsidePointCount++] = &triIn.v3; } else { outside_points[nOutsidePointCount++] = &triIn.v3; }

    if (nInsidePointCount == 0) {
        return 0;
    }

    if (nInsidePointCount == 3) {
        triOut1 = triIn;
        return 1;
    }

    if (nInsidePointCount == 1 && nOutsidePointCount == 2) {

        // Check whether to visualize clipping
        if (RENDER_MODE == MODE_SHOW_CLIPPING) {
           triOut1.color = {0, 0, 255, 255}; 
        } else {
            triOut1.color = triIn.color;
        }

        triOut1.v1 = *inside_points[0];
        triOut1.v2 = linePlaneIntersection(clipPlane, *inside_points[0], *outside_points[0]);
        triOut1.v3 = linePlaneIntersection(clipPlane, *inside_points[0], *outside_points[1]);

        return 1;
    }

    if (nInsidePointCount == 2 && nOutsidePointCount == 1) {

        // Check whether to visualize clipping
        if (RENDER_MODE == MODE_SHOW_CLIPPING) {
            triOut1.color = {255, 0, 0, 255};
            triOut2.color = {0, 255, 0, 255};
        } else {
            triOut1.color = triIn.color;
            triOut2.color = triIn.color;
        }

        triOut1.v1 = *inside_points[0];
        triOut1.v2 = *inside_points[1];
        triOut1.v3 = linePlaneIntersection(clipPlane, *inside_points[0], *outside_points[0]);

        triOut2.v1 = *inside_points[1];
        triOut2.v2 = triOut1.v3;
        triOut2.v3 = linePlaneIntersection(clipPlane, *inside_points[1], *outside_points[0]);

        return 2;
    }

    throw std::runtime_error("Unexpected case in clipTriangleAgainstPlane");
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

void Renderer::visualizeDepthBuffer(SDL_Renderer* renderer) {
    for (int y = 0; y < WINDOW_HEIGHT; y++) {
        for (int x = 0; x < WINDOW_WIDTH; x++) {
            int index = x + y * WINDOW_WIDTH;
            double depth = depthBuffer[index];

            depth = 1 - ((depth - NEAR) / (FAR - NEAR)); // 0.999999

            depth = std::pow(depth, 9250) * ULLONG_MAX;

            // Clamp the normalized depth to the range [0, 1]
            depth = std::clamp(depth, 0.0, 1.0);

            // Map the normalized depth to a color using the jet colormap
            SDL_Color color = jet(depth);

            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}

SDL_Color Renderer::jet(double value) {
    double a = (1 - value) / 0.175;
    int x = static_cast<int>(std::floor(a));
    int y = static_cast<int>(std::floor(255 * (a - x)));

    SDL_Color color;
    switch (x) {
        case 0: color = {static_cast<uint8_t>(255), static_cast<uint8_t>(y), 0, 255}; break;
        case 1: color = {static_cast<uint8_t>(255 - y), static_cast<uint8_t>(255), 0, 255}; break;
        case 2: color = {0, static_cast<uint8_t>(255), static_cast<uint8_t>(y), 255}; break;
        case 3: color = {0, static_cast<uint8_t>(255 - y), static_cast<uint8_t>(255), 255}; break;
        default: color = {0, 0, 0, 255}; break;
    }

    return color;
}