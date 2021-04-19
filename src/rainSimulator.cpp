#include <cmath>
#include <glad/glad.h>

#include <CGL/vector3D.h>
#include <nanogui/nanogui.h>

#include "rainSimulator.h"

#include "camera.h"
#include "collision/plane.h"
#include "collision/sphere.h"
#include "misc/camera_info.h"
#include "misc/file_utils.h"
// Needed to generate stb_image binaries. Should only define in exactly one source file importing stb_image.h.
#define STB_IMAGE_IMPLEMENTATION

#include "misc/stb_image.h"

using namespace nanogui;
using namespace std;

Vector3D load_texture(int frame_idx, GLuint handle, const char *where) {
    Vector3D size_retval;

    if (strlen(where) == 0) return size_retval;

    glActiveTexture(GL_TEXTURE0 + frame_idx);
    glBindTexture(GL_TEXTURE_2D, handle);


    int img_x, img_y, img_n;
    unsigned char *img_data = stbi_load(where, &img_x, &img_y, &img_n, 3);
    size_retval.x = img_x;
    size_retval.y = img_y;
    size_retval.z = img_n;
    if (frame_idx != 1) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_x, img_y, 0, GL_RGB, GL_UNSIGNED_BYTE, img_data);
    stbi_image_free(img_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return size_retval;
}

/* Moves a texture from memory to the GPU. `data` needs to be a RGB char array with 8 bits per
 * channel, 3 channels per pixel, padded to 4-byte boundary, and arranged in a row-major order. */
void RainSimulator::dyn_texture(int frame_idx, GLuint handle, const char *data, int width, int height) {
    glActiveTexture(GL_TEXTURE0 + frame_idx);
    glBindTexture(GL_TEXTURE_2D, handle);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
}

void load_cubemap(int frame_idx, GLuint handle, const std::vector<std::string> &file_locs) {
    glActiveTexture(GL_TEXTURE0 + frame_idx);
    glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
    for (int side_idx = 0; side_idx < 6; ++side_idx) {

        int img_x, img_y, img_n;
        unsigned char *img_data = stbi_load(file_locs[side_idx].c_str(), &img_x, &img_y, &img_n, 3);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side_idx, 0, GL_RGB, img_x, img_y, 0, GL_RGB, GL_UNSIGNED_BYTE,
                     img_data);
        stbi_image_free(img_data);
        std::cout << "Side " << side_idx << " has dimensions " << img_x << ", " << img_y << std::endl;

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }
}

void RainSimulator::load_textures() {
    glGenTextures(1, &m_gl_texture_1);
    glGenTextures(1, &m_gl_texture_2);
    glGenTextures(1, &m_gl_texture_3);
    glGenTextures(1, &m_gl_texture_4);
    glGenTextures(1, &m_gl_cubemap_tex);

    m_gl_texture_1_size = load_texture(1, m_gl_texture_1, (m_project_root + "/textures/texture_1.png").c_str());
    m_gl_texture_2_size = load_texture(2, m_gl_texture_2, (m_project_root + "/textures/texture_2.png").c_str());
    m_gl_texture_3_size = load_texture(3, m_gl_texture_3, (m_project_root + "/textures/texture_3.png").c_str());
    m_gl_texture_4_size = load_texture(4, m_gl_texture_4, (m_project_root + "/textures/texture_4.png").c_str());

    std::cout << "Texture 1 loaded with size: " << m_gl_texture_1_size << std::endl;
    std::cout << "Texture 2 loaded with size: " << m_gl_texture_2_size << std::endl;
    std::cout << "Texture 3 loaded with size: " << m_gl_texture_3_size << std::endl;
    std::cout << "Texture 4 loaded with size: " << m_gl_texture_4_size << std::endl;

    std::vector<std::string> cubemap_fnames = {
            m_project_root + "/textures/cube/posx.jpg",
            m_project_root + "/textures/cube/negx.jpg",
            m_project_root + "/textures/cube/posy.jpg",
            m_project_root + "/textures/cube/negy.jpg",
            m_project_root + "/textures/cube/posz.jpg",
            m_project_root + "/textures/cube/negz.jpg"
    };

    load_cubemap(5, m_gl_cubemap_tex, cubemap_fnames);
    std::cout << "Loaded cubemap texture" << std::endl;
}

