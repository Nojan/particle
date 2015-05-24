#pragma once

#include "opengl_includes.hpp"
#include <memory>

class Texture2D;
class ShaderProgram;

class Skybox {
public:
    static Skybox* CreateSkyboxFrom(const char * xPosPath, const char * xNegPath,
                                    const char * yPosPath, const char * yNegPath,
                                    const char * zPosPath, const char * zNegPath);

    static Skybox* CreateSkyboxFrom(const char * directory);

    Skybox(Texture2D& xPos, Texture2D& xNeg, Texture2D& yPos, Texture2D& yNeg, Texture2D& zPos, Texture2D& zNeg);
    ~Skybox();

    void Draw(ShaderProgram * programShader);
    void Init();
    void Terminate();
    void Update();

#ifdef IMGUI_ENABLE
    void debug_GUI() const;
#endif

private:
    std::unique_ptr<ShaderProgram> mShaderProgram;
    GLuint mIndexBufferId;
    GLuint mVertexBufferId;
    GLuint uvbuffer;
    GLuint mTextureBufferId;
};
