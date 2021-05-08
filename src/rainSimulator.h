#ifndef CGL_RAIN_SIMULATOR_H
#define CGL_RAIN_SIMULATOR_H

#include <nanogui/nanogui.h>
#include <memory>

#include "camera.h"
#include "particleSystem.h"
#include "collision/collisionObject.h"

using namespace nanogui;

struct UserShader;
enum ShaderTypeHint {
    WIREFRAME = 0, NORMALS = 1, PHONG = 2
};

enum ShaderType {
    GROUND_SHADER_IDX = 0,
    SPHERE_SHADER_IDX = 1,
    SPHERE_REF_SHADER_IDX = 2,
    MESH_SHADER_IDX = 3,
    MESH_REF_SHADER_IDX = 4,
    RAIN_SHADER_IDX = 5,
    RAINDROP_SHADER_IDX = 6,
    SPLASH_SHADER_IDX = 7,
};

class RainSimulator {
public:
    RainSimulator(std::string project_root, Screen *screen);

    ~RainSimulator();

    void init();

    void loadCollisionObjects(vector<CollisionObject *> *objects);

    virtual bool isAlive();

    virtual void drawContents();

    // Screen events

    virtual bool cursorPosCallbackEvent(double x, double y);

    virtual bool mouseButtonCallbackEvent(int button, int action, int modifiers);

    virtual bool keyCallbackEvent(int key, int scancode, int action, int mods);

    virtual bool dropCallbackEvent(int count, const char **filenames);

    virtual bool scrollCallbackEvent(double x, double y);

    virtual bool resizeCallbackEvent(int width, int height);

private:
    virtual void initGUI(Screen *screen);

    void drawMesh(GLShader &shader);

    static void dyn_texture(int frame_idx, GLuint handle, const unsigned char *data, int width, int height);
    static void dyn_texture_rgb(int frame_idx, GLuint handle, const unsigned char *data, int width, int height);

    void load_shaders();

    void load_textures();

    // Rain system
    ParticleSystem* rainSystem;

    // File management

    std::string m_project_root;

    // Camera methods

    virtual void resetCamera();

    virtual Matrix4f getProjectionMatrix();

    virtual Matrix4f getViewMatrix();

    // Default simulation values

    int frames_per_sec = 60;
    int simulation_steps = 1;

    CGL::Vector3D gravity = CGL::Vector3D(0, -9.8, 0);
    CGL::Vector3D wind = CGL::Vector3D(0.2, 0, 0.2);
    int t = 0; // counter for drawContents

    nanogui::Color color = nanogui::Color(0.3f, 0.3f, 0.3f, 1.0f);

    vector<CollisionObject *> *collision_objects;

    // OpenGL attributes

    int active_shader_idx = 0;

    vector<UserShader> shaders;
    vector<std::string> shaders_combobox_names;

    // OpenGL textures

    Vector3D m_gl_texture_1_size;
    Vector3D m_gl_texture_2_size;
    Vector3D m_gl_texture_3_size;
    Vector3D m_gl_texture_4_size;
    Vector3D m_gl_texture_5_size;
    Vector3D m_gl_texture_6_size;
    Vector3D m_gl_texture_7_size;
    Vector3D m_gl_texture_8_size;
    GLuint m_gl_texture_1;
    GLuint m_gl_texture_2;
    GLuint m_gl_texture_3;
    GLuint m_gl_texture_4;
    GLuint m_gl_texture_5;
    GLuint m_gl_texture_6;
    GLuint m_gl_texture_7;
    GLuint m_gl_texture_8;
    GLuint m_gl_cubemap_tex;

    // OpenGL customizable inputs

    double m_normal_scaling = 2.0;
    double m_height_scaling = 0.1;

    // Camera attributes

    CGL::Camera camera;
    CGL::Camera canonicalCamera;

    double view_distance;
    double canonical_view_distance;
    double min_view_distance;
    double max_view_distance;

    double scroll_rate;

    // Screen methods

    Screen *screen;

    void mouseLeftDragged(double x, double y);

    void mouseRightDragged(double x, double y);

    void mouseMoved(double x, double y);

    // Mouse flags

    bool left_down = false;
    bool right_down = false;
    bool middle_down = false;

    // Keyboard flags

    bool ctrl_down = false;

    // Simulation flags

    bool is_paused = true;

    // Screen attributes

    int mouse_x;
    int mouse_y;

    int screen_w;
    int screen_h;

    bool is_alive = true;

    Vector2i default_window_size = Vector2i(1024, 800);

    GLShader &prepareShader(int index);

    RaindropRenderer raindrop_renderer;
    SplashRenderer splash_renderer;
};

struct UserShader {
    UserShader(std::string display_name, std::shared_ptr<GLShader> nanogui_shader, ShaderTypeHint type_hint)
            : display_name(display_name), nanogui_shader(nanogui_shader), type_hint(type_hint) {
    }

    std::shared_ptr<GLShader> nanogui_shader;
    std::string display_name;
    ShaderTypeHint type_hint;

};

#endif // CGL_RAIN_SIMULATOR_H