void RainSimulator::load_shaders() {
    std::set<std::string> shader_folder_contents;
    bool success = FileUtils::list_files_in_directory(m_project_root + "/shaders", shader_folder_contents);
    if (!success) {
        std::cout << "Error: Could not find the shaders folder!" << std::endl;
    }

    std::string std_vert_shader = m_project_root + "/shaders/Default.vert";

    for (const std::string &shader_fname : shader_folder_contents) {
        std::string file_extension;
        std::string shader_name;

        FileUtils::split_filename(shader_fname, shader_name, file_extension);

        if (file_extension != "frag") {
            std::cout << "Skipping non-shader file: " << shader_fname << std::endl;
            continue;
        }

        std::cout << "Found shader file: " << shader_fname << std::endl;

        // Check if there is a proper .vert shader or not for it
        std::string vert_shader = std_vert_shader;
        std::string associated_vert_shader_path = m_project_root + "/shaders/" + shader_name + ".vert";
        if (FileUtils::file_exists(associated_vert_shader_path)) {
            vert_shader = associated_vert_shader_path;
        }

        std::shared_ptr<GLShader> nanogui_shader = make_shared<GLShader>();
        nanogui_shader->initFromFiles(shader_name, vert_shader,
                                      m_project_root + "/shaders/" + shader_fname);

        // Special filenames are treated a bit differently
        ShaderTypeHint hint;
        if (shader_name == "Wireframe") {
            hint = ShaderTypeHint::WIREFRAME;
            std::cout << "Type: Wireframe" << std::endl;
        } else if (shader_name == "Normal") {
            hint = ShaderTypeHint::NORMALS;
            std::cout << "Type: Normal" << std::endl;
        } else {
            hint = ShaderTypeHint::PHONG;
            std::cout << "Type: Custom" << std::endl;
        }

        UserShader user_shader(shader_name, nanogui_shader, hint);

        shaders.push_back(user_shader);
        shaders_combobox_names.push_back(shader_name);
    }

    // Assuming that it's there, use "Custom" by default
    for (size_t i = 0; i < shaders_combobox_names.size(); ++i) {
        if (shaders_combobox_names[i] == "Custom") {
            active_shader_idx = i;
            break;
        }
    }

    // Arrange so that the indices of ground/sphere/mesh shaders match the constants
    for (size_t i = 0; i < shaders.size(); ++i) {
        cout << i << shaders[i].display_name << endl;
        if (shaders[i].display_name == "Ground" && i != GROUND_SHADER_IDX) {
            UserShader& temp = shaders[i];
            shaders[i] = shaders[GROUND_SHADER_IDX];
            shaders[GROUND_SHADER_IDX] = temp;
        } else if (shaders[i].display_name == "Mesh" && i != MESH_SHADER_IDX) {
            UserShader& temp = shaders[i];
            shaders[i] = shaders[MESH_SHADER_IDX];
            shaders[MESH_SHADER_IDX] = temp;
        } else if (shaders[i].display_name == "Sphere" && i != SPHERE_SHADER_IDX) {
            UserShader& temp = shaders[i];
            shaders[i] = shaders[SPHERE_SHADER_IDX];
            shaders[SPHERE_SHADER_IDX] = temp;
        }
    }
}

RainSimulator::RainSimulator(std::string project_root, Screen *screen)
        : m_project_root(project_root) {
    this->screen = screen;

    this->load_shaders();
    this->load_textures();

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);
}

RainSimulator::~RainSimulator() {
    for (auto shader : shaders) {
        shader.nanogui_shader->free();
    }
    glDeleteTextures(1, &m_gl_texture_1);
    glDeleteTextures(1, &m_gl_texture_2);
    glDeleteTextures(1, &m_gl_texture_3);
    glDeleteTextures(1, &m_gl_texture_4);
    glDeleteTextures(1, &m_gl_cubemap_tex);

    if (collision_objects) delete collision_objects;
}

void RainSimulator::loadCollisionObjects(vector<CollisionObject *> *objects) { this->collision_objects = objects; }

/**
 * Initializes the rain simulation and spawns a new thread to separate
 * rendering from simulation.
 */
