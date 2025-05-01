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

// Función para renderizar una instancia de modelo
void RenderInstance(Shader& shader, Model& model, const ModelInstance& instance) {
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(instance.rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::translate(modelMatrix, instance.position);
    modelMatrix = glm::scale(modelMatrix, instance.scale);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    model.Draw(shader);
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

    Model piso((char*)"Models/Proyecto/piso/piso.obj"); // Cargar modelo del piso
<<<<<<< Updated upstream
    Model pared((char*)"Models/Proyecto/Pared/tripo_pbr_model_01b89d8b-a342-4896-89aa-fe20ab730130.obj"); // Cargar modelo de pared
=======
    Model pared((char*)"Models/Proyecto/Pared/pared.obj"); // Cargar modelo de pared
>>>>>>> Stashed changes
    Model techoo((char*)"Models/Proyecto/piso1/piso.obj");   // Cargar modelo del techo
    Model lampara((char*)"Models/Proyecto/lampled/lampled.obj"); // Cargar modelo de lámpara
    Model pizarron((char*)"Models/Proyecto/pizarron/pizarron.obj"); // Cargar modelo del pizarrón
    Model cpu((char*)"Models/Proyecto/computadora/computadora.obj"); // Cargar modelo de computadora
    Model silla((char*)"Models/Proyecto/silla/silla.obj"); // Cargar modelo de silla
    Model mesa((char*)"Models/Proyecto/mesa/mesa.obj");   // Cargar modelo de mesa
    Model ventanas((char*)"Models/Proyecto/ventana/ventana.obj");   // Cargar modelo de ventana

    // Configurar puestos de trabajo (mesa, cpu, silla)
    std::vector<Workstation> workstations = {
        // Fila 1 (izquierda)
        {
            {glm::vec3(-25.0f, 5.7f, -23.0f), 90.0f, glm::vec3(10.0f)}, // mesa
<<<<<<< Updated upstream
            {glm::vec3(26.0f, 10.6f, 20.0f), -95.0f, glm::vec3(8.0f)},  // cpu
            {glm::vec3(-29.0f, 6.8f, -23.0f), 90.0f, glm::vec3(7.8f)}   // silla
        },
        {
            {glm::vec3(-25.0f, 5.7f, -13.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(25.5f, 10.6f, 11.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(-29.0f, 6.8f, -13.0f), 90.0f, glm::vec3(7.8f)}
=======
            {glm::vec3(-25.0f, 10.5f, -23.0f), 90.0f, glm::vec3(8.0f)},  // cpu
            {glm::vec3(-32.0f, 6.8f, -23.0f), 90.0f, glm::vec3(7.8f)}   // silla
        },
        {
            {glm::vec3(-25.0f, 5.7f, -13.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(-25.0f, 10.5f, -13.0f), 90.0f, glm::vec3(8.0f)},
            {glm::vec3(-32.0f, 6.8f, -13.0f), 90.0f, glm::vec3(7.8f)}
>>>>>>> Stashed changes
        },
        // Fila 2
        {
            {glm::vec3(-25.0f, 5.7f, 13.0f), 90.0f, glm::vec3(10.0f)},
<<<<<<< Updated upstream
            {glm::vec3(23.8f, 10.6f, -16.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(-29.0f, 6.8f, 13.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(-25.0f, 5.7f, 23.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(22.8f, 10.6f, -25.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(-29.0f, 6.8f, 23.0f), 90.0f, glm::vec3(7.8f)}
=======
            {glm::vec3(-25.0f, 10.5f, 13.0f), 90.0f, glm::vec3(8.0f)},
            {glm::vec3(-32.0f, 6.8f, 13.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(-25.0f, 5.7f, 23.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(-25.0f, 10.5f, 23.0f), 90.0f, glm::vec3(8.0f)},
            {glm::vec3(-32.0f, 6.8f, 23.0f), 90.0f, glm::vec3(7.8f)}
>>>>>>> Stashed changes
        },
        // Fila 3 (centro)
        {
            {glm::vec3(-6.0f, 5.7f, 23.0f), 90.0f, glm::vec3(10.0f)},
<<<<<<< Updated upstream
            {glm::vec3(3.8f, 10.6f, -23.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(-10.0f, 6.8f, 23.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(-6.0f, 5.7f, 13.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(3.8f, 10.6f, -13.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(-10.0f, 6.8f, 13.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(-6.0f, 5.7f, -13.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(6.8f, 10.6f, 13.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(-10.0f, 6.8f, -13.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(-6.0f, 5.7f, -23.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(7.5f, 10.6f, 23.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(-10.0f, 6.8f, -23.0f), 90.0f, glm::vec3(7.8f)}
=======
            {glm::vec3(-6.0f, 10.5f, 23.0f), 90.0f, glm::vec3(8.0f)},
            {glm::vec3(-13.0f, 6.8f, 23.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(-6.0f, 5.7f, 13.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(-6.0f, 10.5f, 13.0f), 90.0f, glm::vec3(8.0f)},
            {glm::vec3(-13.0f, 6.8f, 13.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(-6.0f, 5.7f, -13.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(-6.0f, 10.5f, -13.0f), 90.0f, glm::vec3(8.0f)},
            {glm::vec3(-13.0f, 6.8f, -13.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(-6.0f, 5.7f, -23.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(-6.0f, 10.5f, -23.0f), 90.0f, glm::vec3(8.0f)},
            {glm::vec3(-13.0f, 6.8f, -23.0f), 90.0f, glm::vec3(7.8f)}
>>>>>>> Stashed changes
        },
        // Fila 4 (derecha)
        {
            {glm::vec3(13.0f, 5.7f, -13.0f), 90.0f, glm::vec3(10.0f)},
<<<<<<< Updated upstream
            {glm::vec3(-11.5f, 10.6f, 25.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(9.0f, 6.8f, -13.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(13.0f, 5.7f, -23.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(-11.5f, 10.6f, 15.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(9.0f, 6.8f, -23.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(13.0f, 5.7f, 13.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(-15.0f, 10.6f, -11.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(9.0f, 6.8f, 13.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(13.0f, 5.7f, 23.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(-15.0f, 10.6f, -21.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(9.0f, 6.8f, 23.0f), 90.0f, glm::vec3(7.8f)}
=======
            {glm::vec3(13.0f, 10.5f, -13.0f), 90.0f, glm::vec3(8.0f)},
            {glm::vec3(6.0f, 6.8f, -13.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(13.0f, 5.7f, -23.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(13.0f, 10.5f, -23.0f), 90.0f, glm::vec3(8.0f)},
            {glm::vec3(6.0f, 6.8f, -23.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(13.0f, 5.7f, 13.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(13.0f, 10.5f, 13.0f), 90.0f, glm::vec3(8.0f)},
            {glm::vec3(6.0f, 6.8f, 13.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(13.0f, 5.7f, 23.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(13.0f, 10.5f, 23.0f), 90.0f, glm::vec3(8.0f)},
            {glm::vec3(6.0f, 6.8f, 23.0f), 90.0f, glm::vec3(7.8f)}
>>>>>>> Stashed changes
        },
        // Fila 5 (fondo derecho)
        {
            {glm::vec3(32.0f, 5.7f, 23.0f), 90.0f, glm::vec3(10.0f)},
<<<<<<< Updated upstream
            {glm::vec3(-33.0f, 10.6f, -21.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(28.0f, 6.8f, 23.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(32.0f, 5.7f, 13.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(-33.0f, 10.6f, -11.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(28.0f, 6.8f, 13.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(32.0f, 5.7f, -13.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(-30.5f, 10.6f, 15.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(28.0f, 6.8f, -13.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(32.0f, 5.7f, -23.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(-30.5f, 10.6f, 25.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(28.0f, 6.8f, -23.0f), 90.0f, glm::vec3(7.8f)}
=======
            {glm::vec3(32.0f, 10.5f, 23.0f), 90.0f, glm::vec3(8.0f)},
            {glm::vec3(25.0f, 6.8f, 23.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(32.0f, 5.7f, 13.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(32.0f, 10.5f, 13.0f), 90.0f, glm::vec3(8.0f)},
            {glm::vec3(25.0f, 6.8f, 13.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(32.0f, 5.7f, -13.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(32.5f, 10.5f, -13.0f), 90.0f, glm::vec3(8.0f)},
            {glm::vec3(25.0f, 6.8f, -13.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(32.0f, 5.7f, -23.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(32.5f, 10.5f, -23.0f), 90.0f, glm::vec3(8.0f)},
            {glm::vec3(25.0f, 6.8f, -23.0f), 90.0f, glm::vec3(7.8f)}
>>>>>>> Stashed changes
        },
        // Fila 6 (última fila derecha)
        {
            {glm::vec3(51.0f, 5.7f, -23.0f), 90.0f, glm::vec3(10.0f)},
<<<<<<< Updated upstream
            {glm::vec3(-50.0f, 10.6f, 27.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(47.0f, 6.8f, -23.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(51.0f, 5.7f, -13.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(-50.0f, 10.6f, 17.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(47.0f, 6.8f, -13.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(51.0f, 5.7f, 13.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(-53.0f, 10.6f, -11.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(47.0f, 6.8f, 13.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(51.0f, 5.7f, 23.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(-53.0f, 10.6f, -19.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(47.0f, 6.8f, 23.0f), 90.0f, glm::vec3(7.8f)}
=======
            {glm::vec3(51.0f, 10.5f, -23.0f), 90.0f, glm::vec3(8.0f)},
            {glm::vec3(44.0f, 6.8f, -23.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(51.0f, 5.7f, -13.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(51.0f, 10.5f, -13.0f), 90.0f, glm::vec3(8.0f)},
            {glm::vec3(44.0f, 6.8f, -13.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(51.0f, 5.7f, 13.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(51.0f, 10.5f, 13.0f), 90.0f, glm::vec3(8.0f)},
            {glm::vec3(44.0f, 6.8f, 13.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(51.0f, 5.7f, 23.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(51.0f, 10.5f, 23.0f), 90.0f, glm::vec3(8.0f)},
            {glm::vec3(44.0f, 6.8f, 23.0f), 90.0f, glm::vec3(7.8f)}
>>>>>>> Stashed changes
        }
    };

    // Configuración de paredes
    std::vector<ModelInstance> walls = {
        // Pared izquierda
        {glm::vec3(10.0f, 17.0f, -30.7f), 90.0f, glm::vec3(144.0f, 30.0f, 3.0f)},
        // Pared derecha 1
        {glm::vec3(75.0f, 17.0f, 38.0f), 90.0f, glm::vec3(13.0f, 30.0f, 17.0f)},
        // Pared derecha 2
        {glm::vec3(16.0f, 17.0f, 38.0f), 90.0f, glm::vec3(4.0f, 30.0f, 17.0f)},
        // Pared derecha 3
        {glm::vec3(-47.0f, 17.0f, 38.0f), 90.0f, glm::vec3(31.5f, 30.0f, 17.0f)},
        // Parte pared derecha
        {glm::vec3(30.0f, 7.0f, 45.0f), 90.0f, glm::vec3(125.2f, 9.2f, 5.0f)},
        // Pared frontal
        {glm::vec3(1.0f, 17.0f, -82.3f), 0.0f, glm::vec3(63.0f, 30.0f, 3.0f)},
        // Pared trasera
        {glm::vec3(8.9f, 17.0f, 61.0f), 0.0f, glm::vec3(43.3f, 30.0f, 3.0f)}
    };

    // Configuración de ventanas
    std::vector<ModelInstance> windows = {
        {glm::vec3(37.4f, 19.5f, -42.0f), 0.0f, glm::vec3(40.7f, 20.7f, 50.7f)},
        {glm::vec3(37.4f, 19.5f, 11.0f), 0.0f, glm::vec3(40.7f, 20.7f, 50.7f)}
    };

    // Configuración de mesas adicionales (solo una definición)
    ModelInstance teacherDesk = { glm::vec3(70.0f, 5.7f, -20.0f), 90.0f, glm::vec3(10.0f, 10.0f, 18.0f) };
    ModelInstance additionalDesk = { glm::vec3(-44.0f, 5.7f, 20.0f), 90.0f, glm::vec3(10.0f, 10.0f, 18.0f) };

    // Precalcular matrices para objetos estáticos
    const glm::mat4 lampTransform = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.4f, 30.0f, -15.5f)), glm::vec3(40.7f, 42.7f, 00.7f));
    const glm::mat4 ceilingTransform = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.4f, 30.0f, -15.5f)), glm::vec3(60.7f, 5.7f, 150.7f));
    const glm::mat4 floorTransform = glm::scale(glm::translate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)), glm::vec3(2.2f, -5.0f, -10.5f)), glm::vec3(60.7f, 110.7f, 145.7f));//145.7f
    const glm::mat4 boardTransform = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 10.0f, -80.0f)), glm::vec3(30.0f, 14.0f, 25.0f));
    const glm::mat4 frontWallTransform = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 17.0f, -82.3f)), glm::vec3(63.0f, 30.0f, 3.0f));

    while (!glfwWindowShouldClose(window)) {
        GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
        deltaTime = static_cast<GLfloat>(currentFrame - lastFrame);
        lastFrame = currentFrame;

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