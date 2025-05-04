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
void Animation();

Camera camera(glm::vec3(0.0f, 15.0f, 3.0f));
bool keys[1024];
GLfloat lastX = WIDTH / 2.0, lastY = HEIGHT / 2.0;
bool firstMouse = true;
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

<<<<<<< Updated upstream
// Estructura para configurar instancias de modelos
=======
// Variables para la animación
float globalAnimationTime = -1.0f;
bool animationPlaying = false;
float animationSpeed = 2.0f;

float rotBall = 0;
float transBall = 0; //Valor inicial de transformacion de la pelota
bool AnimBall = false; //Bandera para indicar si la animacion esta activa
bool arriba = true; //Bandera para indicar la direccion
bool rayo = false; //Bandera para indicar si el rayo se activa

// Positions of the point lights
glm::vec3 pointLightPositions[] = {
    glm::vec3(0.0f,2.0f, 0.0f),
    glm::vec3(0.0f,0.0f, 0.0f),
    glm::vec3(0.0f,0.0f,  0.0f),
    glm::vec3(0.0f,0.0f, 0.0f)
};

// Estructura para keyframes de animación
struct Keyframe {
    float time;
    glm::vec3 position;
    float rotation;
    glm::vec3 scale;
};

// Estructura para componentes de la computadora
struct ComputerComponent {
    Model* model;
    std::vector<Keyframe> keyframes;
    bool isAnimating;
    bool hasAnimated;
    float animationStartTime;
    float animationDuration;
};

struct ComputerInstance {
    glm::vec3 position;
    float     rotationY;
    glm::vec3 scale;
};

std::vector<ComputerComponent> components;

// Función para interpolar entre keyframes
Keyframe InterpolateKeyframes(const Keyframe& a, const Keyframe& b, float t) {
    Keyframe result;
    result.time = glm::mix(a.time, b.time, t);
    result.position = glm::mix(a.position, b.position, t);
    result.rotation = glm::mix(a.rotation, b.rotation, t);
    result.scale = glm::mix(a.scale, b.scale, t);
    return result;
}

// Función para obtener el keyframe actual
Keyframe GetCurrentKeyframe(const ComputerComponent& component, float currentTime) {
    if (component.keyframes.empty())
        return Keyframe{ 0, glm::vec3(0.0f), 0.0f, glm::vec3(1.0f) };

    float animTime = currentTime - component.animationStartTime;
    if (animTime <= component.keyframes.front().time)
        return component.keyframes.front();
    if (animTime >= component.keyframes.back().time)
        return component.keyframes.back();

    for (size_t i = 0; i + 1 < component.keyframes.size(); ++i) {
        if (animTime >= component.keyframes[i].time &&
            animTime < component.keyframes[i + 1].time)
        {
            float t = (animTime - component.keyframes[i].time) /
                (component.keyframes[i + 1].time - component.keyframes[i].time);
            return InterpolateKeyframes(component.keyframes[i], component.keyframes[i + 1], t);
        }
    }
    return component.keyframes.back();
}


// Función para actualizar animaciones en secuencia
void UpdateAnimations(float currentTime) {
    if (!animationPlaying) return;

    if (currentTime < 0.0f) {
        globalAnimationTime = 0.0f;
        // Reiniciar todos los componentes
        for (auto& comp : components) {
            comp.isAnimating = false;
            comp.hasAnimated = false;
        }
        // Empezar solo el primero
        if (!components.empty()) {
            components[0].isAnimating = true;
            components[0].animationStartTime = 0.0f;
        }
        return;
    }

    globalAnimationTime += deltaTime * animationSpeed;

    // Controlar la secuencia de animación componente por componente
    for (size_t i = 0; i < components.size(); ++i) {
        if (!components[i].hasAnimated) {
            // Si es el primer componente o el anterior ya terminó
            if (!components[i].isAnimating &&
                (i == 0 || components[i - 1].hasAnimated))
            {
                components[i].isAnimating = true;
                components[i].animationStartTime = globalAnimationTime;
            }
            break;
        }
    }

    // Verificar si todos han terminado
    bool allDone = true;
    for (auto& comp : components) {
        if (!comp.hasAnimated) {
            allDone = false;
            break;
        }
    }

    if (allDone) {
        animationPlaying = false;
    }
}