void RainSimulator::init() {

    // Initialize GUI
    screen->setSize(default_window_size);
    initGUI(screen);

    // Initialize particle system
    rainSystem = new ParticleSystem(512, 512, 100);

    // Initialize camera

    CGL::Collada::CameraInfo camera_info;
    camera_info.hFov = 50;
    camera_info.vFov = 35;
    camera_info.nClip = 0.01;
    camera_info.fClip = 10000;

    // TODO: figure out the best parameters in this part
    CGL::Vector3D target(1, 1 / 2, 1);
    CGL::Vector3D c_dir(0., 0., 0.);
    canonical_view_distance = 10;
    scroll_rate = canonical_view_distance / 10;

    view_distance = canonical_view_distance * 2;
    min_view_distance = canonical_view_distance / 10.0;
    max_view_distance = canonical_view_distance * 20.0;

    // canonicalCamera is a copy used for view resets

    camera.place(target, acos(c_dir.y), atan2(c_dir.x, c_dir.z), view_distance,
                 min_view_distance, max_view_distance);
    canonicalCamera.place(target, acos(c_dir.y), atan2(c_dir.x, c_dir.z),
                          view_distance, min_view_distance, max_view_distance);

    screen_w = default_window_size(0);
    screen_h = default_window_size(1);

    camera.configure(camera_info, screen_w, screen_h);
    canonicalCamera.configure(camera_info, screen_w, screen_h);
}

bool RainSimulator::isAlive() { return is_alive; }

/* if index = -1, the currently selected in GUI is used. */
GLShader &RainSimulator::prepareShader(int index) {
    if (index == -1) index = active_shader_idx;

    const UserShader &active_shader = shaders[index];

    // Prepare the camera projection matrix

    Matrix4f model;
    model.setIdentity();

    Matrix4f view = getViewMatrix();
    Matrix4f projection = getProjectionMatrix();

    Matrix4f viewProjection = projection * view;

    // Bind the active shader
    GLShader &shader = *active_shader.nanogui_shader;
    shader.bind();

    shader.setUniform("u_model", model);
    shader.setUniform("u_view_projection", viewProjection);

    Vector3D cam_pos = camera.position();
    shader.setUniform("u_color", color, false);
    shader.setUniform("u_cam_pos", Vector3f(cam_pos.x, cam_pos.y, cam_pos.z), false);
    shader.setUniform("u_light_pos", Vector3f(0.5, 2, 2), false);
    shader.setUniform("u_light_intensity", Vector3f(3, 3, 3), false);
    shader.setUniform("u_texture_1_size", Vector2f(m_gl_texture_1_size.x, m_gl_texture_1_size.y), false);
    shader.setUniform("u_texture_2_size", Vector2f(m_gl_texture_2_size.x, m_gl_texture_2_size.y), false);
    shader.setUniform("u_texture_3_size", Vector2f(m_gl_texture_3_size.x, m_gl_texture_3_size.y), false);
    shader.setUniform("u_texture_4_size", Vector2f(m_gl_texture_4_size.x, m_gl_texture_4_size.y), false);
    // Textures
    shader.setUniform("u_texture_1", 1, false);
    shader.setUniform("u_texture_2", 2, false);
    shader.setUniform("u_texture_3", 3, false);
    shader.setUniform("u_texture_4", 4, false);

    shader.setUniform("u_normal_scaling", m_normal_scaling, false);
    shader.setUniform("u_height_scaling", m_height_scaling, false);

    shader.setUniform("u_texture_cubemap", 5, false);

    return shader;
}

void RainSimulator::drawContents() {
    glEnable(GL_DEPTH_TEST);

    if (!is_paused) {
        vector<Vector3D> external_accelerations = {gravity};
        rainSystem->updateWind(wind);
        for (int i = 0; i < simulation_steps; i++) {
            rainSystem->simulate(frames_per_sec, simulation_steps, external_accelerations, collision_objects);
        }
        dyn_texture(1, m_gl_texture_1, rainSystem->wetMap, rainSystem->width, rainSystem->height);
    }

    // TODO: maybe support different shaders for multiple meshes/multiple spheres

    // Mesh
    GLShader &shader = prepareShader(MESH_SHADER_IDX);
    drawMesh(shader);

    // Plane & Sphere
    for (CollisionObject *co : *collision_objects) {
        if (typeid(*co) == typeid(Sphere)) {
            shader = prepareShader(SPHERE_SHADER_IDX);
        } else if (typeid(*co) == typeid(Plane)) {
            shader = prepareShader(GROUND_SHADER_IDX);
        } else {
            shader = prepareShader(-1);
        }
        co->render(shader);
    }
}

