#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
glm::vec3 lightDirPo;
glm::vec3 lightColorPo;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLfloat lightAngle;
GLint lightDirLocPo;
GLint lightColorLocPo;
bool lum;


gps::SkyBox mySkyBox;
gps::Shader skyboxShader;



//gps::Shader myCustomShader;
gps::Model3D myModel;
bool anim = true;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 4.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;
GLfloat cSpeed = 0.05f;


GLboolean pressedKeys[1024];


// models
gps::Model3D teapot;
gps::Model3D bird;
gps::Model3D ball;

GLfloat angle;
GLfloat birdAngle = 0.0f;
GLfloat ballS = 0.02f;
GLfloat ballSMax = 14.0f;
GLfloat ballSMin = 0.00001f;

// shaders
gps::Shader myBasicShader;

float fogDensity = 0.02f;
glm::mat4 lightRotation;



GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

double lastX = 0.0;
double lastY = 0.0;
float sesitivity = 0.1f;
bool firstMouse = true;
float pitch = 0;
float yaw = -90;

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
   //TODO
 
    if (firstMouse)
    {
       lastX = xpos;
       lastY = ypos;
       firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    myCamera.rotate(yoffset, xoffset);

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
   
}

void processMovement() {
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        anim = false;
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        anim = false;
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        anim = false;
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        anim = false;
    }

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        anim = false;
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        anim = false;
    }
    // for fog
    if (pressedKeys[GLFW_KEY_C]) {
        if (fogDensity < 0.30f) {
            fogDensity += 0.002f;
            myBasicShader.useShaderProgram();
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity"), fogDensity);
        }
    }

    if (pressedKeys[GLFW_KEY_V]) {
        if (fogDensity > 0.0f) {
            fogDensity -= 0.002f;
            myBasicShader.useShaderProgram();
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity"), fogDensity);
        }
    }

    if (pressedKeys[GLFW_KEY_H]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (pressedKeys[GLFW_KEY_J]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    if (pressedKeys[GLFW_KEY_K]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }

    if (pressedKeys[GLFW_KEY_L]) {
        anim = true;
    }

    if (pressedKeys[GLFW_KEY_O]) {
        lum = false;
        myBasicShader.useShaderProgram();
        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "lum"), lum);

    }

    if (pressedKeys[GLFW_KEY_P]) {
        lum = true;
        myBasicShader.useShaderProgram();
        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "lum"), lum);
    }

    if (pressedKeys[GLFW_KEY_U]) {
        lightColor *= 1.2f;
        myBasicShader.useShaderProgram();
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
        
    }

    if (pressedKeys[GLFW_KEY_Y]) {
        lightColor *= 0.8f;
        myBasicShader.useShaderProgram();
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    }


}


void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Ionas Andreea");
   // glfwSetCursorPosCallback(myWindow, mouseCallback);
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    //teapot.LoadModel("models/teapot/teapot20segUT.obj");
    teapot.LoadModel("models/world/world_done.obj");
    bird.LoadModel("models/bird/bird_fly.obj");
    ball.LoadModel("models/ball/ball_move.obj");

}

void initShaders() {
	myBasicShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
    myBasicShader.useShaderProgram();
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
}

void initSkybox()
{
    std::vector<const GLchar*> faces;
    faces.push_back("skybox/right.tga");
    faces.push_back("skybox/left.tga");
    faces.push_back("skybox/top.tga");
    faces.push_back("skybox/bottom.tga");
    faces.push_back("skybox/back.tga");
    faces.push_back("skybox/front.tga");
    mySkyBox.Load(faces);
}

void initUniforms() {
	//myBasicShader.useShaderProgram();
    myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 80.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    lightDirPo = glm::vec3(1.87212f, 2.42411f, 0.27f);
    lightDirLocPo = glGetUniformLocation(myBasicShader.shaderProgram, "lightDirPo");
    // send light dir to shader
    glUniform3fv(lightDirLocPo, 1, glm::value_ptr(lightDirPo));

    //set light color
    lightColorPo = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLocPo = glGetUniformLocation(myBasicShader.shaderProgram, "lightColorPo");
    // send light color to shader
    glUniform3fv(lightColorLocPo, 1, glm::value_ptr(lightColorPo));
 
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity"), fogDensity);
}
void renderBird()
{
    myBasicShader.useShaderProgram();
    model = glm::rotate(glm::mat4(1.0f), glm::radians(birdAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    birdAngle += 0.5f;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    bird.Draw(myBasicShader);

}


void renderBall()
{
    myBasicShader.useShaderProgram();
    //float movementSpeed = 0.5f;
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, ballS));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    ballS += 0.02f;
    if (ballS > ballSMax) {
        ballS -= 12.0f;
    }
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    ball.Draw(myBasicShader);
   
}

void renderTeapot(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    model = glm::mat4(1.0f);
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
  
    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    teapot.Draw(shader);
  
}

void renderScene() {
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //render the scene

    // render the teapot
    //renderTeapot(myBasicShader);
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

     mySkyBox.Draw(skyboxShader, view, projection);
     renderTeapot(myBasicShader);
     renderBird();
     renderBall();
     

     if (anim)
     {
         myCamera.move(gps::MOVE_BACKWARD, cSpeed);
         //update view matrix
         view = myCamera.getViewMatrix();
         myBasicShader.useShaderProgram();
         glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
         // compute normal matrix for teapot
         normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

     }
    //myBasicShader.useShaderProgram();
    //teapot.Draw(myBasicShader);
    
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    setWindowCallbacks();
    initSkybox();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
