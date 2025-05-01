#include <string>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SOIL2/SOIL2.h"
#include "stb_image.h"

const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

Camera camera(glm::vec3(0.0f, 15.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400.0f, lastY = 300.0f;
bool firstMouse = true;
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Variables para la animación
float globalAnimationTime = -1.0f;
bool animationPlaying = false;
float animationSpeed = 2.0f;
int currentComponent = 0;

// Estructura para configurar instancias de modelos
struct ModelInstance {
    glm::vec3 position;
    float rotationY;
    glm::vec3 scale;
};

// Estructura para configurar puestos de trabajo (mesa + CPU + silla)
struct Workstation {
    ModelInstance desk;
    ModelInstance cpu;
    ModelInstance chair;
};

// Estructura para componentes de la computadora
struct ComputerComponent {
    Model* model;
    glm::vec3 position;
    float rotationY;
    glm::vec3 scale;
    float animationStartTime;
    float rebuildDuration;
    bool isAnimating;
    bool hasAnimated;
};

std::vector<ComputerComponent> components;

// Función para renderizar una instancia de modelo
void RenderInstance(Shader& shader, Model& model, const ModelInstance& instance) {
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(instance.rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::translate(modelMatrix, instance.position);
    modelMatrix = glm::scale(modelMatrix, instance.scale);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    model.Draw(shader);
}

// Función para renderizar un componente con animación de 3 giros
void RenderComponent(Shader& shader, ComputerComponent& component, float currentTime) {
    if (!component.isAnimating) {
        if (component.hasAnimated) {
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, component.position);
            modelMatrix = glm::rotate(modelMatrix, glm::radians(component.rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
            modelMatrix = glm::scale(modelMatrix, component.scale);
            glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
            component.model->Draw(shader);
        }
        return;
    }

    float animProgress = glm::clamp((currentTime - component.animationStartTime) / component.rebuildDuration, 0.0f, 1.0f);

    // Solo 3 fragmentos (uno por giro)
    const int fragmentCount = 3;
    float fragmentScale = 0.5f; // Mayor escala inicial para mejor visibilidad

    for (int i = 0; i < fragmentCount; i++) {
        float fragmentProgress = glm::clamp(animProgress * fragmentCount - i, 0.0f, 1.0f);

        if (fragmentProgress <= 0.0f) continue;

        glm::vec3 targetPos = component.position;
        // Posiciones fijas para los 3 giros (arriba, izquierda, derecha)
        glm::vec3 startPos;
        switch (i % 3) {
        case 0: startPos = targetPos + glm::vec3(0.0f, 2.0f, 0.0f); break; // Arriba
        case 1: startPos = targetPos + glm::vec3(-2.0f, 0.0f, 0.0f); break; // Izquierda
        case 2: startPos = targetPos + glm::vec3(2.0f, 0.0f, 0.0f); break; // Derecha
        }

        glm::vec3 renderPos = startPos + (targetPos - startPos) * fragmentProgress;
        glm::vec3 renderScale = component.scale * fragmentScale +
            (component.scale - component.scale * fragmentScale) * fragmentProgress;

        // Cada fragmento gira 3 veces (1080 grados) durante la animación
        float rotations = 3.0f;
        float renderRot = rotations * 360.0f * fragmentProgress;

        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, renderPos);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(renderRot), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, renderScale);

        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

        glm::vec3 animColor(0.3f + 0.7f * fragmentProgress,
            0.3f + 0.7f * fragmentProgress,
            0.3f + 0.7f * fragmentProgress);
        glUniform3f(glGetUniformLocation(shader.Program, "material.diffuse"),
            animColor.r, animColor.g, animColor.b);

        component.model->Draw(shader);
    }

    glUniform3f(glGetUniformLocation(shader.Program, "material.diffuse"), 1.0f, 1.0f, 1.0f);

    if (animProgress >= 1.0f) {
        component.isAnimating = false;
        component.hasAnimated = true;
    }
}

void UpdateAnimations(float currentTime) {
    if (!animationPlaying) return;

    if (currentTime < 0) {
        globalAnimationTime = 0.0f;
        if (!components.empty()) {
            components[0].isAnimating = true;
            components[0].animationStartTime = 0.0f;
        }
        return;
    }

    globalAnimationTime += deltaTime * animationSpeed;

    bool allAnimated = true;
    for (size_t i = 0; i < components.size(); ++i) {
        if (!components[i].hasAnimated) {
            allAnimated = false;

            if (!components[i].isAnimating && (i == 0 || components[i - 1].hasAnimated)) {
                components[i].isAnimating = true;
                components[i].animationStartTime = globalAnimationTime;
            }
            break;
        }
    }

    if (allAnimated) {
        animationPlaying = false;
    }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecto", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    Shader shader("Shader/lighting.vs", "Shader/lighting.frag");
    Shader shadowShader("Shader/shadow.vs", "Shader/shadow.frag");

    // Cargar modelos principales
    Model piso((char*)"Models/Proyecto/piso/piso.obj");
    Model pared((char*)"Models/Proyecto/Pared/pared.obj");
    Model techoo((char*)"Models/Proyecto/piso1/piso.obj");
    Model lampara((char*)"Models/Proyecto/lampled/lampled.obj");
    Model pizarron((char*)"Models/Proyecto/pizarron/pizarron.obj");
    Model cpu((char*)"Models/Proyecto/computadora/computadora.obj");
    Model silla((char*)"Models/Proyecto/silla/silla.obj");
    Model mesa((char*)"Models/Proyecto/mesa/mesa.obj");
    Model ventanas((char*)"Models/Proyecto/ventana/ventana.obj");

    // Cargar modelos de los componentes de computadora
    Model gabinete((char*)"Models/Proyecto/gabinete/gabinete.obj");
    Model placamadre((char*)"Models/Proyecto/placamadre/placamadre.obj");
    Model procesador((char*)"Models/Proyecto/procesador/procesador.obj");
    Model ram((char*)"Models/Proyecto/ram/ram.obj");
    Model ssd((char*)"Models/Proyecto/ssd/ssd.obj");
    Model tarjetagrafica((char*)"Models/Proyecto/tarjetagrafica/tarjetagrafica.obj");
    Model ventilador((char*)"Models/Proyecto/ventilador/ventilador.obj");
    Model ventilador2((char*)"Models/Proyecto/ventilador2/ventilador.obj");
    Model fuente((char*)"Models/Proyecto/fuente/fuente.obj");

    // Configurar componentes de computadora para animación
    components = {
        {&gabinete, glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, glm::vec3(10.0f), 0.0f, 1.2f, false, false},
        {&placamadre, glm::vec3(0.0f, 0.5f, 0.0f), 0.0f, glm::vec3(10.0f), 0.0f, 1.1f, false, false},
        {&procesador, glm::vec3(0.0f, 0.5f, 0.1f), 0.0f, glm::vec3(10.0f), 0.0f, 1.0f, false, false},
        {&ram, glm::vec3(-0.1f, 0.5f, 0.1f), 0.0f, glm::vec3(10.0f), 0.0f, 0.9f, false, false},
        {&ssd, glm::vec3(0.1f, 0.5f, -0.1f), 0.0f, glm::vec3(10.0f), 0.0f, 1.0f, false, false},
        {&tarjetagrafica, glm::vec3(0.1f, 0.5f, 0.0f), 0.0f, glm::vec3(10.0f), 0.0f, 1.2f, false, false},
        {&ventilador, glm::vec3(0.0f, 0.5f, -0.1f), 0.0f, glm::vec3(10.0f), 0.0f, 1.0f, false, false},
        {&ventilador2, glm::vec3(0.0f, 0.5f, 0.2f), 0.0f, glm::vec3(10.0f), 0.0f, 1.0f, false, false},
        {&fuente, glm::vec3(0.0f, 0.5f, -0.2f), 0.0f, glm::vec3(10.0f), 0.0f, 1.2f, false, false}
    };

    // Configurar puestos de trabajo (mesa, cpu, silla)
    std::vector<Workstation> workstations = {
        // Fila 1 (izquierda)
        {
            {glm::vec3(-25.0f, 5.7f, -23.0f), 90.0f, glm::vec3(10.0f)}, // mesa
            {glm::vec3(-25.0f, 10.5f, -23.0f), 90.0f, glm::vec3(8.0f)},  // cpu
            {glm::vec3(-32.0f, 6.8f, -23.0f), 90.0f, glm::vec3(7.8f)}   // silla
        },
        // ... (resto de las configuraciones de puestos de trabajo)
    };

    // Configuración de paredes
    std::vector<ModelInstance> walls = {
        // ... (configuraciones de paredes existentes)
    };

    // Configuración de ventanas
    std::vector<ModelInstance> windows = {
        {glm::vec3(37.4f, 19.5f, -42.0f), 0.0f, glm::vec3(40.7f, 20.7f, 50.7f)},
        {glm::vec3(37.4f, 19.5f, 11.0f), 0.0f, glm::vec3(40.7f, 20.7f, 50.7f)}
    };

    // Configuración de mesas adicionales
    ModelInstance teacherDesk = { glm::vec3(70.0f, 5.7f, -20.0f), 90.0f, glm::vec3(10.0f, 10.0f, 18.0f) };
    ModelInstance additionalDesk = { glm::vec3(-44.0f, 5.7f, 20.0f), 90.0f, glm::vec3(10.0f, 10.0f, 18.0f) };

    // Precalcular matrices para objetos estáticos
    const glm::mat4 lampTransform = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.4f, 30.0f, -15.5f)), glm::vec3(40.7f, 42.7f, 00.7f));
    const glm::mat4 ceilingTransform = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.4f, 30.0f, -15.5f)), glm::vec3(60.7f, 5.7f, 150.7f));
    const glm::mat4 floorTransform = glm::scale(glm::translate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)), glm::vec3(2.2f, -5.0f, -10.5f)), glm::vec3(60.7f, 110.7f, 145.7f));
    const glm::mat4 boardTransform = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 10.0f, -80.0f)), glm::vec3(30.0f, 14.0f, 25.0f));
    const glm::mat4 frontWallTransform = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 17.0f, -82.3f)), glm::vec3(63.0f, 30.0f, 3.0f));

    while (!glfwWindowShouldClose(window)) {
        GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
        deltaTime = static_cast<GLfloat>(currentFrame - lastFrame);
        lastFrame = currentFrame;

        UpdateAnimations(globalAnimationTime);

        glfwPollEvents();
        DoMovement();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();

        // Configurar luces
        // Luz direccional
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.ambient"), 0.5f, 0.5f, 0.5f);
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.diffuse"), 0.8f, 0.8f, 0.8f);
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.specular"), 0.5f, 0.5f, 0.5f);

        // Luz puntual (lámpara)
        glm::vec3 lightPos(0.4f, 20.0f, -10.5f);
        glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].position"), lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].ambient"), 0.2f, 0.2f, 0.2f);
        glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].diffuse"), 0.8f, 0.8f, 0.8f);
        glUniform3f(glGetUniformLocation(shader.Program, "pointLights[0].specular"), 1.0f, 1.0f, 1.0f);
        glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].linear"), 0.09f);
        glUniform1f(glGetUniformLocation(shader.Program, "pointLights[0].quadratic"), 0.032f);

        // Spotlight (foco de cámara)
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.ambient"), 0.1f, 0.1f, 0.1f);
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.diffuse"), 0.8f, 0.8f, 0.8f);
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.specular"), 1.0f, 1.0f, 1.0f);
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.linear"), 0.09f);
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.quadratic"), 0.032f);
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.5f)));
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(15.0f)));

        // Propiedades del material
        glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"), 32.0f);

        // Configuración de vista y proyección
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(camera.GetZoom(), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3f(glGetUniformLocation(shader.Program, "viewPos"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

        // Renderizar ventanas
        for (const auto& windowInstance : windows) {
            RenderInstance(shader, ventanas, windowInstance);
        }

        // Renderizar lámpara
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(lampTransform));
        lampara.Draw(shader);

        // Renderizar techo
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(ceilingTransform));
        techoo.Draw(shader);

        // Renderizar piso
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(floorTransform));
        piso.Draw(shader);

        // Renderizar paredes
        for (const auto& wall : walls) {
            RenderInstance(shader, pared, wall);
        }

        // Renderizar pared frontal
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(frontWallTransform));
        pared.Draw(shader);

        // Renderizar pizarrón
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(boardTransform));
        pizarron.Draw(shader);

        // Cambiar material para mesas (madera - medio brillo)
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.ambient"), 0.3f, 0.3f, 0.3f);
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.diffuse"), 0.9f, 0.9f, 0.9f);
        glUniform3f(glGetUniformLocation(shader.Program, "material.specular"), 0.2f, 0.2f, 0.2f);
        glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"), 30.0f);

        // Renderizar puestos de trabajo
        for (const auto& workstation : workstations) {
            RenderInstance(shader, mesa, workstation.desk);
            RenderInstance(shader, cpu, workstation.cpu);
            RenderInstance(shader, silla, workstation.chair);
        }

        // Renderizar mesas adicionales
        RenderInstance(shader, mesa, teacherDesk);
        RenderInstance(shader, mesa, additionalDesk);

        // Renderizar componentes de computadora con animación
        for (auto& component : components) {
            RenderComponent(shader, component, globalAnimationTime);
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

void DoMovement() {
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])    camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])  camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])  camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT, deltaTime);

    // Control de la animación
    if (keys[GLFW_KEY_SPACE] && !animationPlaying) {
        animationPlaying = true;
        if (globalAnimationTime < 0) globalAnimationTime = 0.0f;
        keys[GLFW_KEY_SPACE] = false;
    }
    else if (keys[GLFW_KEY_SPACE] && animationPlaying) {
        animationPlaying = false;
        keys[GLFW_KEY_SPACE] = false;
    }

    if (keys[GLFW_KEY_R]) {
        globalAnimationTime = -1.0f;
        animationPlaying = true;
        for (auto& component : components) {
            component.isAnimating = false;
            component.hasAnimated = false;
        }
        keys[GLFW_KEY_R] = false;
    }

    if (keys[GLFW_KEY_LEFT_BRACKET]) {
        animationSpeed = glm::max(0.5f, animationSpeed - 0.1f);
        keys[GLFW_KEY_LEFT_BRACKET] = false;
    }
    if (keys[GLFW_KEY_RIGHT_BRACKET]) {
        animationSpeed = glm::min(5.0f, animationSpeed + 0.1f);
        keys[GLFW_KEY_RIGHT_BRACKET] = false;
    }
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)      keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos) {
    if (firstMouse) {
        lastX = static_cast<GLfloat>(xPos);
        lastY = static_cast<GLfloat>(yPos);
        firstMouse = false;
    }
    GLfloat xOffset = static_cast<GLfloat>(xPos - lastX);
    GLfloat yOffset = static_cast<GLfloat>(lastY - yPos);
    lastX = static_cast<GLfloat>(xPos);
    lastY = static_cast<GLfloat>(yPos);
    camera.ProcessMouseMovement(xOffset, yOffset);
}