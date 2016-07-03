#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "config.hpp"
#include "iupdater.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera : public IUpdater {
public:
    struct perspective{
        float fov;
        float ratio;
        float zNear;
        float zFar;
    };

    struct frustum {
        float left;
        float right;
        float bottom;
        float top;
        float zNear;
        float zFar;
    };

    Camera();
    ~Camera();

    void FrameStep() override;
    void Update(const float frameDuration) override;
    void Move(const float speed);

    static frustum ConvertTo(perspective const& perspective); 
    perspective const& Perspective() const;
    glm::ivec2 const& ScreenSize() const;

    glm::vec3 const& Position() const;
    void SetPosition(glm::vec3 const& position);

    glm::vec3 const& Direction() const;
    void SetDirection(glm::vec3 const& direction);

    glm::vec3 const& MouseDirection() const;
    glm::vec3 ProjectScreenCoordToWorld(const glm::vec2& screenCoord) const;
    glm::vec3 ProjectScreenCoordNormalizedToWorld(const glm::vec2& screenCoord) const;

    glm::vec3 const& Up() const;
    void SetUp(glm::vec3 const& up);

    glm::mat4 const& View() const;
    glm::mat4 const& ViewInv() const;
    glm::mat4 const& Projection() const;
    glm::mat4 const& ProjectionView() const;
    glm::mat4 const& ProjectionViewInv() const;

    // Mouse and keyboard handle
    void EventKey(int key, int action);
    void HandleWindowResize(int width, int height);
    void HandleMousePosition(double x, double y);
    void HandleMouseButton(int button, int state);
    void HandleMouseWheel(double wheel);

#ifdef IMGUI_ENABLE
    void debug_GUI();
#endif

private:
    bool mUpdateFixedFrameRate;
    bool mUpdateView;
    bool mUpdateProjection;
    bool mMousePan;

    int mMoveMask;
    float mSpeed;

    glm::ivec2 mScreenSize;

    perspective mPerspective;

    glm::vec2 mMousePosition;
    glm::vec3 mMouseDirectionWorld;

    glm::vec3 mPosition;
    glm::vec3 mDirection;
    glm::vec3 mOrthoDirection;
    glm::vec3 mUp;

    glm::mat4 mView;
    glm::mat4 mProjection;
    glm::mat4 mProjectionView;
    glm::mat4 mViewInv;
    glm::mat4 mProjectionInv;
    glm::mat4 mProjectionViewInv;
};

#endif
