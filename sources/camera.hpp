#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    struct perspective{
        float fov;
        float ratio;
        float zNear;
        float zFar;
    };

    Camera();
    ~Camera();

    void Init();
    void Terminate();
    void Update(const double frameDuration);

    perspective const& Perspective() const;

    glm::vec3 const& Position() const;
    void SetPosition(glm::vec3 const& position);

    glm::vec3 const& Direction() const;
    void SetDirection(glm::vec3 const& direction);

    glm::vec3 const& Up() const;
    void SetUp(glm::vec3 const& up);

    glm::mat4 const& View() const;
    glm::mat4 const& Projection() const;
    glm::mat4 const& ProjectionView() const;

    // Mouse and keyboard handle
    void EventKey(int key, int action);
    void HandleWindowResize(int width, int height);
    void HandleMousePosition(int x, int y);
    void HandleMouseButton(int button, int state);
    void HandleMouseWheel(int wheel);

private:
    bool mUpdateView;
    bool mUpdateProjection;
    bool mMousePan;

    int mMoveMask;

    perspective mPerspective;

    glm::vec2 mMousePosition;

    glm::vec3 mPosition;
    glm::vec3 mDirection;
    glm::vec3 mOrthoDirection;
    glm::vec3 mUp;

    glm::mat4 mView;
    glm::mat4 mProjection;
    glm::mat4 mProjectionView;
};

#endif
