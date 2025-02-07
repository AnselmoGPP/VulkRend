#include "physics.hpp"
#include "toolkit.hpp"

#include "systems.hpp"


void s_Engine::update(float timeStep)
{
    #ifdef DEBUG_SYSTEM
        std::cout << typeid(this).name() << "::" << __func__ << std::endl;
    #endif
    
    c_Engine* c_eng = (c_Engine*)em->getSComponent(CT::engine);
    if (!c_eng) { std::cout << "Singleton component not found (s_Engine)" << std::endl; return; }

    c_eng->time += timeStep;
    c_eng->frameCount++;
}

void s_Input::update(float timeStep)
{
    #ifdef DEBUG_SYSTEM
        std::cout << typeid(this).name() << "::" << __func__ << std::endl;
    #endif

    c_Input*  c_inp  = (c_Input* ) em->getSComponent(CT::input);
    const c_Engine* c_eng = (c_Engine*) em->getSComponent(CT::engine);
    if (!c_inp || !c_eng) { std::cout << "Singleton component not found (s_Input)" << std::endl; return; }

    // KEYBOARD input:

    if (c_eng->io.getKey(GLFW_KEY_W) == GLFW_PRESS) c_inp->W_press = true;
    else c_inp->W_press = false;

    if (c_eng->io.getKey(GLFW_KEY_S) == GLFW_PRESS) c_inp->S_press = true;
    else c_inp->S_press = false;

    if (c_eng->io.getKey(GLFW_KEY_A) == GLFW_PRESS) c_inp->A_press = true;
    else c_inp->A_press = false;

    if (c_eng->io.getKey(GLFW_KEY_D) == GLFW_PRESS) c_inp->D_press = true;
    else c_inp->D_press = false;

    if (c_eng->io.getKey(GLFW_KEY_Q) == GLFW_PRESS) c_inp->Q_press = true;
    else c_inp->Q_press = false;

    if (c_eng->io.getKey(GLFW_KEY_E) == GLFW_PRESS) c_inp->E_press = true;
    else c_inp->E_press = false;

    if (c_eng->io.getKey(GLFW_KEY_UP) == GLFW_PRESS) c_inp->up_press = true;
    else c_inp->up_press = false;

    if (c_eng->io.getKey(GLFW_KEY_DOWN) == GLFW_PRESS) c_inp->down_press = true;
    else c_inp->down_press = false;

    if (c_eng->io.getKey(GLFW_KEY_LEFT) == GLFW_PRESS) c_inp->left_press = true;
    else c_inp->left_press = false;

    if (c_eng->io.getKey(GLFW_KEY_RIGHT) == GLFW_PRESS) c_inp->right_press = true;
    else c_inp->right_press = false;

    // MOUSE input:

    c_inp->LMB_justPressed = false;
    c_inp->RMB_justPressed = false;
    c_inp->MMB_justPressed = false;

    if (c_eng->io.getMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (c_inp->LMB_pressed == false)
            c_inp->LMB_justPressed = true;

        c_inp->LMB_pressed = true;
    }
    else // if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
        c_inp->LMB_pressed = false;

    if (c_eng->io.getMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        if (c_inp->RMB_pressed == false)
            c_inp->RMB_justPressed = true;

        c_inp->RMB_pressed = true;
    }
    else // if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
        c_inp->RMB_pressed = false;

    if (c_eng->io.getMouseButton(GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
    {
        if (c_inp->MMB_pressed == false)
            c_inp->MMB_justPressed = true;

        c_inp->MMB_pressed = true;
    }
    else // if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_RELEASE)
        c_inp->MMB_pressed = false;

    c_inp->lastX = c_inp->xpos;
    c_inp->lastY = c_inp->ypos;
    c_eng->io.getCursorPos(&c_inp->xpos, &c_inp->ypos);

    c_inp->yScrollOffset = c_eng->io.getYscrollOffset();
}

void s_Input::getKeyboardInput(IOmanager& io, c_Input* c_input, float timeStep)
{
    if (io.getKey(GLFW_KEY_W) == GLFW_PRESS) c_input->W_press = true;
    else c_input->W_press = false;

    if (io.getKey(GLFW_KEY_S) == GLFW_PRESS) c_input->S_press = true;
    else c_input->S_press = false;
        
    if (io.getKey(GLFW_KEY_A) == GLFW_PRESS) c_input->A_press = true;
    else c_input->A_press = false;

    if (io.getKey(GLFW_KEY_D) == GLFW_PRESS) c_input->D_press = true;
    else c_input->D_press = false;

    if (io.getKey(GLFW_KEY_Q) == GLFW_PRESS) c_input->Q_press = true;
    else c_input->Q_press = false;

    if (io.getKey(GLFW_KEY_E) == GLFW_PRESS) c_input->E_press = true;
    else c_input->E_press = false;

    if (io.getKey(GLFW_KEY_UP) == GLFW_PRESS) c_input->up_press = true;
    else c_input->up_press = false;

    if (io.getKey(GLFW_KEY_DOWN) == GLFW_PRESS) c_input->down_press = true;
    else c_input->down_press = false;

    if (io.getKey(GLFW_KEY_LEFT) == GLFW_PRESS) c_input->left_press = true;
    else c_input->left_press = false;

    if (io.getKey(GLFW_KEY_RIGHT) == GLFW_PRESS) c_input->right_press = true;
    else c_input->right_press = false;
}

void s_Input::getMouseInput(IOmanager& io, c_Input* c_input, float timeStep)
{
    c_input->LMB_justPressed = false;
    c_input->RMB_justPressed = false;
    c_input->MMB_justPressed = false;

    if (io.getMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (c_input->LMB_pressed == false)
            c_input->LMB_justPressed = true;

        c_input->LMB_pressed = true;
    }
    else // if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
        c_input->LMB_pressed = false;

    if (io.getMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        if (c_input->RMB_pressed == false)
            c_input->RMB_justPressed = true;

        c_input->RMB_pressed = true;
    }
    else // if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
        c_input->RMB_pressed = false;

    if (io.getMouseButton(GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
    {
        if (c_input->MMB_pressed == false)
            c_input->MMB_justPressed = true;

        c_input->MMB_pressed = true;
    }
    else // if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_RELEASE)
        c_input->MMB_pressed = false;
    
    c_input->lastX = c_input->xpos;
    c_input->lastY = c_input->ypos;
    io.getCursorPos(&c_input->xpos, &c_input->ypos);

    c_input->yScrollOffset = io.getYscrollOffset();
}

glm::mat4 s_Camera::getViewMatrix(glm::vec3& camPos, glm::vec3& front, glm::vec3& camUp)
{
    return glm::lookAt(camPos, camPos + front, camUp);      // Params: Eye position, center position, up axis.
}

glm::mat4 s_Camera::getProjectionMatrix(float aspectRatio, float fov, float nearViewPlane, float farViewPlane)
{
    glm::mat4 proj = glm::perspective(fov, aspectRatio, nearViewPlane, farViewPlane);   // Params: FOV, aspect ratio, near and far view planes.
    proj[1][1] *= -1;       // GLM returns the Y clip coordinate inverted.
    return proj;
}

void s_Camera::updateAxes(c_Camera* c_cam, glm::vec4& rotQuat)
{
    // Rotate
    c_cam->front = rotatePoint(rotQuat, c_cam->front);
    c_cam->right = rotatePoint(rotQuat, c_cam->right);
    c_cam->camUp = rotatePoint(rotQuat, c_cam->camUp);

    // Normalize & make perpendicular (to front)
    c_cam->right = glm::normalize(glm::cross(c_cam->front, c_cam->camUp));
    c_cam->camUp = glm::normalize(glm::cross(c_cam->right, c_cam->front));
    c_cam->front = glm::normalize(c_cam->front);
}

void s_Camera::updateAxes_worldUp(c_Camera* c_cam, glm::vec4& rotQuat, glm::vec3& worldUp)
{
    // Rotate
    c_cam->front = rotatePoint(rotQuat, c_cam->front);
    c_cam->right = rotatePoint(rotQuat, c_cam->right);
    c_cam->camUp = rotatePoint(rotQuat, c_cam->camUp);

    // Normalize & make perpendicular (to front)
    c_cam->right = glm::normalize(glm::cross(c_cam->front, worldUp));
    c_cam->camUp = glm::normalize(glm::cross(c_cam->right, c_cam->front));
    c_cam->front = glm::normalize(c_cam->front);
}

void s_Camera::update(float timeStep)
{
    #ifdef DEBUG_SYSTEM
        std::cout << typeid(this).name() << "::" << __func__ << std::endl;
    #endif

    c_Camera* c_cam = (c_Camera*)em->getSComponent(CT::camera);
    if (!c_cam) { std::cout << "Singleton component not found (s_Camera)" << std::endl; return; }

    switch (c_cam->mode)
    {
    case c_Camera::sphere:
        update_Sphere(timeStep, (c_Cam_Sphere*)c_cam); // (c_Cam_Sphere*)
        break;
    case c_Camera::plane_polar:
        update_Plane_polar(timeStep, (c_Cam_Plane_polar*)c_cam);
        break;
    case c_Camera::plane_polar_sphere:
        update_Plane_polar_sphere(timeStep, (c_Cam_Plane_polar_sphere*)c_cam);
        break;
    case c_Camera::plane_free:
        update_Plane_free(timeStep, (c_Cam_Plane_free*)c_cam);
        break;
    case c_Camera::fpv:
        update_FPV(timeStep, (c_Cam_FPV*)c_cam);
        break;
    default:
        break;
    }
}

void s_Camera::update_Sphere(float timeStep, c_Cam_Sphere* c_cam)
{
    #ifdef DEBUG_SYSTEM
        std::cout << typeid(this).name() << "::" << __func__ << std::endl;
    #endif

    //c_Camera* c_cam = (c_Camera*)em->getSComponent(CT::camera);
    const c_Input* c_input = (c_Input*)em->getSComponent(CT::input);
    const c_Engine* c_eng = (c_Engine*)em->getSComponent(CT::engine);
    if (!c_cam || !c_input || !c_eng) { std::cout << "Singleton component not found (s_SphereCam)" << std::endl; return; }
    
    // Cursor modes
    if (c_input->LMB_justPressed)
        c_eng->io.setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!c_input->LMB_pressed)
        c_eng->io.setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Keyboard moves
    float velocity = c_cam->keysSpeed * timeStep;
    float& yaw = c_cam->euler.z;				//!< cam yaw       Y|  R
    float& pitch = c_cam->euler.x;				//!< cam pitch      | /
    //float& roll = c_cam->euler.z;				//!< cam roll       |/____P

    if (c_input->W_press || c_input->up_press) c_cam->radius -= velocity;
    if (c_input->S_press || c_input->down_press) c_cam->radius += velocity;
    if (c_input->A_press || c_input->left_press) c_cam->center -= c_cam->right * velocity;
    if (c_input->D_press || c_input->right_press) c_cam->center += c_cam->right * velocity;
    if (c_input->Q_press) c_cam->center -= c_cam->camUp * velocity;
    if (c_input->E_press) c_cam->center += c_cam->camUp * velocity;

    // constrain Radius
    if (c_cam->radius < c_cam->minRadius) c_cam->radius = c_cam->minRadius;
    if (c_cam->radius > c_cam->maxRadius) c_cam->radius = c_cam->maxRadius;

    // Mouse moves
    if (c_input->LMB_pressed) {
        yaw -= c_input->xOffset() * c_cam->mouseSpeed;
        pitch -= c_input->yOffset() * c_cam->mouseSpeed;
    }
    
    // constrain Pitch
    if (abs(pitch) > c_cam->maxPitch)
        pitch > 0 ? pitch =  c_cam->maxPitch : pitch = -c_cam->maxPitch;

    // Mouse scroll
    if (c_input->yScrollOffset)
    {
        c_cam->fov -= (float)c_input->yScrollOffset * c_cam->scrollSpeed;
        if (c_cam->fov < c_cam->minFov) c_cam->fov = c_cam->minFov;
        if (c_cam->fov > c_cam->maxFov) c_cam->fov = c_cam->maxFov;
    }

    // Update cam vectors
    c_cam->front  = glm::vec3(-1, 0, 0);
    c_cam->right  = glm::vec3( 0, 1, 0);
    c_cam->camUp  = glm::vec3( 0, 0, 1);

    glm::vec4 rotQuat = productQuat(
        //getRotQuat(c_cam->front, roll),
        getRotQuat(c_cam->right, pitch),
        getRotQuat(c_cam->worldUp, yaw)
    );
    updateAxes(c_cam, rotQuat);

    // Update camPos
    c_cam->camPos = rotatePoint(rotQuat, glm::vec3(c_cam->radius, 0, 0));
    c_cam->camPos += c_cam->center;

    // Prevent error propagation
    c_cam->right = glm::normalize(glm::cross(c_cam->front, c_cam->worldUp));
    c_cam->camUp = glm::normalize(glm::cross(c_cam->right, c_cam->front));

    // Update View & Projection transformation matrices
    c_cam->view = getViewMatrix(c_cam->camPos, c_cam->front, c_cam->camUp);
    c_cam->proj = getProjectionMatrix(c_eng->getAspectRatio(), c_cam->fov, c_cam->nearViewPlane, c_cam->farViewPlane);
}

void s_Camera::update_Plane_polar(float timeStep, c_Cam_Plane_polar* c_cam)
{
    #ifdef DEBUG_SYSTEM
        std::cout << typeid(this).name() << "::" << __func__ << std::endl;
    #endif

    //c_Camera* c_cam = (c_Camera*)em->getSComponent(CT::camera);
    c_Input const* c_input = (c_Input*)em->getSComponent(CT::input);
    c_Engine const* c_eng = (c_Engine*)em->getSComponent(CT::engine);
    if (!c_cam || !c_input || !c_eng) { std::cout << "Singleton component not found (s_PolarCam)" << std::endl; return; }

    // Cursor modes
    if (c_input->LMB_justPressed)
        c_eng->io.setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!c_input->LMB_pressed)
        c_eng->io.setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Keyboard moves
    float velocity = c_cam->keysSpeed * timeStep;
    float& yaw = c_cam->euler.z;				//!< cam yaw       Y|  R
    float& pitch = c_cam->euler.x;				//!< cam pitch      | /
    //float& roll = c_cam->euler.z;				//!< cam roll       |/____P

    if (c_input->W_press || c_input->up_press) c_cam->camPos += c_cam->front * velocity;
    if (c_input->S_press || c_input->down_press) c_cam->camPos -= c_cam->front * velocity;
    if (c_input->A_press || c_input->left_press) c_cam->camPos -= c_cam->right * velocity;
    if (c_input->D_press || c_input->right_press) c_cam->camPos += c_cam->right * velocity;
    if (c_input->Q_press) c_cam->camPos -= c_cam->worldUp * velocity;
    if (c_input->E_press) c_cam->camPos += c_cam->worldUp * velocity;

    // Mouse moves
    if (c_input->LMB_pressed) {
        yaw -= c_input->xOffset() * c_cam->mouseSpeed;
        pitch -= c_input->yOffset() * c_cam->mouseSpeed;
    }

    if (abs(pitch) > 1.3) pitch > 0 ? pitch = 1.3 : pitch = -1.3;
    
    // Mouse scroll
    if (c_input->yScrollOffset)
    {
        c_cam->fov -= (float)c_input->yScrollOffset * c_cam->scrollSpeed;
        if (c_cam->fov < c_cam->minFov) c_cam->fov = c_cam->minFov;
        if (c_cam->fov > c_cam->maxFov) c_cam->fov = c_cam->maxFov;
    }

    // Update cam vectors
    c_cam->front = glm::vec3(1, 0, 0);
    c_cam->right = glm::vec3(0,-1, 0);
    c_cam->camUp = glm::vec3(0, 0, 1);

    glm::vec4 rotQuat = productQuat(
        //getRotQuat(c_cam->front, roll),
        getRotQuat(glm::vec3(0, -1, 0), pitch),
        getRotQuat(c_cam->worldUp, yaw)
    );
    updateAxes(c_cam, rotQuat);
    
    // Prevent error propagation
    c_cam->right = glm::normalize(glm::cross(c_cam->front, c_cam->worldUp));
    c_cam->camUp = glm::normalize(glm::cross(c_cam->right, c_cam->front));

    // Update View & Projection transformation matrices
    c_cam->view = getViewMatrix(c_cam->camPos, c_cam->front, c_cam->camUp);
    c_cam->proj = getProjectionMatrix(c_eng->getAspectRatio(), c_cam->fov, c_cam->nearViewPlane, c_cam->farViewPlane);
}

void s_Camera::update_Plane_polar_sphere(float timeStep, c_Cam_Plane_polar_sphere* c_cam)
{
    #ifdef DEBUG_SYSTEM
        std::cout << typeid(this).name() << "::" << __func__ << std::endl;
    #endif

    //c_Camera* c_cam = (c_Camera*)em->getSComponent(CT::camera);
    c_Input const* c_input = (c_Input*)em->getSComponent(CT::input);
    c_Engine const* c_eng = (c_Engine*)em->getSComponent(CT::engine);
    if (!c_cam || !c_input || !c_eng) { std::cout << "Singleton component not found (s_PlaneCam)" << std::endl; return; }

    // Cursor modes
    if (c_input->LMB_justPressed)
        c_eng->io.setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!c_input->LMB_pressed)
        c_eng->io.setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    /*
        Move types:
            - Camera changes position: Update camPos variable (maybe using rotation quaternions).
            - Camera changes orientation (updateAxes()): Euler angles (yaw, pitch, roll) are used to adjust orientation (front, right, camUp)
    */

    // Keyboard moves
    float velocity = c_cam->keysSpeed * timeStep;
    //float yaw = 0;				//!< cam yaw       Y|  R
    //float pitch = 0;				//!< cam pitch      | /
    //float roll = 0;				//!< cam roll       |/____P
    
    glm::vec3 sphereDir = glm::cross(c_cam->worldUp, glm::normalize(glm::cross(c_cam->front, c_cam->worldUp)));
    float verticalValue = glm::dot(c_cam->front, c_cam->worldUp);
    float frontValue = glm::dot(c_cam->front, sphereDir);
    glm::vec4 rotQuat1 = { 1, 0, 0, 0 };    // Rotation of camera position (x, y, z)
    glm::vec4 rotQuat2 = { 1, 0, 0, 0 };    // Rotation of camera orientation (front, right, camUp)
    float angle;

    if (c_input->W_press || c_input->up_press)
    {
        c_cam->camPos += c_cam->worldUp * verticalValue * velocity;
        angle = -(frontValue * velocity) / getDist(c_cam->camPos, glm::vec3(0));
        rotQuat1 = productQuat(rotQuat1, getRotQuat(c_cam->right, angle));
    }
    if (c_input->S_press || c_input->down_press)
    {
        c_cam->camPos -= c_cam->worldUp * verticalValue * velocity;
        angle = (frontValue * velocity) / getDist(c_cam->camPos, glm::vec3(0));
        rotQuat1 = productQuat(rotQuat1, getRotQuat(c_cam->right, angle));
    }
    if (c_input->A_press || c_input->left_press)
    {
        angle = -velocity / getDist(c_cam->camPos, glm::vec3(0));
        rotQuat1 = productQuat(rotQuat1, getRotQuat(glm::cross(c_cam->worldUp, c_cam->right), angle));
    }
    if (c_input->D_press || c_input->right_press)
    {
        angle = velocity / getDist(c_cam->camPos, glm::vec3(0));
        rotQuat1 = productQuat(rotQuat1, getRotQuat(glm::cross(c_cam->worldUp, c_cam->right), angle));
    }
    if (c_input->Q_press)
        c_cam->camPos -= velocity * c_cam->worldUp;
    if (c_input->E_press)
        c_cam->camPos += velocity * c_cam->worldUp;

    // Mouse moves
    if (c_input->LMB_pressed) {
        //yaw -= c_input->xOffset() * c_cam->mouseSpeed;
        //pitch -= c_input->yOffset() * c_cam->mouseSpeed;

        angle = -c_input->yOffset() * c_cam->mouseSpeed;
        rotQuat2 = productQuat(rotQuat2, getRotQuat(c_cam->right, angle));

        angle = -c_input->xOffset() * c_cam->mouseSpeed;
        rotQuat2 = productQuat(rotQuat2, getRotQuat(c_cam->worldUp, angle));
    }

    // Mouse scroll
    if (c_input->yScrollOffset)
    {
        c_cam->fov -= (float)c_input->yScrollOffset * c_cam->scrollSpeed;
        if (c_cam->fov < c_cam->minFov) c_cam->fov = c_cam->minFov;
        if (c_cam->fov > c_cam->maxFov) c_cam->fov = c_cam->maxFov;
    }

    // Apply rotations (to camPos and its orientation axes)
    c_cam->camPos = rotatePoint(rotQuat1, c_cam->camPos);
    updateAxes_worldUp(c_cam, rotQuat2, c_cam->worldUp);

    c_cam->worldUp = glm::normalize(c_cam->camPos);

    // Update View & Projection transformation matrices
    c_cam->view = getViewMatrix(c_cam->camPos, c_cam->front, c_cam->camUp);
    c_cam->proj = getProjectionMatrix(c_eng->getAspectRatio(), c_cam->fov, c_cam->nearViewPlane, c_cam->farViewPlane);
}

void s_Camera::update_Plane_free(float timeStep, c_Cam_Plane_free* c_cam)
{
    #ifdef DEBUG_SYSTEM
        std::cout << typeid(this).name() << "::" << __func__ << std::endl;
    #endif

    //c_Camera* c_cam = (c_Camera*)em->getSComponent(CT::camera);
    c_Input const* c_input = (c_Input*)em->getSComponent(CT::input);
    c_Engine const* c_eng = (c_Engine*)em->getSComponent(CT::engine);
    if (!c_cam || !c_input || !c_eng) { std::cout << "Singleton component not found (s_PlaneCam)" << std::endl; return; }

    // Cursor modes
    if (c_input->LMB_justPressed)
        c_eng->io.setInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!c_input->LMB_pressed)
        c_eng->io.setInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Keyboard moves
    float velocity = c_cam->keysSpeed * timeStep;
    float yaw   = 0;					//!< cam yaw       Y|  R
    float pitch = 0;				    //!< cam pitch      | /
    float roll  = 0;					//!< cam roll       |/____P
    
    if (c_input->W_press || c_input->up_press   ) c_cam->camPos += c_cam->front * velocity;
    if (c_input->S_press || c_input->down_press ) c_cam->camPos -= c_cam->front * velocity;
    if (c_input->A_press || c_input->left_press ) roll = -c_cam->spinSpeed * velocity;
    if (c_input->D_press || c_input->right_press) roll =  c_cam->spinSpeed * velocity;
    if (c_input->Q_press) yaw =  c_cam->spinSpeed * velocity;
    if (c_input->E_press) yaw = -c_cam->spinSpeed * velocity;
    
    // Mouse moves
    if (c_input->LMB_pressed) {
        yaw -= c_input->xOffset() * c_cam->mouseSpeed;
        pitch -= c_input->yOffset() * c_cam->mouseSpeed;
    }
    
    // Mouse scroll
    if (c_input->yScrollOffset)
    {
        c_cam->fov -= (float)c_input->yScrollOffset * c_cam->scrollSpeed;
        if (c_cam->fov < c_cam->minFov) c_cam->fov = c_cam->minFov;
        if (c_cam->fov > c_cam->maxFov) c_cam->fov = c_cam->maxFov;
    }
    
    // Update cam vectors
    glm::vec4 rotQuat = productQuat(
        getRotQuat(c_cam->front, roll),
        getRotQuat(c_cam->camUp, yaw),
        getRotQuat(c_cam->right, pitch)
    );
    updateAxes(c_cam, rotQuat);
    
    // Update View & Projection transformation matrices
    c_cam->view = getViewMatrix(c_cam->camPos, c_cam->front, c_cam->camUp);
    c_cam->proj = getProjectionMatrix(c_eng->getAspectRatio(), c_cam->fov, c_cam->nearViewPlane, c_cam->farViewPlane);
}