void RainSimulator::drawMesh(GLShader &shader) {

    // TODO: update this to render a mesh if possible

    /*int num_tris = cloth->clothMesh->triangles.size();

    MatrixXf positions(4, num_tris * 3);
    MatrixXf normals(4, num_tris * 3);
    MatrixXf uvs(2, num_tris * 3);
    MatrixXf tangents(4, num_tris * 3);

    for (int i = 0; i < num_tris; i++) {
        Triangle *tri = cloth->clothMesh->triangles[i];

        Vector3D p1 = tri->v1->position;
        Vector3D p2 = tri->v2->position;
        Vector3D p3 = tri->v3->position;

        Vector3D n1 = tri->v1->normal();
        Vector3D n2 = tri->v2->normal();
        Vector3D n3 = tri->v3->normal();

        positions.col(i * 3) << p1.x, p1.y, p1.z, 1.0;
        positions.col(i * 3 + 1) << p2.x, p2.y, p2.z, 1.0;
        positions.col(i * 3 + 2) << p3.x, p3.y, p3.z, 1.0;

        normals.col(i * 3) << n1.x, n1.y, n1.z, 0.0;
        normals.col(i * 3 + 1) << n2.x, n2.y, n2.z, 0.0;
        normals.col(i * 3 + 2) << n3.x, n3.y, n3.z, 0.0;

        uvs.col(i * 3) << tri->uv1.x, tri->uv1.y;
        uvs.col(i * 3 + 1) << tri->uv2.x, tri->uv2.y;
        uvs.col(i * 3 + 2) << tri->uv3.x, tri->uv3.y;

        tangents.col(i * 3) << 1.0, 0.0, 0.0, 1.0;
        tangents.col(i * 3 + 1) << 1.0, 0.0, 0.0, 1.0;
        tangents.col(i * 3 + 2) << 1.0, 0.0, 0.0, 1.0;
    }


    shader.uploadAttrib("in_position", positions, false);
    shader.uploadAttrib("in_normal", normals, false);
    shader.uploadAttrib("in_uv", uvs, false);
    shader.uploadAttrib("in_tangent", tangents, false);

    shader.drawArray(GL_TRIANGLES, 0, num_tris * 3);*/
}

// ----------------------------------------------------------------------------
// CAMERA CALCULATIONS
//
// OpenGL 3.1 deprecated the fixed pipeline, so we lose a lot of useful OpenGL
// functions that have to be recreated here.
// ----------------------------------------------------------------------------

void RainSimulator::resetCamera() { camera.copy_placement(canonicalCamera); }

Matrix4f RainSimulator::getProjectionMatrix() {
    Matrix4f perspective;
    perspective.setZero();

    double cam_near = camera.near_clip();
    double cam_far = camera.far_clip();

    double theta = camera.v_fov() * PI / 360;
    double range = cam_far - cam_near;
    double invtan = 1. / tanf(theta);

    perspective(0, 0) = invtan / camera.aspect_ratio();
    perspective(1, 1) = invtan;
    perspective(2, 2) = -(cam_near + cam_far) / range;
    perspective(3, 2) = -1;
    perspective(2, 3) = -2 * cam_near * cam_far / range;
    perspective(3, 3) = 0;

    return perspective;
}

Matrix4f RainSimulator::getViewMatrix() {
    Matrix4f lookAt;
    Matrix3f R;

    lookAt.setZero();

    // Convert CGL vectors to Eigen vectors
    // TODO: Find a better way to do this!

    CGL::Vector3D c_pos = camera.position();
    CGL::Vector3D c_udir = camera.up_dir();
    CGL::Vector3D c_target = camera.view_point();

    Vector3f eye(c_pos.x, c_pos.y, c_pos.z);
    Vector3f up(c_udir.x, c_udir.y, c_udir.z);
    Vector3f target(c_target.x, c_target.y, c_target.z);

    R.col(2) = (eye - target).normalized();
    R.col(0) = up.cross(R.col(2)).normalized();
    R.col(1) = R.col(2).cross(R.col(0));

    lookAt.topLeftCorner<3, 3>() = R.transpose();
    lookAt.topRightCorner<3, 1>() = -R.transpose() * eye;
    lookAt(3, 3) = 1.0f;

    return lookAt;
}

// ----------------------------------------------------------------------------
// EVENT HANDLING
// ----------------------------------------------------------------------------

bool RainSimulator::cursorPosCallbackEvent(double x, double y) {
    if (left_down && !middle_down && !right_down) {
        if (ctrl_down) {
            mouseRightDragged(x, y);
        } else {
            mouseLeftDragged(x, y);
        }
    } else if (!left_down && !middle_down && right_down) {
        mouseRightDragged(x, y);
    } else if (!left_down && !middle_down && !right_down) {
        mouseMoved(x, y);
    }

    mouse_x = x;
    mouse_y = y;

    return true;
}