std::vector<Keyframe> CreateRandomRotationKeyframes(
    const glm::vec3& finalPos,
    const glm::vec3& finalScale,
    float duration)
{
    std::vector<Keyframe> kfs;

    // Generar valores aleatorios para la animación
    float randomRotation = static_cast<float>(rand() % 1080); // Hasta 3 vueltas completas
    float direction = (rand() % 2) ? 1.0f : -1.0f;
    randomRotation *= direction;

    // Generar ángulo inicial aleatorio para la órbita
    float orbitAngle = static_cast<float>(rand() % 360);

    // Keyframe inicial (posición original)
    kfs.push_back({ 0.0f, finalPos, 0.0f, finalScale });

    // Puntos intermedios con movimiento orbital
    for (int i = 1; i <= 5; ++i) {
        float progress = i * 0.2f;
        float currentOrbitAngle = orbitAngle + 360.0f * progress;

        // Calcular posición orbital (radio de 2 unidades)
        glm::vec3 orbitPos = finalPos + glm::vec3(
            2.0f * sin(glm::radians(currentOrbitAngle)),
            0.0f,
            2.0f * cos(glm::radians(currentOrbitAngle))
        );

        kfs.push_back({
            duration * progress,
            orbitPos,
            randomRotation * progress,
            finalScale
            });
    }

    // Final suavizado a posición original
    kfs.push_back({ duration * 0.9f,
                   finalPos + glm::vec3(0.5f * sin(glm::radians(orbitAngle + 324.0f)), 0.0f, 0.5f * cos(glm::radians(orbitAngle + 324.0f))),
                   randomRotation * 0.9f,
                   finalScale });
    kfs.push_back({ duration, finalPos, 0.0f, finalScale });

    return kfs;
}