void s_Camera::update_FPV(float timeStep, c_Cam_FPV* c_cam)
{
    #ifdef DEBUG_SYSTEM
        std::cout << typeid(this).name() << "::" << __func__ << std::endl;
    #endif

}

void s_Lights::update(float timeStep)
{
    #ifdef DEBUG_SYSTEM
        std::cout << typeid(this).name() << "::" << __func__ << std::endl;
    #endif

    c_Lights* c_lights = (c_Lights*)em->getSComponent(CT::lights);
    const c_Sky* c_sky = (c_Sky*)em->getSComponent(CT::sky);
    const c_Camera* c_cam = (c_Camera*)em->getSComponent(CT::camera);
    if (!c_lights) { std::cout << "Singleton component not found (c_lights)" << std::endl; return; }

    unsigned count = c_lights->lights.numLights;
    unsigned i = 0;
    
    if (i < count && c_sky) c_lights->lights.posDir[i].direction = -c_sky->sunDir;    // Dir. from sun
    i++;

    if (i < count && c_sky) c_lights->lights.posDir[i].direction =  c_sky->sunDir;    // Dir. to sun
    i++;

    if (i < count && c_cam)
    {
        c_lights->lights.posDir[i].position = c_cam->camPos;                          // Flashlight from cam
        c_lights->lights.posDir[i].direction = c_cam->front;
    }
}

