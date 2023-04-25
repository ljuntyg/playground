#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>

#include "renderer.h"
#include "matrices.h"

std::vector<double> Renderer::depthBuffer(WINDOW_WIDTH * WINDOW_HEIGHT, std::numeric_limits<double>::max());

Uint32 Renderer::frameCounter = 0;
Uint32 Renderer::fpsUpdateTime = 0;
int Renderer::fps = 0;

double Renderer::cameraYaw = 0;
double Renderer::cameraPitch = 0;

Eigen::Vector4d Renderer::lookDir(0, 0, 1, 1);
Eigen::Vector4d Renderer::cameraPos(0, 0, -50, 1); // Changed from negative z to positive, positive z goes "out" of the screen?
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

    // Define the clip planes (Fix, only the near plane is kind of correct, no edge clipping now)
    std::vector<std::tuple<Eigen::Vector4d, Eigen::Vector4d>> clipPlanes = {
        std::tuple(Eigen::Vector4d(0, 0, 0, 1), Eigen::Vector4d(0, 1, 0, 1)),
		std::tuple(Eigen::Vector4d(0, (double)WINDOW_HEIGHT - 1, 0, 1), Eigen::Vector4d(0, -1, 0, 1)),
        std::tuple(Eigen::Vector4d(0, 0, 0, 1), Eigen::Vector4d(1, 0, 0, 1)),
        std::tuple(Eigen::Vector4d((double)WINDOW_WIDTH - 1, 0, 0, 1), Eigen::Vector4d(-1, 0, 0, 1)),
        std::tuple(Eigen::Vector4d(0, 0, -NEAR, 1), Eigen::Vector4d(0, 0, -1, 1)), // Near, FLIPPED Z TO NEG, HAVE TO FLIP NEAR AS WELL
        std::tuple(Eigen::Vector4d(0, 0, -FAR, 1), Eigen::Vector4d(0, 0, 1, 1)) // Far, FLIPPED Z AND FAR DUE TO ABOVE NEAR CHANGE, NO FAR CLIPPING IMPLEMENTED
    };

    // Common vector for all triangles to draw
    std::vector<Triangle> finalTriangles;

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

        Eigen::Vector4d viewedV1 = viewMatrix * translatedV1;
        Eigen::Vector4d viewedV2 = viewMatrix * translatedV2;
        Eigen::Vector4d viewedV3 = viewMatrix * translatedV3;

        //std::cout << "viewed tri v1, about to be near clipped:\n" << viewedV1 << std::endl;

        std::vector<Triangle> clippedNearTris = clipTriangleAgainstPlane(std::get<0>(clipPlanes[4]), std::get<1>(clipPlanes[4]), Triangle(viewedV1, viewedV2, viewedV3, normLightDotProd));
        //std::cout << "clipped near size: " << clippedNearTris.size() << std::endl;

        for (const Triangle& clippedNearTri : clippedNearTris) {
            // Transform and project the triangle
            Eigen::Vector4d projectedV1 = projectionMatrix * clippedNearTri.v1;
            Eigen::Vector4d projectedV2 = projectionMatrix * clippedNearTri.v2;
            Eigen::Vector4d projectedV3 = projectionMatrix * clippedNearTri.v3;

            // Convert to screen space coordinates
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

            Triangle projectedTriangle = Triangle(projectedV1, projectedV2, projectedV3, normLightDotProd);

            //std::cout << "proj triangle v1, screen space?\n" << projectedTriangle.v1 << "\n" << std::endl;

            finalTriangles.emplace_back(projectedTriangle);
        }
    }

    for (const Triangle& finalTriangle : finalTriangles) {
        // All tris in finalTris are already clipped against near, ignore far for now, clip against edges
        std::vector<Triangle> frustumClippedTris;
        std::vector<Triangle> temp;
        for (int i = 0; i < 4; i++) {
            switch (i) {
            case 0: temp = clipTriangleAgainstPlane(std::get<0>(clipPlanes[0]), std::get<1>(clipPlanes[0]), finalTriangle);
                frustumClippedTris.insert(frustumClippedTris.end(), temp.begin(), temp.end());
                break;
            
            case 1: temp = clipTriangleAgainstPlane(std::get<0>(clipPlanes[1]), std::get<1>(clipPlanes[1]), finalTriangle);
                frustumClippedTris.insert(frustumClippedTris.end(), temp.begin(), temp.end());
                break;

            case 2: temp = clipTriangleAgainstPlane(std::get<0>(clipPlanes[2]), std::get<1>(clipPlanes[2]), finalTriangle);
                frustumClippedTris.insert(frustumClippedTris.end(), temp.begin(), temp.end());
                break;

            case 3: temp = clipTriangleAgainstPlane(std::get<0>(clipPlanes[3]), std::get<1>(clipPlanes[3]), finalTriangle);
                frustumClippedTris.insert(frustumClippedTris.end(), temp.begin(), temp.end());
                break;
            }
        }

        for (const Triangle& frustumClippedTri : frustumClippedTris) {
            //std::cout << "frustum clipped tri v1:\n" << frustumClippedTri.v1 << "\n" << std::endl;
            drawTriangle(renderer, frustumClippedTri, getShadingColor(frustumClippedTri.shadeVal));
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

Eigen::Vector4d Renderer::linePlaneIntersection(const Eigen::Vector4d& planePoint, const Eigen::Vector4d& planeNormal, const Eigen::Vector4d& linePoint1, const Eigen::Vector4d& linePoint2) {
    // Convert points to 3D
    Eigen::Vector3d planeNormal3D = planeNormal.head<3>();
    Eigen::Vector3d planePoint3D = planePoint.head<3>() / planePoint.w();
    Eigen::Vector3d linePoint3D1 = linePoint1.head<3>() / linePoint1.w();
    Eigen::Vector3d linePoint3D2 = linePoint2.head<3>() / linePoint2.w();

    // Compute the line direction vector
    Eigen::Vector3d lineDirection = (linePoint3D2 - linePoint3D1).normalized();

    // Compute the denominator of the intersection equation
    double denom = planeNormal3D.dot(lineDirection);

    /* // If the denominator is close to zero, the line is parallel to the plane and there's no intersection
    if (std::abs(denom) < 1e-6) {
        throw std::runtime_error("Line is parallel to the plane, no intersection found.");
    } */

    // Compute the numerator of the intersection equation
    double numer = planeNormal3D.dot(planePoint3D - linePoint3D1);

    // Compute the intersection distance along the line
    double t = numer / denom;

    // Compute the intersection point in 3D
    Eigen::Vector3d intersectionPoint3D = linePoint3D1 + t * lineDirection;

    // Convert the intersection point to homogeneous coordinates
    Eigen::Vector4d intersectionPoint = Eigen::Vector4d(intersectionPoint3D.x(), intersectionPoint3D.y(), intersectionPoint3D.z(), 1.0);

    return intersectionPoint;
}

double Renderer::pointPlaneDistance(const Eigen::Vector4d& planePoint, const Eigen::Vector4d& planeNormal, const Eigen::Vector4d& point) {
    Eigen::Vector3d point3D = point.head<3>() / point.w();
    Eigen::Vector3d planeNormal3D = planeNormal.head<3>();
    Eigen::Vector3d planePoint3D = planePoint.head<3>() / planePoint.w();
    return planeNormal3D.dot(point3D - planePoint3D);
}

std::vector<Triangle> Renderer::clipTriangleAgainstPlane(const Eigen::Vector4d& planePoint, const Eigen::Vector4d& planeNormal, const Triangle& triangle) {
    // Compute the signed distances from each triangle vertex to the plane
    double d1 = pointPlaneDistance(planePoint, planeNormal, triangle.v1);
    double d2 = pointPlaneDistance(planePoint, planeNormal, triangle.v2);
    double d3 = pointPlaneDistance(planePoint, planeNormal, triangle.v3);

    std::vector<Triangle> clippedTriangles;

    // Case 1: No points inside the plane
    if (d1 <= 0 && d2 <= 0 && d3 <= 0) {
        return clippedTriangles; // Empty vector, the triangle is fully outside the plane
    }

    // Case 2: All points inside the plane
    if (d1 >= 0 && d2 >= 0 && d3 >= 0) {
        clippedTriangles.push_back(triangle);
        return clippedTriangles;
    }

    // Case 3: Two points inside the plane
    if ((d1 >= 0 && d2 >= 0) || (d1 >= 0 && d3 >= 0) || (d2 >= 0 && d3 >= 0)) {
        Triangle newTriangle;
        newTriangle.shadeVal = 1;
        if (d1 >= 0 && d2 >= 0) {
            newTriangle.v1 = triangle.v1;
            newTriangle.v2 = triangle.v2;
            newTriangle.v3 = linePlaneIntersection(planePoint, planeNormal, triangle.v2, triangle.v3);
            clippedTriangles.push_back(newTriangle);

            newTriangle.v1 = triangle.v1;
            newTriangle.v2 = newTriangle.v3;
            newTriangle.v3 = linePlaneIntersection(planePoint, planeNormal, triangle.v1, triangle.v3);
            clippedTriangles.push_back(newTriangle);
        } else if (d1 >= 0 && d3 >= 0) {
            newTriangle.v1 = triangle.v1;
            newTriangle.v2 = triangle.v3;
            newTriangle.v3 = linePlaneIntersection(planePoint, planeNormal, triangle.v1, triangle.v2);
            clippedTriangles.push_back(newTriangle);

            newTriangle.v1 = triangle.v1;
            newTriangle.v2 = newTriangle.v3;
            newTriangle.v3 = linePlaneIntersection(planePoint, planeNormal, triangle.v3, triangle.v2);
            clippedTriangles.push_back(newTriangle);
        } else { // d2 >= 0 && d3 >= 0
            newTriangle.v1 = triangle.v2;
            newTriangle.v2 = triangle.v3;
            newTriangle.v3 = linePlaneIntersection(planePoint, planeNormal, triangle.v3, triangle.v1);
            clippedTriangles.push_back(newTriangle);

            newTriangle.v1 = triangle.v2;
            newTriangle.v2 = newTriangle.v3;
            newTriangle.v3 = linePlaneIntersection(planePoint, planeNormal, triangle.v2, triangle.v1);
            clippedTriangles.push_back(newTriangle);
        }
        return clippedTriangles;
    }

    // Case 4: Two points outside the plane
    if ((d1 <= 0 && d2 <= 0) || (d1 <= 0 && d3 <= 0) || (d2 <= 0 && d3 <= 0)) {
        Triangle newTriangle;
        newTriangle.shadeVal = 1;
        if (d1 >= 0) {
            newTriangle.v1 = triangle.v1;
            newTriangle.v2 = linePlaneIntersection(planePoint, planeNormal, triangle.v1, triangle.v2);
            newTriangle.v3 = linePlaneIntersection(planePoint, planeNormal, triangle.v1, triangle.v3);
            clippedTriangles.push_back(newTriangle);
        } else if (d2 >= 0) {
            newTriangle.v1 = triangle.v2;
            newTriangle.v2 = linePlaneIntersection(planePoint, planeNormal, triangle.v2, triangle.v1);
            newTriangle.v3 = linePlaneIntersection(planePoint, planeNormal, triangle.v2, triangle.v3);
            clippedTriangles.push_back(newTriangle);
        } else { // d3 >= 0
            newTriangle.v1 = triangle.v3;
            newTriangle.v2 = linePlaneIntersection(planePoint, planeNormal, triangle.v3, triangle.v1);
            newTriangle.v3 = linePlaneIntersection(planePoint, planeNormal, triangle.v3, triangle.v2);
            clippedTriangles.push_back(newTriangle);
        }
        return clippedTriangles;
    }

    // This point should never be reached
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