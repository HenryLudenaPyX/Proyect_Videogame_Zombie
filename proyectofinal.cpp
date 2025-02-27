#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION 
#include <learnopengl/stb_image.h>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//gravedad
float gravity = -9.8f; // Gravedad (en unidades/s�)
float verticalVelocity = 0.0f; // Velocidad vertical de la c�mara
const float groundLevel = 0.0f; // Nivel del suelo



// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;



int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Proyecto TLoU", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    //stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader("shaders/shader_pf.vs", "shaders/shader_pf.fs");

    // load models
    // -----------

    Model escenarioModel("model/escenario/escenario.obj");
    Model necromorphModel("model/necromorph/necromorph.obj");
    Model zombieDogModel("model/zombiedog/zombiedog.obj");
    Model zombie1Model("model/zombie1/zombie1.obj");
    Model zombie2Model("model/zombie2/zombie2.obj");
    Model zombie3Model("model/zombie3/zombie3.obj");


    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    camera.MovementSpeed = 1; //Optional. Modify the speed of the camera

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.5f, 0.5f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        ourShader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        //calculo gravedad
        // Gravedad y colisi�n con el suelo
        if (camera.Position.y > groundLevel) {
            verticalVelocity += gravity * deltaTime; // Aumentar velocidad por la gravedad
            camera.Position.y += verticalVelocity * deltaTime; // Actualizar posici�n
        }
        else {
            camera.Position.y = groundLevel; // Mantener en el suelo
            verticalVelocity = 0.0f; // Restablecer la velocidad al estar en el suelo
        }

        // render the loaded model
        // Renderizar el escenario
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Posici�n del escenario
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// Escalar el modelo
        ourShader.setMat4("model", model);
        escenarioModel.Draw(ourShader);

        // Renderizar el necromorph
        glm::mat4 necromorphTransform = glm::mat4(1.0f);
        necromorphTransform = glm::translate(necromorphTransform, glm::vec3(2.0f, 0.0f, -3.0f)); // Mover el personaje
        necromorphTransform = glm::scale(necromorphTransform, glm::vec3(0.05f, 0.05f, 0.05f));
        ourShader.setMat4("model", necromorphTransform);
        necromorphModel.Draw(ourShader);

        // Renderizar los zombis
        glm::mat4 zombieDogTransform = glm::mat4(1.0f);
        zombieDogTransform = glm::translate(zombieDogTransform, glm::vec3(-2.0f, 0.0f, -1.0f)); // Posici�n del zombie dog
        zombieDogTransform = glm::scale(zombieDogTransform, glm::vec3(0.03f, 0.03f, 0.03f));
        ourShader.setMat4("model", zombieDogTransform);
        zombieDogModel.Draw(ourShader);

        glm::mat4 zombie1Transform = glm::mat4(1.0f);
        zombie1Transform = glm::translate(zombie1Transform, glm::vec3(4.0f, 0.0f, -5.0f)); // Posici�n del zombie 1
        zombie1Transform = glm::scale(zombie1Transform, glm::vec3(0.04f, 0.04f, 0.04f));
        ourShader.setMat4("model", zombie1Transform);
        zombie1Model.Draw(ourShader);

        glm::mat4 zombie2Transform = glm::mat4(1.0f);
        zombie2Transform = glm::translate(zombie2Transform, glm::vec3(-4.0f, 0.0f, -6.0f)); // Posici�n del zombie 2
        zombie2Transform = glm::scale(zombie2Transform, glm::vec3(0.04f, 0.04f, 0.04f));
        ourShader.setMat4("model", zombie2Transform);
        zombie2Model.Draw(ourShader);

        glm::mat4 zombie3Transform = glm::mat4(1.0f);
        zombie3Transform = glm::translate(zombie3Transform, glm::vec3(0.0f, 0.0f, -7.0f)); // Posici�n del zombie 3
        zombie3Transform = glm::scale(zombie3Transform, glm::vec3(0.04f, 0.04f, 0.04f));
        ourShader.setMat4("model", zombie3Transform);
        zombie3Model.Draw(ourShader);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{



    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    //Tocar piso
    const float groundLevel = 0.1f; // Altura m�nima permitida
    if (camera.Position.y < groundLevel) {
        camera.Position.y = groundLevel; // Evita que atraviese el piso
    }

    //salto
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && camera.Position.y == groundLevel) {
        verticalVelocity = 2.0f; // Velocidad inicial del salto
    }


}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}