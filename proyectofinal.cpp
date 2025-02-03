#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <cstdlib> // Para rand()
#include <ctime>   // Para seed aleatorio

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION 
#include <learnopengl/stb_image.h>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

unsigned int loadTexture(const char* path);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//gravedad
float gravity = -9.8f; // Gravedad (en unidades/s²)
float verticalVelocity = 0.0f; // Velocidad vertical de la cámara
const float groundLevel = 0.02f; // Nivel del suelo

// camera
Camera camera(glm::vec3(0.0f, 0.02f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

glm::vec3 initialCameraPosition = glm::vec3(0.0f, 0.02f, 3.0f); // Posición inicial

// Configuración del movimiento para cada enemigo
struct Enemy {
    glm::vec3 initialPosition;      //Posición inicial de aparición
    glm::vec3 position;
    glm::vec2 direction;
    float speed;
    float changeDirectionTime;
    float rotationAngle; // Nuevo: ángulo de rotación
};

glm::vec3 getRandomPositionAround(const glm::vec3& center, float minRadius, float maxRadius) {
    // Generar un ángulo aleatorio
    float angle = static_cast<float>(rand()) / RAND_MAX * 2 * glm::pi<float>();

    // Generar un radio aleatorio entre minRadius y maxRadius
    float radius = minRadius + static_cast<float>(rand()) / RAND_MAX * (maxRadius - minRadius);

    // Calcular posición X y Z
    float x = center.x + radius * cos(angle);
    float z = center.z + radius * sin(angle);

    return glm::vec3(x, center.y, z); // Mantener la Y igual al jugador
}

// Inicialización de los enemigos con sus posiciones iniciales
Enemy zombie1 = { glm::vec3(1.0f, 0.02f, 1.0f),  glm::vec3(0.0f, 0.02f, 2.0f), glm::vec2(0.0f, 1.0f), 0.25f, 0.0f };
Enemy zombie2 = { glm::vec3(1.0f, 0.02f, 2.5f), glm::vec3(0.1f, 0.02f, 3.5f), glm::vec2(0.0f, 1.0f), 0.25f, 0.0f };
Enemy zombieDog = { getRandomPositionAround(initialCameraPosition, 3.0f, 5.0f), glm::vec3(0.2f, 0.02f, 3.0f), glm::vec2(0.0f, 1.0f), 0.25f, 0.0f };
Enemy necromorph = { getRandomPositionAround(initialCameraPosition, 3.0f, 5.0f), glm::vec3(-0.2f, 0.02f, 3.0f), glm::vec2(0.0f, 1.0f), 0.25f, 0.0f };

void respawnEnemies(std::vector<Enemy*>& enemies, const glm::vec3& playerPosition) {
    for (Enemy* enemy : enemies) {
        if (!(enemy == &zombie1 || enemy == &zombie2)) {
            // Generar nueva posición aleatoria alrededor del jugador
            enemy->position = getRandomPositionAround(playerPosition, 3.0f, 5.0f);
            enemy->position.y = groundLevel; // Asegurar que estén en el suelo
        }
        
    }
}

void resetPlayer(Camera& camera, std::vector<Enemy*>& enemies) {
    camera.Position = initialCameraPosition;
    camera.Front = glm::vec3(0.0f, 0.0f, -1.0f);
    camera.Yaw = -90.0f;
    camera.Pitch = 0.0f;
    camera.updateCameraVectors();

    // Reposicionar enemigos lejos del jugador
    respawnEnemies(enemies, camera.Position);
}

std::vector<Enemy*> enemies = { &zombie1, &zombie2, &zombieDog, &necromorph };

// Semilla de números aleatorios
void initRandom() {
    srand(static_cast<unsigned>(time(0)));
}

// Método para actualizar la posición de un enemigo
void updateEnemy(Enemy& enemy, float deltaTime) {
    enemy.changeDirectionTime += deltaTime;

    // Cambio de dirección cada 1.5 segundos
    if (enemy.changeDirectionTime >= 1.5f) {
        int randomDirection = rand() % 4; // 0 = adelante, 1 = atrás, 2 = izquierda, 3 = derecha
        switch (randomDirection) {
        case 0: enemy.direction = glm::vec2(0.0f, 1.0f); break; // Adelante
        case 1: enemy.direction = glm::vec2(0.0f, -1.0f); break; // Atrás
        case 2: enemy.direction = glm::vec2(-1.0f, 0.0f); break; // Izquierda
        case 3: enemy.direction = glm::vec2(1.0f, 0.0f); break; // Derecha
        }
        enemy.changeDirectionTime = 0.0f;
    }

    // Movimiento al enemigo en la dirección actual
    enemy.position.x += enemy.direction.x * enemy.speed * deltaTime;
    enemy.position.z += enemy.direction.y * enemy.speed * deltaTime;
    enemy.position.y = groundLevel; // Mantener en el suelo

    // Limitación de la posición dentro del área de juego
    if (enemy.position.x > 2.0f) enemy.position.x = 2.0f;
    if (enemy.position.x < -2.0f) enemy.position.x = -2.0f;
    if (enemy.position.z > 4.0f) enemy.position.z = 4.0f;
    if (enemy.position.z < 2.0f) enemy.position.z = 2.0f;
}

// Método para actualizar la posición de los enemigos especiales.
void updateSpecialEnemy(Enemy& enemy, float deltaTime, const glm::vec3& playerPosition) {
    glm::vec3 directionToPlayer = playerPosition - enemy.position;

    // Normalización de la dirección para que sea un vector unitario
    glm::vec2 normalizedDirection = glm::normalize(glm::vec2(directionToPlayer.x, directionToPlayer.z));

    if (&enemy == &zombieDog) {
        // El Zombie Dog sigue directamente al jugador
        enemy.direction = normalizedDirection;
    }
    else if (&enemy == &necromorph) {
        // Aumenta la velocidad del Necromorph para que sea más rápido
        enemy.speed = 0.5f;

        // El Necromorph persigue al jugador, pero con una ligera desviación para diferenciarse del Zombie Dog
        glm::vec2 perpendicular(-normalizedDirection.y, normalizedDirection.x);  // Vector perpendicular
        enemy.direction = normalizedDirection + (perpendicular * 0.3f); // Desviación leve
        enemy.direction = glm::normalize(enemy.direction); // Normalizar para mantener la magnitud
    }

    // Mover al enemigo en la dirección calculada
    enemy.position.x += enemy.direction.x * enemy.speed * deltaTime;
    enemy.position.z += enemy.direction.y * enemy.speed * deltaTime;
    enemy.position.y = groundLevel; // Mantener en el suelo

    // Permite que los enemigos se acerquen más al jugador y tengan un campo de movimiento más amplio
    if (enemy.position.x > 5.0f) enemy.position.x = 5.0f;  // Aumentar rango de movimiento en X
    if (enemy.position.x < -5.0f) enemy.position.x = -5.0f; // Aumentar rango de movimiento en X
    if (enemy.position.z > 5.0f) enemy.position.z = 5.0f;  // Aumentar rango de movimiento en Z
    if (enemy.position.z < 0.5f) enemy.position.z = 0.5f;  // Permitir que se acerque más al jugador (en Z)

    // Calculo del ángulo de rotación
    enemy.rotationAngle = glm::degrees(atan2(normalizedDirection.x, normalizedDirection.y));
}

//Iluminación linterna
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

//Tiempo de inclinación
float cameraTiltTime = 0.0f;

// luna movimiento
float angle = 0.0f;

//Velocidad camara
float cameraSpeed = 1.0f; // Velocidad de movimiento de la cámara

//Apagar/prender linterna
bool flashlightOn = true;  // La linterna comienza encendida

bool checkEnemyCollision(const glm::vec3& playerPos, const std::vector<Enemy*>& enemies, float collisionThreshold = 0.5f) {
    for (const Enemy* enemy : enemies) {
        float distance = glm::distance(playerPos, enemy->position);
        if (distance < collisionThreshold) {
            return true; // Colisión detectada
        }
    }
    return false;
}

void resetPlayer(Camera& camera) {
    camera.Position = initialCameraPosition;
    camera.Front = glm::vec3(0.0f, 0.0f, -1.0f); // Orientación inicial
    camera.Yaw = -90.0f; // Asegura que la dirección sea consistente
    camera.Pitch = 0.0f;
    camera.updateCameraVectors(); 
}

glm::mat4 projection;

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
    //GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Proyecto TLoU", NULL, NULL);
        // Obtener el monitor principal y su resolución
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    // Crear la ventana en modo pantalla completa con la resolución del monitor
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "The Legacy of the Curse", primaryMonitor, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float aspectRatio = (float)width / (float)height;

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

    Model escenarioModel("modelos/escenario/escenario.obj");
    Model necromorphModel("modelos/necromorph/necromorph.obj");
    Model zombieDogModel("modelos/zombiedog/zombiedog.obj");
    Model zombie1Model("modelos/zombie1/zombie1.obj");
    Model zombie2Model("modelos/zombie2/zombie2.obj");
    Model skyModel("modelos/cielo/cielo.obj");
    Model moonModel("modelos/moon/moon.obj");

    //TEXTURA SUELO
    float floorVertices[] = {
        // Posiciones        // Normales       // Coordenadas de textura
        -50.0f, 0.0f, -50.0f,  0.0f, 1.0f, 0.0f,  0.0f,  50.0f,
         50.0f, 0.0f, -50.0f,  0.0f, 1.0f, 0.0f,  50.0f, 50.0f,
         50.0f, 0.0f,  50.0f,  0.0f, 1.0f, 0.0f,  50.0f,  0.0f,
    
        -50.0f, 0.0f, -50.0f,  0.0f, 1.0f, 0.0f,  0.0f,  50.0f,
         50.0f, 0.0f,  50.0f,  0.0f, 1.0f, 0.0f,  50.0f,  0.0f,
        -50.0f, 0.0f,  50.0f,  0.0f, 1.0f, 0.0f,  0.0f,   0.0f
    };

    unsigned int floorVAO, floorVBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

    glBindVertexArray(floorVAO);
    // Posiciones
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normales
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Coordenadas UV
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    unsigned int floorTexture = loadTexture("textures/suelo.jpeg");
    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Inicializar la semilla de números aleatorios
    srand(time(0));

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
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.MovementSpeed = cameraSpeed; //Optional. Modify the speed of the camera
        
        // don't forget to enable shader before setting uniforms
        ourShader.use();
        ourShader.setBool("useLighting", true);

        // Efecto de inclinación de cámara
        float tiltAngle = glm::radians(0.5f) * sin(cameraTiltTime);  // Oscilación suave
        cameraTiltTime += deltaTime * 0.2f;  // Control de velocidad del balanceo
        // Crear matriz de rotación para la inclinación
        glm::mat4 tilt = glm::rotate(glm::mat4(1.0f), tiltAngle, glm::vec3(0.0f, 0.0f, 1.0f));
        // Modificar la matriz de vista con inclinación
        glm::mat4 view = camera.GetViewMatrix() * tilt;

        // view/projection transformations
        projection = glm::perspective(glm::radians(camera.Zoom), aspectRatio, 0.1f, 100.0f);
        //glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        
        //world transformation
        glm::mat4 model = glm::mat4(1.0f);

         // render the floor
         ourShader.use();
         model = glm::mat4(1.0f);
         ourShader.setMat4("model", model);

         // Activar textura
         glActiveTexture(GL_TEXTURE0);
         glBindTexture(GL_TEXTURE_2D, floorTexture);
         ourShader.setInt("texture_emissive1", 0); // Enlazar con el shader

         // Dibujar el suelo
         glBindVertexArray(floorVAO);
         glDrawArrays(GL_TRIANGLES, 0, 6);
         glBindVertexArray(0);

        //calculo gravedad
        // Gravedad y colisión con el suelo
        if (camera.Position.y > groundLevel) {
            verticalVelocity += gravity * deltaTime; // Aumentar velocidad por la gravedad
            camera.Position.y += verticalVelocity * deltaTime; // Actualizar posición
        }
        else {
            camera.Position.y = groundLevel; // Mantener en el suelo
            verticalVelocity = 0.0f; // Restablecer la velocidad al estar en el suelo
        }

        //Uso de ourShader
        ourShader.use();
        ourShader.setBool("useLighting", true); // Activar iluminación
        ourShader.setBool("flashlightOn", flashlightOn); // Estado de la linterna
        ourShader.setVec3("flashlightPos", camera.Position); // Posición de la linterna
        ourShader.setVec3("flashlightDir", camera.Front); // Dirección de la linterna
        ourShader.setFloat("cutoff", glm::cos(glm::radians(12.5f))); // Ángulo interno
        ourShader.setFloat("outerCutoff", glm::cos(glm::radians(17.5f))); // Ángulo externo
        ourShader.setFloat("constant", 1.0f); // Atenuación constante
        ourShader.setFloat("linear", 0.09f); // Atenuación lineal
        ourShader.setFloat("quadratic", 0.032f); // Atenuación cuadrática
        
        // render the loaded model
        // Renderizar el escenario
        glm::mat4 cityModel = glm::mat4(1.0f);
        cityModel = glm::translate(cityModel, glm::vec3(0.0f, 0.0f, 0.0f)); // Posición del escenario
        cityModel = glm::scale(cityModel, glm::vec3(0.1f, 0.1f, 0.1f));	// Escalar el modelo
        ourShader.setMat4("model", cityModel);
        escenarioModel.Draw(ourShader);
        
        //Renderizar el Cierlo (sky)
        ourShader.use();
        // Para el cielo, desactivar la iluminación
        //ourShader.setBool("useLighting", false);
        glm::mat4 modelSky = glm::mat4(1.0f);
        modelSky = glm::translate(modelSky, glm::vec3(0.0f, 15.0f, 0.0f));
        modelSky = glm::scale(modelSky, glm::vec3(0.3f, 0.3f, 0.3f));
        ourShader.setMat4("model", modelSky);
        skyModel.Draw(ourShader);
        // Reactivar iluminación para los siguientes objetos
        //ourShader.setBool("useLighting", true);

        // Actualizar la posición de cada enemigo
        updateEnemy(zombie1, deltaTime);
        updateEnemy(zombie2, deltaTime);
        updateSpecialEnemy(zombieDog, deltaTime, camera.Position);
        updateSpecialEnemy(necromorph, deltaTime, camera.Position);

        // Verificar colisión con enemigos
        if (checkEnemyCollision(camera.Position, enemies)) {
        resetPlayer(camera, enemies); // Reiniciar al jugador
        std::cout << "¡Has sido atrapado! Reiniciando posición..." << std::endl;
        }
        
        // Dibujo de cada enemigo con su nueva posición
        glm::mat4 transform;

        transform = glm::translate(glm::mat4(1.0f), zombie1.position);
        transform = glm::rotate(transform, glm::radians(zombie1.rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotación
        transform = glm::scale(transform, glm::vec3(0.02f, 0.02f, 0.02f));
        ourShader.setMat4("model", transform);
        zombie1Model.Draw(ourShader);

        transform = glm::translate(glm::mat4(1.0f), zombie2.position);
        transform = glm::rotate(transform, glm::radians(zombie2.rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotación
        transform = glm::scale(transform, glm::vec3(0.04f, 0.04f, 0.04f));
        ourShader.setMat4("model", transform);
        zombie2Model.Draw(ourShader);

        transform = glm::translate(glm::mat4(1.0f), zombieDog.position);
        transform = glm::rotate(transform, glm::radians(zombieDog.rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotación
        transform = glm::scale(transform, glm::vec3(0.0008f, 0.0008f, 0.0008f));
        ourShader.setMat4("model", transform);
        zombieDogModel.Draw(ourShader);

        transform = glm::translate(glm::mat4(1.0f), necromorph.position);
        transform = glm::rotate(transform, glm::radians(necromorph.rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotación
        transform = glm::scale(transform, glm::vec3(0.1f, 0.1f, 0.1f));
        ourShader.setMat4("model", transform);
        necromorphModel.Draw(ourShader);

        // Renderizar la Luna
        ourShader.use();
        ourShader.setBool("useLighting", false);
        glm::mat4 modelMoon = glm::mat4(1.0f);
        modelMoon = glm::translate(modelMoon, glm::vec3(0.0f, 20.0f, 0.0f));
        modelMoon = glm::scale(modelMoon, glm::vec3(1.5f, 1.5f, 1.5f));
        // Movimiento de la Luna
        angle = glfwGetTime() * 15.0f;
        modelMoon = glm::rotate(modelMoon, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMoon = glm::translate(modelMoon, glm::vec3(0.0f, 0.0f, -5.0f));
        // Extraer la posición de la luna (última columna de la matriz modelMoon)
        glm::vec3 moonPos = glm::vec3(modelMoon[3][0], modelMoon[3][1], modelMoon[3][2]);

        // Configurar la luz
        ourShader.setVec3("light.position", moonPos);
        ourShader.setVec3("light.ambient", glm::vec3(0.05f, 0.05f, 0.05f));  // Luz ambiente tenue
        ourShader.setVec3("light.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));  // Luz difusa moderada
        ourShader.setVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));  // Luz especular moderada
        // Pasar la posición de la cámara (para el cálculo especular)
        ourShader.setVec3("viewPos", camera.Position);

        ourShader.setMat4("model", modelMoon);
        moonModel.Draw(ourShader);
        // Reactivar iluminación para los siguientes objetos
        ourShader.setBool("useLighting", true);

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
    const float groundLevel = 0.18f; // Altura mínima permitida
    if (camera.Position.y < groundLevel) {
        camera.Position.y = groundLevel; // Evita que atraviese el piso
    }

    //salto
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && camera.Position.y <= groundLevel + 0.01f) {
        verticalVelocity = 3.0f; // Velocidad inicial del salto
    }

    //balanceo de la camara
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraTiltTime += deltaTime * 0.5f;
    }
    else {
        cameraTiltTime = 0.0f;  // Reinicia la inclinación si el jugador está quieto
    }

    /Apagar/Prender linterna
    static bool flashlightKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !flashlightKeyPressed) {
        flashlightOn = !flashlightOn; // Alternar el estado de la linterna
        flashlightKeyPressed = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
        flashlightKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        cameraSpeed = 2.0f;
    }
    else {
        cameraSpeed = 1.0f;
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

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