void s_Sky_XY::update(float timeStep)
{
    #ifdef DEBUG_SYSTEM
        std::cout << typeid(this).name() << "::" << __func__ << std::endl;
    #endif

    c_Sky* c_sky = (c_Sky*)em->getSComponent(CT::sky);
    c_Engine* c_eng = (c_Engine*)em->getSComponent(CT::engine);
    if (!c_sky || !c_eng) { std::cout << "Singleton component not found (s_Sky_XY)" << std::endl; return; }
    
    c_sky->skyAngle = -fmod(c_eng->time * c_sky->skySpeed + c_sky->skyAngle_0, 2 * pi);
    c_sky->sunAngle = -fmod(c_eng->time * c_sky->sunSpeed + c_sky->sunAngle_0, 2 * pi);
    c_sky->sunDir = { cos(c_sky->sunAngle), sin(c_sky->sunAngle), 0 };
}

void s_Move::updateSkyMove(c_ModelParams* c_mParams, const c_Move* c_mov, const c_Camera* c_cam, float angle, float dist)
{
    c_mParams->mp[0].pos.x = c_cam->camPos.x + cos(angle) * dist;
    c_mParams->mp[0].pos.y = c_cam->camPos.y + sin(angle) * dist;
    c_mParams->mp[0].pos.z = c_cam->camPos.z;

    c_mParams->mp[0].rotQuat = getRotQuat(glm::vec3(0, 0, 1), angle);

    //c_mParams->mp[0].rotQuat = productQuat(
    //    getRotQuat(glm::vec3(0, 1, 0), 3 * pi / 2),
    //    getRotQuat(glm::vec3(0, 0, 1), angle));
}