void RenderComponent(Shader& shader,
    ComputerComponent& component,
    float currentTime,
    const glm::mat4& parentTransform)
{
    Keyframe cf = GetCurrentKeyframe(component, currentTime);

    // Matriz de transformación con rotación y posición orbital
    glm::mat4 localM = glm::mat4(1.0f);
    localM = glm::translate(localM, cf.position); // Posición orbital
    localM = glm::rotate(localM, glm::radians(cf.rotation), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotación propia

    // Matriz final
    glm::mat4 modelMatrix = parentTransform * localM;
    glUniformMatrix4fv(
        glGetUniformLocation(shader.Program, "model"),
        1, GL_FALSE, glm::value_ptr(modelMatrix)
    );

    // Animación de color para resaltar el componente activo
    float progress = glm::clamp(
        (currentTime - component.animationStartTime) / component.animationDuration,
        0.0f, 1.0f
    );

    if (component.isAnimating) {
        // Color que cambia durante la animación
        glm::vec3 animColor(
            0.5f + 0.5f * sin(progress * 3.1416f),
            0.5f - 0.5f * cos(progress * 3.1416f),
            0.7f
        );
        glUniform3f(
            glGetUniformLocation(shader.Program, "material.diffuse"),
            animColor.r, animColor.g, animColor.b
        );
    }

    component.model->Draw(shader);

    // Restaurar color original
    if (component.isAnimating) {
        glUniform3f(
            glGetUniformLocation(shader.Program, "material.diffuse"),
            1.0f, 1.0f, 1.0f
        );
    }

    // Marcar como completado cuando termine
    if (progress >= 1.0f) {
        component.isAnimating = false;
        component.hasAnimated = true;
    }
}

void RenderComputer(Shader& shader,
    float currentTime,
    const glm::mat4& parentTransform)
{
    if (!showComputer && !animationPlaying) return; // No renderizar si no se debe mostrar

    for (auto& comp : components) {
        RenderComponent(shader, comp, currentTime, parentTransform);
    }
}

// Estructuras para el resto de la escena
>>>>>>> Stashed changes
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
    //Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
    Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");

    Model piso((char*)"Models/Proyecto/piso/piso.obj"); // Cargar modelo del piso
    Model pared((char*)"Models/Proyecto/Pared/tripo_pbr_model_01b89d8b-a342-4896-89aa-fe20ab730130.obj"); // Cargar modelo de pared
    Model techoo((char*)"Models/Proyecto/piso1/piso.obj");   // Cargar modelo del techo
    Model lampara((char*)"Models/Proyecto/lampled/lampled.obj"); // Cargar modelo de lámpara
    Model pizarron((char*)"Models/Proyecto/pizarron/pizarron.obj"); // Cargar modelo del pizarrón
    Model cpu((char*)"Models/Proyecto/computadora/computadora.obj"); // Cargar modelo de computadora
    Model silla((char*)"Models/Proyecto/silla/silla.obj"); // Cargar modelo de silla
    Model mesa((char*)"Models/Proyecto/mesa/mesa.obj");   // Cargar modelo de mesa
    Model ventanas((char*)"Models/Proyecto/ventana/ventana.obj");   // Cargar modelo de ventana

<<<<<<< Updated upstream
    // Configurar puestos de trabajo (mesa, cpu, silla)
=======
    //Cargar modelos de la nave
    Model Nave((char*)"Models/nave_espacial/nave_espacial.obj");
    Model Rayo((char*)"Models/rayo_laser/Rayo45.obj");

    // Cargar componentes de computadora (animables)
    Model gabinete((char*)"Models/Proyecto/gabinete/gabinete.obj");
    Model placamadre((char*)"Models/Proyecto/placamadre/placamadre.obj");
    Model procesador((char*)"Models/Proyecto/procesador/procesador.obj");
    Model ram((char*)"Models/Proyecto/ram/ram.obj");
    Model ssd((char*)"Models/Proyecto/ssd/ssd.obj");
    Model tarjetagrafica((char*)"Models/Proyecto/tarjetagrafica/tarjetagrafica.obj");
    Model ventilador((char*)"Models/Proyecto/ventilador/ventilador.obj");
    Model ventilador2((char*)"Models/Proyecto/ventilador2/ventilador.obj");
    Model fuente((char*)"Models/Proyecto/fuente/fuente.obj");
    Model monitor((char*)"Models/Proyecto/monitor/monitor.obj");
    Model teclado((char*)"Models/Proyecto/teclado/teclado.obj");

    // Configurar puestos de trabajo
>>>>>>> Stashed changes
    std::vector<Workstation> workstations = {
        // Fila 1 (izquierda)
        {
            {glm::vec3(-25.0f, 5.7f, -23.0f), 90.0f, glm::vec3(10.0f)}, // mesa
            {glm::vec3(26.0f, 10.6f, 20.0f), -95.0f, glm::vec3(8.0f)},  // cpu
            {glm::vec3(-29.0f, 6.8f, -23.0f), 90.0f, glm::vec3(7.8f)}   // silla
        },
        {
            {glm::vec3(-25.0f, 5.7f, -13.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(25.5f, 10.6f, 11.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(-29.0f, 6.8f, -13.0f), 90.0f, glm::vec3(7.8f)}
        },
        // Fila 2
        {
            {glm::vec3(-25.0f, 5.7f, 13.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(23.8f, 10.6f, -16.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(-29.0f, 6.8f, 13.0f), 90.0f, glm::vec3(7.8f)}
        },
        {
            {glm::vec3(-25.0f, 5.7f, 23.0f), 90.0f, glm::vec3(10.0f)},
            {glm::vec3(22.8f, 10.6f, -25.0f), -95.0f, glm::vec3(8.0f)},
            {glm::vec3(-29.0f, 6.8f, 23.0f), 90.0f, glm::vec3(7.8f)}
        },
        // Fila 3 (centro)
        {
            {glm::vec3(-6.0f, 5.7f, 23.0f), 90.0f, glm::vec3(10.0f)},
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
        },
        // Fila 4 (derecha)
        {
            {glm::vec3(13.0f, 5.7f, -13.0f), 90.0f, glm::vec3(10.0f)},
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
        },
        // Fila 5 (fondo derecho)
        {
            {glm::vec3(32.0f, 5.7f, 23.0f), 90.0f, glm::vec3(10.0f)},
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
        },
        // Fila 6 (última fila derecha)
        {
            {glm::vec3(51.0f, 5.7f, -23.0f), 90.0f, glm::vec3(10.0f)},
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
        Animation();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();

        // OpenGL options
        glEnable(GL_DEPTH_TEST);

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
<<<<<<< Updated upstream
        glm::mat4 projection = glm::perspective(camera.GetZoom(), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3f(glGetUniformLocation(shader.Program, "viewPos"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
=======
        glm::mat4 projection = glm::perspective(camera.GetZoom(),
            (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
            0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"),
            1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"),
            1, GL_FALSE, glm::value_ptr(view));
        glUniform3f(glGetUniformLocation(shader.Program, "viewPos"),
            camera.GetPosition().x,
            camera.GetPosition().y,
            camera.GetPosition().z);
        
        // Get the uniform locations
        shader.Use();

        GLint modelLoc = glGetUniformLocation(shader.Program, "model");
        GLint viewLoc = glGetUniformLocation(shader.Program, "view");
        GLint projLoc = glGetUniformLocation(shader.Program, "projection");

        // Pass the matrices to the shader
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


        glm::mat4 model(1);
>>>>>>> Stashed changes

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

<<<<<<< Updated upstream
        // Cambiar material para mesas (madera - medio brillo)
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.ambient"), 0.3f, 0.3f, 0.3f);
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.diffuse"), 0.9f, 0.9f, 0.9f);
        glUniform3f(glGetUniformLocation(shader.Program, "material.specular"), 0.2f, 0.2f, 0.2f);
        glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"), 30.0f);
=======
        //Nave
        model = glm::mat4(1);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(shader.Program, "transparency"), 0);
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
        model = glm::translate(model, glm::vec3(0.0f, 5.0f, 0.0f));
        model = glm::translate(model, glm::vec3(transBall, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotBall), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        Nave.Draw(shader);
        //Rayo
        if (rayo == true) {
            model = glm::mat4(1);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1i(glGetUniformLocation(shader.Program, "transparency"), 0);
            model = glm::translate(model, glm::vec3(0.0f, 1.6f, 1.75f));


            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            Rayo.Draw(shader);
        }

        // Definición de instancias de computadoras
        std::vector<ComputerInstance> computerInstances = {
            // fila 1 (izquierda):
            { glm::vec3(-24.0f, 9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-18.0f, 9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-12.0f, 9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-6.0f,  9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },
>>>>>>> Stashed changes

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
    //Tecla para activar la animación de la nave
    if (keys[GLFW_KEY_N] && action == GLFW_PRESS)
    {
        AnimBall = !AnimBall;

    }
}

void Animation() {
    float velocidad = 0.0004f;
    float tolerancia = 0.001f;
    float meta = pointLightPositions[0][0];  // Meta en coordenadas del mundo
    float origenModelo = -3.0f;              // El origen de tu modelo

    if (!AnimBall) return;

    float posicionActual = origenModelo + transBall;  // Posición actual real en X
    rotBall += 0.02f;

    if (posicionActual < meta - tolerancia) {
        transBall += velocidad;
        if (origenModelo + transBall > meta) {
            transBall = meta - origenModelo;  // Ajustar para no pasarse
        }
    }
    else if (posicionActual > meta + tolerancia) {
        transBall -= velocidad;
        if (origenModelo + transBall < meta) {
            transBall = meta - origenModelo;
        }
    }
    else {
        transBall = meta - origenModelo;  // Posición exacta final
        AnimBall = false;                 // Detener animación si lo deseas
        rotBall = 0.0f;
        rayo = true;
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