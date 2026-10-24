#include "controller/TransformController.h"
#include "model/Camera.h"
#include "model/Mesh.h"
#include "model/Transform.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace
{

    // Intersect a ray with a plane defined as dot(n, p) = d.
    bool rayPlane(const Ray &r, const glm::vec3 &n, float d, glm::vec3 &hit)
    {
        const float denom = glm::dot(n, r.dir);
        if (std::abs(denom) < 1e-6f)
            return false;
        const float t = (d - glm::dot(n, r.origin)) / denom;
        if (t < 0.0f)
            return false;
        hit = r.origin + t * r.dir;
        return true;
    }

} // namespace

void TransformController::update(GLFWwindow *window, Camera &camera,
                                 const Mesh &mesh, Transform &transform,
                                 float vpOffsetX, float vpOffsetY,
                                 float viewportW, float viewportH)
{
    double mx = 0.0, my = 0.0;
    glfwGetCursorPos(window, &mx, &my);

    const float sx = static_cast<float>(mx - vpOffsetX);
    const float sy = static_cast<float>(my - vpOffsetY);

    const bool rmb =
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    // --- Begin drag ---
    if (rmb && !m_dragging)
    {
        if (sx < 0.0f || sx > viewportW || sy < 0.0f || sy > viewportH)
            return;

        const Ray worldRay = camera.screenToRay(sx, sy, viewportW, viewportH);

        // Convert to model space
        const glm::mat4 modelMat = transform.matrix();
        const glm::mat4 invModel = glm::inverse(modelMat);

        Ray localRay;
        localRay.origin = glm::vec3(invModel * glm::vec4(worldRay.origin, 1.0f));
        localRay.dir = glm::normalize(
            glm::vec3(invModel * glm::vec4(worldRay.dir, 0.0f)));

        float tHit = 0.0f;
        if (rayAabb(localRay, mesh.minBounds, mesh.maxBounds, tHit))
        {
            m_dragging = true;
            m_lastX = mx;
            m_lastY = my;

            // Drag plane: passes through the object's origin,
            // faces the camera.
            m_dragPlaneNormal = glm::normalize(
                camera.position() - transform.position);
            m_dragPlaneD = glm::dot(m_dragPlaneNormal, transform.position);

            // Where on the plane did we grab?
            glm::vec3 grabWorld;
            if (rayPlane(worldRay, m_dragPlaneNormal, m_dragPlaneD, grabWorld))
            {
                m_grabOffset = grabWorld - transform.position;
            }
            else
            {
                m_grabOffset = glm::vec3(0.0f);
            }
        }
    }

    // --- Continue drag ---
    if (rmb && m_dragging)
    {
        const Ray worldRay = camera.screenToRay(sx, sy, viewportW, viewportH);

        glm::vec3 hit;
        if (rayPlane(worldRay, m_dragPlaneNormal, m_dragPlaneD, hit))
        {
            transform.position = hit - m_grabOffset;
        }

        m_lastX = mx;
        m_lastY = my;
    }

    // --- End drag ---
    if (!rmb && m_dragging)
    {
        m_dragging = false;
    }
}