void s_Move::update(float timeStep)
{
    #ifdef DEBUG_SYSTEM
        std::cout << typeid(this).name() << "::" << __func__ << std::endl;
    #endif

    // Entities with the component
    std::vector<uint32_t> entities = em->getEntitySet(CT::move);
    
    // Singleton components
    const c_Camera* c_cam = (c_Camera*)em->getSComponent(CT::camera);
    const c_Sky* c_sky = (c_Sky*)em->getSComponent(CT::sky);
    if (!c_cam || !c_sky) std::cout << "Single component not found (s_Model)" << std::endl;

    // Traverse the entities
    const c_Move* c_mov;
    c_ModelParams* c_mParams;   // component to update

    for (uint32_t eId : entities)
    {
        c_mov = (c_Move*)em->getComponent(CT::move, eId);
        c_mParams = (c_ModelParams*)em->getComponent(CT::modelParams, eId);
        if (!c_mParams) { std::cout << "c_mParams not found" << std::endl; continue; }

        switch (c_mov->moveType)
        {
        case followCam:             // follow cam
            if (c_cam)
                c_mParams->mp[0].pos = c_cam->camPos - safeMod(c_cam->camPos, c_mov->jumpStep);
            break;

        case followCamXY:           // follow cam on axis XY
            if (c_cam)
                c_mParams->mp[0].pos = glm::vec3(
                    c_cam->camPos.x - safeMod(c_cam->camPos.x, c_mov->jumpStep),
                    c_cam->camPos.y - safeMod(c_cam->camPos.y, c_mov->jumpStep),
                    c_mParams->mp[0].pos.z );
            break;

        case skyOrbit:              // Update using c_Sky::skyAngle
            if (c_cam && c_sky)
                updateSkyMove(c_mParams, c_mov, c_cam, c_sky->skyAngle, 0);
            break;

        case sunOrbit:              // Update using c_Sky::sunAngle
            if (c_cam && c_sky)
                updateSkyMove(c_mParams, c_mov, c_cam, c_sky->sunAngle, c_sky->sunDist);
            break;

        default:
            std::cout << "MoveType not found" << std::endl;
            break;
        }
    }
}