bool RainSimulator::mouseButtonCallbackEvent(int button, int action,
                                             int modifiers) {
    switch (action) {
        case GLFW_PRESS:
            switch (button) {
                case GLFW_MOUSE_BUTTON_LEFT:
                    left_down = true;
                    break;
                case GLFW_MOUSE_BUTTON_MIDDLE:
                    middle_down = true;
                    break;
                case GLFW_MOUSE_BUTTON_RIGHT:
                    right_down = true;
                    break;
            }
            return true;

        case GLFW_RELEASE:
            switch (button) {
                case GLFW_MOUSE_BUTTON_LEFT:
                    left_down = false;
                    break;
                case GLFW_MOUSE_BUTTON_MIDDLE:
                    middle_down = false;
                    break;
                case GLFW_MOUSE_BUTTON_RIGHT:
                    right_down = false;
                    break;
            }
            return true;
    }

    return false;
}

void RainSimulator::mouseMoved(double x, double y) { y = screen_h - y; }

void RainSimulator::mouseLeftDragged(double x, double y) {
    float dx = x - mouse_x;
    float dy = y - mouse_y;

    camera.rotate_by(-dy * (PI / screen_h), -dx * (PI / screen_w));
}

void RainSimulator::mouseRightDragged(double x, double y) {
    camera.move_by(mouse_x - x, y - mouse_y, canonical_view_distance);
}

bool RainSimulator::keyCallbackEvent(int key, int scancode, int action,
                                     int mods) {
    ctrl_down = (bool) (mods & GLFW_MOD_CONTROL);

    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                is_alive = false;
                break;
            case 'r':
            case 'R':
                // reset the rain sim
                break;
            case ' ':
                resetCamera();
                break;
            case 'p':
            case 'P':
                is_paused = !is_paused;
                break;
            case 'n':
            case 'N':
                if (is_paused) {
                    is_paused = false;
                    drawContents();
                    is_paused = true;
                }
                break;
        }
    }

    return true;
}

bool RainSimulator::dropCallbackEvent(int count, const char **filenames) {
    return true;
}

bool RainSimulator::scrollCallbackEvent(double x, double y) {
    camera.move_forward(y * scroll_rate);
    return true;
}

bool RainSimulator::resizeCallbackEvent(int width, int height) {
    screen_w = width;
    screen_h = height;

    camera.set_screen_size(screen_w, screen_h);
    return true;
}