void s_Model::update(float timeStep)
{
    #ifdef DEBUG_SYSTEM
        std::cout << typeid(this).name() << "::" << __func__ << std::endl;
    #endif

    // Entities with the component
    std::vector<uint32_t> entities = em->getEntitySet(CT::model);

    // Singleton components
    const c_Engine* c_eng = (c_Engine*)em->getSComponent(CT::engine);
    const c_Camera* c_cam = (c_Camera*)em->getSComponent(CT::camera);
    const c_Lights* c_lights = (c_Lights*)em->getSComponent(CT::lights);
    if (!c_eng || !c_cam || !c_lights) { std::cout << "Single component not found (s_Model)" << std::endl; return; }

    // Traverse the entities
    c_Model* c_model;
    const c_ModelParams* c_mParams;

    glm::vec4 cam_time;
    uint8_t* dest;
    int i;

    float aspectRatio = c_eng->getAspectRatio();
    glm::vec2 clipPlanes{ c_cam->nearViewPlane, c_cam->farViewPlane };
    glm::vec2 screenSize{ c_eng->getWidth(), c_eng->getHeight() };

    glm::mat4 MM;

    for (uint32_t eId : entities)
    {
        c_model = (c_Model*)em->getComponent(CT::model, eId);
        if (c_model->renderLast) c_eng->r.toLastDraw(((c_Model_normal*)c_model)->model);

        switch (c_model->ubo_type)
        {
        case UboType::noData:
            break;
        case UboType::mvp:      // MVP
            {
                c_mParams = (c_ModelParams*)em->getComponent(CT::modelParams, eId);
                if (c_mParams) c_eng->r.setRenders(((c_Model_normal*)c_model)->model, c_mParams->mp.size());
                else { std::cout << "c_mParams not found" << std::endl; break; }

                for (i = 0; i < ((c_Model_normal*)c_model)->model->activeInstances; i++)
                {
                    dest = ((c_Model_normal*)c_model)->model->vsUBO.getUBOptr(i);
                    memcpy(dest, &getModelMatrix(c_mParams->mp[i].scale, c_mParams->mp[i].rotQuat, c_mParams->mp[i].pos), size.mat4);
                    dest += size.mat4;
                    memcpy(dest, &c_cam->view, sizeof(c_cam->view));
                    dest += size.mat4;
                    memcpy(dest, &c_cam->proj, sizeof(c_cam->proj));
                }
                break;
            }
        case UboType::mvpncl:   // MVP, MN, camPos, lights
            {
                c_mParams = (c_ModelParams*)em->getComponent(CT::modelParams, eId);
                if (c_mParams) c_eng->r.setRenders(((c_Model_normal*)c_model)->model, c_mParams->mp.size());
                else { std::cout << "c_mParams not found" << std::endl; break; }
                
                for (i = 0; i < ((c_Model_normal*)c_model)->model->activeInstances; i++)
                {
                    dest = ((c_Model_normal*)c_model)->model->vsUBO.getUBOptr(i);
                    memcpy(dest, &getModelMatrix(c_mParams->mp[i].scale, c_mParams->mp[i].rotQuat, c_mParams->mp[i].pos), size.mat4);
                    dest += size.mat4;
                    memcpy(dest, &c_cam->view, sizeof(c_cam->view));
                    dest += size.mat4;
                    memcpy(dest, &c_cam->proj, sizeof(c_cam->proj));
                    dest += size.mat4;
                    memcpy(dest, &getModelMatrixForNormals(*(glm::mat4*)((c_Model_normal*)c_model)->model->vsUBO.getUBOptr(i)), size.mat4);
                    dest += size.mat4;
                    cam_time = { c_cam->camPos, c_eng->time };
                    memcpy(dest, &cam_time, sizeof(cam_time));
                    dest += size.vec4;
                    memcpy(dest, c_lights->lights.posDir, c_lights->lights.posDirBytes);
                }

                dest = ((c_Model_normal*)c_model)->model->fsUBO.getUBOptr(0);
                memcpy(dest, c_lights->lights.props, c_lights->lights.propsBytes);
                break;
            }
        case UboType::planet:
            {
                ((c_Model_planet*)c_model)->planet->updateState(c_cam->camPos, c_cam->view, c_cam->proj, c_lights->lights, c_eng->time, 100);   // <<< groundHeight
                ((c_Model_planet*)c_model)->planet->toLastDraw();
                break;
            }
        case UboType::atmosphere:
            {
                for (i = 0; i < ((c_Model_normal*)c_model)->model->activeInstances; i++)
                {
                    dest = ((c_Model_normal*)c_model)->model->vsUBO.getUBOptr(i);
                    memcpy(dest, &c_cam->fov, sizeof(c_cam->fov));
                    dest += size.vec4;
                    memcpy(dest, &aspectRatio, sizeof(aspectRatio));
                    dest += size.vec4;
                    memcpy(dest, &c_cam->camPos, sizeof(c_cam->camPos));
                    dest += size.vec4;
                    memcpy(dest, &c_cam->front, sizeof(c_cam->front));
                    dest += size.vec4;
                    memcpy(dest, &c_cam->camUp, sizeof(c_cam->camUp));
                    dest += size.vec4;
                    memcpy(dest, &c_cam->right, sizeof(c_cam->right));
                    dest += size.vec4;
                    memcpy(dest, &c_lights->lights.posDir[0].direction, sizeof(glm::vec3)); // sun direction
                    dest += size.vec4;
                    memcpy(dest, &clipPlanes, sizeof(clipPlanes));
                    dest += size.vec4;
                    memcpy(dest, &screenSize, sizeof(screenSize));
                    dest += size.vec4;
                    memcpy(dest, &c_cam->view, sizeof(c_cam->view));
                    dest += size.mat4;
                    memcpy(dest, &c_cam->proj, sizeof(c_cam->proj));
                }
                break;
            }
        default:
            {
                std::cout << "Wrong UBO type" << std::endl;
                break;
            }
        }
    }
}

void s_Distributor::update(float timeStep)
{
    // Entities with the component
    std::vector<uint32_t> entities = em->getEntitySet(CT::distributor);
    if (!entities.size()) return;

    // Singleton components
    const c_Camera* c_cam = (c_Camera*)em->getSComponent(CT::camera);
    if (!c_cam) { std::cout << "Single component not found (s_Model)" << std::endl; return; }

    // World component (planet)
    c_Model_planet* c_mPlanet = getPlanetComponent();
    if (!c_mPlanet) return;

    std::vector<const Chunk*> chunks;
    c_mPlanet->planet->getActiveLeafChunks(chunks, 0);      // Get all chunks

    // Precalculations
    glm::vec3 camNormal = glm::normalize(c_cam->camPos - c_mPlanet->planet->nucleus);  // Cam's normal is considered the normal for all items.
    glm::vec4 latLonQuat = getLatLonRotQuat(camNormal);                                // Rotation around world coordinates for all items.

    // Traverse the entities
    c_ModelParams* c_mParams;   // component to update
    c_Distributor* c_distrib;

    unsigned i, j, chunkId;
    const std::vector<float>* vertices;
    std::vector<float> vertices_subGeometry;
    glm::vec3 position;
    float slope;
    glm::vec4 randomQuat, normalQuat = { 1,0,0,0 };        // rotation for additional randomness / for adapting to terrain normal
    glm::vec3 terrainNormal, terrainVertNormal;
    bool getNormalQuat;
    std::vector<unsigned> keys;

    // Traverse each entity
    for (uint32_t eId : entities)
    {
        c_mParams = (c_ModelParams*)em->getComponent(CT::modelParams, eId);
        if (!c_mParams) continue;
        c_distrib = (c_Distributor*)em->getComponent(CT::distributor, eId);
        getNormalQuat = c_distrib->adaptToTerrainNormal;

        c_mParams->mp.clear();
        normalQuat = { 1,0,0,0 };

        // Traverse each chunk
        for (i = 0; i < chunks.size(); i++)
        {
            if (chunks[i]->depth < c_distrib->minDepth || chunks[i]->depth > c_distrib->maxDepth) continue;
            chunkId = chunks[i]->chunkID;

            if (c_distrib->filledChunks.find(chunkId) == c_distrib->filledChunks.end()) // If chunk's population was not found, compute it
            {
                vertices = chunks[i]->getVertices();

                // Traverse each vertex (3, 3, 3) > Compute all chunk's population
                for (j = 0; j < vertices->size(); j += 9)
                {
                    position = glm::vec3((*vertices)[j + 0], (*vertices)[j + 1], (*vertices)[j + 2]);
                    //if (!withinFOV(position, c_cam->camPos, c_cam->front, c_cam->fov * 1.2, 5)) continue;       // is outside fov?

                    terrainVertNormal = normalize(glm::vec3((*vertices)[j + 0], (*vertices)[j + 1], (*vertices)[j + 2]));
                    terrainNormal = normalize(glm::vec3((*vertices)[j + 3], (*vertices)[j + 4], (*vertices)[j + 5]));
                    slope = 1.f - glm::dot(terrainNormal, terrainVertNormal);                                   // 1 - dot(groundNormal, sphereNormal)
                    if (!c_distrib->itemSupported(position, slope, c_distrib->noisers)) continue;               // user condition

                    randomQuat = getSecondQuat(position, c_distrib->rotType);   // rotation around vertical axis

                    if (getNormalQuat)                                           // rotation for adapting to terrain normal
                        if (glm::dot(terrainVertNormal, terrainNormal) < 0.99)
                            normalQuat = getRotQuat(glm::cross(terrainVertNormal, terrainNormal), angleBetween(terrainVertNormal, terrainNormal));

                    c_distrib->filledChunks[chunkId].push_back(ModelParams(
                        getScale(position, c_distrib->maxScale),            // scale
                        productQuat(randomQuat, latLonQuat, normalQuat),    // rotation
                        position));                                         // position)
                }

                if (c_distrib->subGeometry)     // Compute population for additional vertices (between the existing ones).
                {
                    
                }
            }

            // Take visible objects from storage
            for (unsigned i = 0; i < c_distrib->filledChunks[chunkId].size(); i++)
            {
                if (!withinFOV(c_distrib->filledChunks[chunkId][i].pos, c_cam->camPos, c_cam->front, c_cam->fov * 1.2, 10)) continue;       // is outside fov?
                else c_mParams->mp.push_back(c_distrib->filledChunks[chunkId][i]);
            }
        }

        // Delete population from no-longer existing chunks
        keys.clear();

        for (auto it = c_distrib->filledChunks.begin(); it != c_distrib->filledChunks.end(); it++)
            if (!c_mPlanet->planet->contains(it->first)) keys.push_back(it->first);

        for(unsigned i = 0; i < keys.size(); i++)
            c_distrib->filledChunks.erase(keys[i]);
    }
}