void RainSimulator::initGUI(Screen *screen) {
    Window *window;

    window = new Window(screen, "Simulation");
    window->setPosition(Vector2i(default_window_size(0) - 245, 15));
    window->setLayout(new GroupLayout(15, 6, 14, 5));

    // Simulation constants

    new Label(window, "Simulation", "sans-bold");

    {
        Widget *panel = new Widget(window);
        GridLayout *layout =
                new GridLayout(Orientation::Horizontal, 2, Alignment::Middle, 5, 5);
        layout->setColAlignment({Alignment::Maximum, Alignment::Fill});
        layout->setSpacing(0, 10);
        panel->setLayout(layout);

        new Label(panel, "frames/s :", "sans-bold");

        IntBox<int> *fsec = new IntBox<int>(panel);
        fsec->setEditable(true);
        fsec->setFixedSize(Vector2i(100, 20));
        fsec->setFontSize(14);
        fsec->setValue(frames_per_sec);
        fsec->setSpinnable(true);
        fsec->setCallback([this](int value) { frames_per_sec = value; });

        new Label(panel, "steps/frame :", "sans-bold");

        IntBox<int> *num_steps = new IntBox<int>(panel);
        num_steps->setEditable(true);
        num_steps->setFixedSize(Vector2i(100, 20));
        num_steps->setFontSize(14);
        num_steps->setValue(simulation_steps);
        num_steps->setSpinnable(true);
        num_steps->setMinValue(0);
        num_steps->setCallback([this](int value) { simulation_steps = value; });
    }

    // Gravity

    new Label(window, "Gravity", "sans-bold");

    {
        Widget *panel = new Widget(window);
        GridLayout *layout =
                new GridLayout(Orientation::Horizontal, 2, Alignment::Middle, 5, 5);
        layout->setColAlignment({Alignment::Maximum, Alignment::Fill});
        layout->setSpacing(0, 10);
        panel->setLayout(layout);

        new Label(panel, "x :", "sans-bold");

        FloatBox<double> *fb = new FloatBox<double>(panel);
        fb->setEditable(true);
        fb->setFixedSize(Vector2i(100, 20));
        fb->setFontSize(14);
        fb->setValue(gravity.x);
        fb->setUnits("m/s^2");
        fb->setSpinnable(true);
        fb->setCallback([this](float value) { gravity.x = value; });

        new Label(panel, "y :", "sans-bold");

        fb = new FloatBox<double>(panel);
        fb->setEditable(true);
        fb->setFixedSize(Vector2i(100, 20));
        fb->setFontSize(14);
        fb->setValue(gravity.y);
        fb->setUnits("m/s^2");
        fb->setSpinnable(true);
        fb->setCallback([this](float value) { gravity.y = value; });

        new Label(panel, "z :", "sans-bold");

        fb = new FloatBox<double>(panel);
        fb->setEditable(true);
        fb->setFixedSize(Vector2i(100, 20));
        fb->setFontSize(14);
        fb->setValue(gravity.z);
        fb->setUnits("m/s^2");
        fb->setSpinnable(true);
        fb->setCallback([this](float value) { gravity.z = value; });
    }

    // Wind

    new Label(window, "Wind", "sans-bold");

    {
        Widget *panel = new Widget(window);
        GridLayout *layout =
                new GridLayout(Orientation::Horizontal, 2, Alignment::Middle, 5, 5);
        layout->setColAlignment({Alignment::Maximum, Alignment::Fill});
        layout->setSpacing(0, 10);
        panel->setLayout(layout);

        new Label(panel, "x :", "sans-bold");

        FloatBox<double> *fb = new FloatBox<double>(panel);
        fb->setEditable(true);
        fb->setFixedSize(Vector2i(100, 20));
        fb->setFontSize(14);
        fb->setValue(wind.x);
        fb->setUnits("m/s");
        fb->setSpinnable(true);
        fb->setCallback([this](float value) { wind.x = value; });

        new Label(panel, "y :", "sans-bold");

        fb = new FloatBox<double>(panel);
        fb->setEditable(true);
        fb->setFixedSize(Vector2i(100, 20));
        fb->setFontSize(14);
        fb->setValue(wind.y);
        fb->setUnits("m/s");
        fb->setSpinnable(true);
        fb->setCallback([this](float value) { wind.y = value; });

        new Label(panel, "z :", "sans-bold");

        fb = new FloatBox<double>(panel);
        fb->setEditable(true);
        fb->setFixedSize(Vector2i(100, 20));
        fb->setFontSize(14);
        fb->setValue(wind.z);
        fb->setUnits("m/s");
        fb->setSpinnable(true);
        fb->setCallback([this](float value) { wind.z = value; });
    }

    window = new Window(screen, "Appearance");
    window->setPosition(Vector2i(15, 15));
    window->setLayout(new GroupLayout(15, 6, 14, 5));

    // Appearance

    {
        ComboBox *cb = new ComboBox(window, shaders_combobox_names);
        cb->setFontSize(14);
        cb->setCallback(
                [this, screen](int idx) { active_shader_idx = idx; });
        cb->setSelectedIndex(active_shader_idx);
    }

    // Shader Parameters

    new Label(window, "Color", "sans-bold");

    {
        ColorWheel *cw = new ColorWheel(window, color);
        cw->setColor(this->color);
        cw->setCallback(
                [this](const nanogui::Color &color) { this->color = color; });
    }

    new Label(window, "Parameters", "sans-bold");

    {
        Widget *panel = new Widget(window);
        GridLayout *layout =
                new GridLayout(Orientation::Horizontal, 2, Alignment::Middle, 5, 5);
        layout->setColAlignment({Alignment::Maximum, Alignment::Fill});
        layout->setSpacing(0, 10);
        panel->setLayout(layout);

        new Label(panel, "Normal :", "sans-bold");

        FloatBox<double> *fb = new FloatBox<double>(panel);
        fb->setEditable(true);
        fb->setFixedSize(Vector2i(100, 20));
        fb->setFontSize(14);
        fb->setValue(this->m_normal_scaling);
        fb->setSpinnable(true);
        fb->setCallback([this](float value) { this->m_normal_scaling = value; });

        new Label(panel, "Height :", "sans-bold");

        fb = new FloatBox<double>(panel);
        fb->setEditable(true);
        fb->setFixedSize(Vector2i(100, 20));
        fb->setFontSize(14);
        fb->setValue(this->m_height_scaling);
        fb->setSpinnable(true);
        fb->setCallback([this](float value) { this->m_height_scaling = value; });
    }
}