bool s_Distributor::withinFOV(const glm::vec3& itemPos, const glm::vec3& camPos, const glm::vec3& camDir, float fov, float minDist) const
{
    /* Readable version
    glm::vec3 itemDir = glm::normalize(itemPos - camPos);
    float camDirSide = glm::dot(itemDir, camDir);
    float angle = acos(camDirSide);
    
    if (angle > fov) return false;
    else return true;
    */

    if (acos(glm::dot(glm::normalize(itemPos - camPos), camDir)) > fov &&
        getDist(camPos, itemPos) > minDist
        ) return false;
    return true;
}

glm::vec4 s_Distributor::getLatLonRotQuat(glm::vec3& normal)
{
    glm::vec3 normalXY = { normal.x, normal.y, 0 };

    float longitude = angleBetween(xAxis, normalXY);
    if (normalXY.y < 0) longitude *= -1.f;

    float latitude = angleBetween(normalXY, normal);
    if (normal.z < 0) latitude *= -1.f;

    return productQuat(
        getRotQuat(yAxis, (pi/2) - latitude),
        getRotQuat(zAxis, longitude));
}

glm::vec3 s_Distributor::getProjectionOnPlane(glm::vec3& normal, glm::vec3& vec)
{
    // B � (X � Y) / ||(X � Y)||^2

    // Proj(on Normal) = ((Vec � Normal) / ||Normal||^2) Normal
    // Vec = Proj(on Plane) + Proj(on Normal)
    // Proj(on Plane) = Vec - Proj(on Normal)

    glm::vec3 projOnNormal = glm::dot(vec, normal) * normal;
    return vec - projOnNormal;
}

c_Model_planet* s_Distributor::getPlanetComponent()
{
    std::vector<Component*> c_models = em->getComponents(CT::model);
    
    for (int i = 0; i < c_models.size(); i++)
        if (((c_Model*)c_models[i])->ubo_type == UboType::planet && ((c_Model_planet*)c_models[i])->planet->getNoiseGen())
            return (c_Model_planet*)c_models[i];

    return nullptr;
}

glm::vec4 s_Distributor::getSecondQuat(const glm::vec3& pos, RotType rotationType)
{
    switch (rotationType)
    {
    case zAxisRandom:     // Z axis, random
        return getRotQuat(zAxis, pos.x * pos.y * pos.z + pos.x + pos.y + pos.z);
        break;
    case allAxesRandom:     // all axes, random
        return productQuat(
            getRotQuat(xAxis, pos.x * pos.y + pos.z),
            getRotQuat(yAxis, pos.x * pos.z + pos.y),
            getRotQuat(zAxis, pos.y * pos.z + pos.x));
        break;
    case faceCam:     // face cam
        return noRotQuat;
        break;
    default:
        return noRotQuat;
        break;
    }
}

glm::vec3 s_Distributor::getScale(const glm::vec3& pos, unsigned maxScale)
{
    switch (maxScale)
    {
    case 0:
    case 1:     // scale == 1
        return glm::vec3(1);
        break;
    default:    // scale > 1
        return glm::vec3(1 +
            (glm::abs((int)(pos.x * pos.y * pos.z + pos.x + pos.y + pos.z)) % 9) *     // range [0, 10]
            maxScale / 10.f);
        break;
    }
}

bool s_Distributor::renderRequired(const Planet& planet, float minDepth, unsigned chunksCount)
{
    std::vector<const Chunk*> availableChunks;
    planet.getActiveLeafChunks(availableChunks, minDepth);

    if (availableChunks.size() == chunksCount) return false;
    else return true;
}