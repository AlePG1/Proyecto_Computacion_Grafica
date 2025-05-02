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
bool showComputer = false; // Variable para controlar la visibilidad

// Variables para la animación
float globalAnimationTime = -1.0f;
bool animationPlaying = false;
float animationSpeed = 2.0f;

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
struct ModelInstance {
    glm::vec3 position;
    float rotationY;
    glm::vec3 scale;
};
struct Workstation {
    ModelInstance desk;
    ModelInstance cpu1;
    ModelInstance cpu2;
    ModelInstance chair1;
    ModelInstance chair2;
};

// Función para renderizar una instancia
void RenderInstance(Shader& shader, Model& model, const ModelInstance& ins) {
    glm::mat4 M(1.0f);
    M = glm::rotate(M, glm::radians(ins.rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    M = glm::translate(M, ins.position);
    M = glm::scale(M, ins.scale);
    glUniformMatrix4fv(
        glGetUniformLocation(shader.Program, "model"),
        1, GL_FALSE, glm::value_ptr(M)
    );
    model.Draw(shader);
}


int main() {
    // Inicialización de GLFW/GLEW y ventana
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecto", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW\n";
        return EXIT_FAILURE;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    Shader shader("Shader/lighting.vs", "Shader/lighting.frag");
    Shader shadowShader("Shader/shadow.vs", "Shader/shadow.frag");

    // Cargar modelos de la escena
    Model piso((char*)"Models/Proyecto/piso/piso.obj");
    Model pared((char*)"Models/Proyecto/Pared/pared.obj");
    Model techoo((char*)"Models/Proyecto/piso1/piso.obj");
    Model lampara((char*)"Models/Proyecto/lampled/lampled.obj");
    Model pizarron((char*)"Models/Proyecto/pizarron/pizarron.obj");
    Model cpu((char*)"Models/Proyecto/computadora/computadora.obj");
    Model silla((char*)"Models/Proyecto/silla/silla.obj");
    Model mesa((char*)"Models/Proyecto/mesa/mesa.obj");
    Model ventanas((char*)"Models/Proyecto/ventana/ventana.obj");

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
    std::vector<Workstation> workstations = {
        // Fila 1 (izquierda)
        {
            {glm::vec3(-25.0f, 5.7f, -23.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(-25.0f, 9.1f, -26.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-25.0f, 9.1f, -20.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-30.0f, 6.8f, -26.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(-30.0f, 6.8f, -20.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(-25.0f, 5.7f, -10.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(-25.0f, 9.1f, -14.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-25.0f, 9.1f, -8.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-30.0f, 6.8f, -14.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(-30.0f, 6.8f, -8.0f), 90.0f, glm::vec3(4.8f)}
        },

        // Fila 2 (izquierda)
        {
            {glm::vec3(-10.0f, 5.7f, -23.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(-10.0f, 9.1f, -26.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-10.0f, 9.1f, -20.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-15.0f, 6.8f, -26.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(-15.0f, 6.8f, -20.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(-10.0f, 5.7f, -10.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(-10.0f, 9.1f, -14.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-10.0f, 9.1f, -8.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-15.0f, 6.8f, -14.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(-15.0f, 6.8f, -8.0f), 90.0f, glm::vec3(4.8f)}
        },

        // Fila 3 (izquierda)
        {
            {glm::vec3(5.0f, 5.7f, -23.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(5.0f, 9.1f, -26.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(5.0f, 9.1f, -20.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(0.0f, 6.8f, -26.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(0.0f, 6.8f, -20.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(5.0f, 5.7f, -10.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(5.0f, 9.1f, -14.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(5.0f, 9.1f, -8.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(0.0f, 6.8f, -14.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(0.0f, 6.8f, -8.0f), 90.0f, glm::vec3(4.8f)}
        },

        // Fila 4 (izquierda)
        {
            {glm::vec3(20.0f, 5.7f, -23.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(20.0f, 9.1f, -26.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(20.0f, 9.1f, -20.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(15.0f, 6.8f, -26.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(15.0f, 6.8f, -20.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(20.0f, 5.7f, -10.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(20.0f, 9.1f, -14.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(20.0f, 9.1f, -8.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(15.0f, 6.8f, -14.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(15.0f, 6.8f, -8.0f), 90.0f, glm::vec3(4.8f)}
        },

        // Fila 5 (izquierda)
        {
            {glm::vec3(35.0f, 5.7f, -23.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(35.0f, 9.1f, -26.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(35.0f, 9.1f, -20.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(30.0f, 6.8f, -26.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(30.0f, 6.8f, -20.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(35.0f, 5.7f, -10.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(35.0f, 9.1f, -14.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(35.0f, 9.1f, -8.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(30.0f, 6.8f, -14.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(30.0f, 6.8f, -8.0f), 90.0f, glm::vec3(4.8f)}
        },

        // Fila 1 (derecha)
        {
            {glm::vec3(-25.0f, 5.7f, 18.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(-25.0f, 9.1f, 15.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-25.0f, 9.1f, 21.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-30.0f, 6.8f, 15.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(-30.0f, 6.8f, 21.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(-25.0f, 5.7f, 31.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(-25.0f, 9.1f, 27.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-25.0f, 9.1f, 33.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-30.0f, 6.8f, 27.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(-30.0f, 6.8f, 33.0f), 90.0f, glm::vec3(4.8f)}
        },

        // Fila 2 (derecha)
        {
            {glm::vec3(-10.0f, 5.7f, 18.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(-10.0f, 9.1f, 15.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-10.0f, 9.1f, 21.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-15.0f, 6.8f, 15.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(-15.0f, 6.8f, 21.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(-10.0f, 5.7f, 31.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(-10.0f, 9.1f, 27.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-10.0f, 9.1f, 33.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(-15.0f, 6.8f, 27.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(-15.0f, 6.8f, 33.0f), 90.0f, glm::vec3(4.8f)}
        },

        // Fila 3 (derecha)
        {
            {glm::vec3(5.0f, 5.7f, 18.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(5.0f, 9.1f, 15.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(5.0f, 9.1f, 21.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(0.0f, 6.8f, 15.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(0.0f, 6.8f, 21.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(5.0f, 5.7f, 31.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(5.0f, 9.1f, 27.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(5.0f, 9.1f, 33.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(0.0f, 6.8f, 27.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(0.0f, 6.8f, 33.0f), 90.0f, glm::vec3(4.8f)}
        },

        // Fila 4 (derecha)
        {
            {glm::vec3(20.0f, 5.7f, 18.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(20.0f, 9.1f, 15.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(20.0f, 9.1f, 21.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(15.0f, 6.8f, 15.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(15.0f, 6.8f, 21.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(20.0f, 5.7f, 31.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(20.0f, 9.1f, 27.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(20.0f, 9.1f, 33.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(15.0f, 6.8f, 27.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(15.0f, 6.8f, 33.0f), 90.0f, glm::vec3(4.8f)}
        },

        // Fila 5 (derecha)
        {
            {glm::vec3(35.0f, 5.7f, 18.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(35.0f, 9.1f, 15.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(35.0f, 9.1f, 21.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(30.0f, 6.8f, 15.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(30.0f, 6.8f, 21.0f), 90.0f, glm::vec3(4.8f)}
        },
        {
            {glm::vec3(35.0f, 5.7f, 31.0f), 90.0f, glm::vec3(9.0f, 7.0f, 13.0f)},
            {glm::vec3(35.0f, 9.1f, 27.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(35.0f, 9.1f, 33.0f), 90.0f, glm::vec3(5.5f)},
            {glm::vec3(30.0f, 6.8f, 27.0f), 90.0f, glm::vec3(4.8f)},
            {glm::vec3(30.0f, 6.8f, 33.0f), 90.0f, glm::vec3(4.8f)}
        }
    };

    // Configuración de paredes
    std::vector<ModelInstance> walls = {
        {glm::vec3(10.0f, 17.0f, -30.7f), 90.0f, glm::vec3(144.0f, 30.0f, 3.0f)},
        {glm::vec3(75.0f, 17.0f,  38.0f), 90.0f, glm::vec3(13.0f,  30.0f, 17.0f)},
        {glm::vec3(16.0f, 17.0f,  38.0f), 90.0f, glm::vec3(4.0f,  30.0f, 17.0f)},
        {glm::vec3(-47.0f,17.0f,  38.0f), 90.0f, glm::vec3(31.5f,  30.0f, 17.0f)},
        {glm::vec3(30.0f,  7.0f,  45.0f), 90.0f, glm::vec3(125.2f,  9.2f,  5.0f)},
        {glm::vec3(1.0f, 17.0f, -82.3f),  0.0f, glm::vec3(63.0f, 30.0f,  3.0f)},
        {glm::vec3(8.9f,17.0f,   61.0f),  0.0f, glm::vec3(43.3f, 30.0f,  3.0f)}
    };

    // Configuración de ventanas
    std::vector<ModelInstance> windows = {
        {glm::vec3(37.4f, 19.5f, -42.0f), 0.0f, glm::vec3(40.7f, 20.7f, 50.7f)},
        {glm::vec3(37.4f, 19.5f,  11.0f), 0.0f, glm::vec3(40.7f, 20.7f, 50.7f)}
    };

    // Mesas adicionales
    ModelInstance teacherDesk = { glm::vec3(50.0f, 5.7f, -20.0f), 90.0f, glm::vec3(10.0f,10.0f,18.0f) };
    ModelInstance additionalDesk = { glm::vec3(-10.0f,5.7f, 20.0f),  90.0f, glm::vec3(10.0f,10.0f,18.0f) };

    // Inicializar animaciones de componentes
    components = {
     {&gabinete,       CreateRandomRotationKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 1.2f},
     {&placamadre,     CreateRandomRotationKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 1.0f},
     {&procesador,     CreateRandomRotationKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 0.8f},
     {&ram,            CreateRandomRotationKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 0.8f},
     {&ssd,            CreateRandomRotationKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 0.8f},
     {&tarjetagrafica, CreateRandomRotationKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 1.0f},
     {&ventilador,     CreateRandomRotationKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 0.6f},
     {&ventilador2,    CreateRandomRotationKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 0.6f},
     {&fuente,         CreateRandomRotationKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 1.0f},
     {&monitor,        CreateRandomRotationKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 1.2f},
     {&teclado,        CreateRandomRotationKeyframes(glm::vec3(0.0f), glm::vec3(3.0f), 1.0f), false, false, 0.0f, 1.0f}
    };


    // Matrices precalculadas para objetos estáticos
    const glm::mat4 lampTransform = glm::scale(glm::translate(glm::mat4(1.0f),
        glm::vec3(0.4f, 30.0f, -15.5f)),
        glm::vec3(40.7f, 42.7f, 00.7f));
    const glm::mat4 ceilingTransform = glm::scale(glm::translate(glm::mat4(1.0f),
        glm::vec3(0.4f, 30.0f, -15.5f)),
        glm::vec3(60.7f, 5.7f, 150.7f));
    const glm::mat4 floorTransform = glm::scale(glm::translate(
        glm::rotate(glm::mat4(1.0f),
            glm::radians(90.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)),
        glm::vec3(2.2f, -5.0f, -10.5f)),
        glm::vec3(60.7f, 110.7f, 145.7f));//145.7f
    const glm::mat4 boardTransform = glm::scale(glm::translate(glm::mat4(1.0f),
        glm::vec3(1.0f, 10.0f, -80.0f)),
        glm::vec3(30.0f, 14.0f, 25.0f));
    const glm::mat4 frontWallTransform = glm::scale(glm::translate(glm::mat4(1.0f),
        glm::vec3(1.0f, 17.0f, -82.3f)),
        glm::vec3(63.0f, 30.0f, 3.0f));

    // Bucle principal
    while (!glfwWindowShouldClose(window)) {

        GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (keys[GLFW_KEY_R]) {
            if (!animationPlaying) {
                showComputer = true; // Hacer visible la computadora
                globalAnimationTime = -1.0f;
                animationPlaying = true;
                // Reiniciar estados de componentes
                for (auto& comp : components) {
                    comp.isAnimating = false;
                    comp.hasAnimated = false;
                }
            }
            keys[GLFW_KEY_R] = false;
        }

        // Actualizar animaciones
        UpdateAnimations(globalAnimationTime);

        // Eventos y movimiento de cámara
        glfwPollEvents();
        DoMovement();

        // Clear
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();

        // Configurar luces
        // Luz direccional
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.direction"),
            -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.ambient"),
            0.5f, 0.5f, 0.5f);
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.diffuse"),
            0.8f, 0.8f, 0.8f);
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.specular"),
            0.5f, 0.5f, 0.5f);

        // Luz puntual (lámpara)
        glm::vec3 lightPos(0.4f, 20.0f, -10.5f);
        glUniform3f(glGetUniformLocation(shader.Program,
            "pointLights[0].position"),
            lightPos.x, lightPos.y, lightPos.z);
        glUniform3f(glGetUniformLocation(shader.Program,
            "pointLights[0].ambient"),
            0.2f, 0.2f, 0.2f);
        glUniform3f(glGetUniformLocation(shader.Program,
            "pointLights[0].diffuse"),
            0.8f, 0.8f, 0.8f);
        glUniform3f(glGetUniformLocation(shader.Program,
            "pointLights[0].specular"),
            1.0f, 1.0f, 1.0f);
        glUniform1f(glGetUniformLocation(shader.Program,
            "pointLights[0].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(shader.Program,
            "pointLights[0].linear"), 0.09f);
        glUniform1f(glGetUniformLocation(shader.Program,
            "pointLights[0].quadratic"), 0.032f);

        // Spotlight (cámara)
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.position"),
            camera.GetPosition().x, camera.GetPosition().y,
            camera.GetPosition().z);
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.direction"),
            camera.GetFront().x, camera.GetFront().y,
            camera.GetFront().z);
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.ambient"),
            0.1f, 0.1f, 0.1f);
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.diffuse"),
            0.8f, 0.8f, 0.8f);
        glUniform3f(glGetUniformLocation(shader.Program, "spotLight.specular"),
            1.0f, 1.0f, 1.0f);
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.linear"), 0.09f);
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.quadratic"), 0.032f);
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.cutOff"),
            glm::cos(glm::radians(12.5f)));
        glUniform1f(glGetUniformLocation(shader.Program, "spotLight.outerCutOff"),
            glm::cos(glm::radians(15.0f)));

        // Material
        glUniform3f(glGetUniformLocation(shader.Program, "material.specular"),
            0.5f, 0.5f, 0.5f);
        glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"),
            32.0f);

        // Vista y proyección
        glm::mat4 view = camera.GetViewMatrix();
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

        // Renderizar ventanas
        for (const auto& wi : windows) {
            RenderInstance(shader, ventanas, wi);
        }

        // Lámpara
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(lampTransform));
        lampara.Draw(shader);

        // Techo
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(ceilingTransform));
        techoo.Draw(shader);

        // Piso
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(floorTransform));
        piso.Draw(shader);

        // Paredes
        for (const auto& w : walls) {
            RenderInstance(shader, pared, w);
        }

        // Pared frontal
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(frontWallTransform));
        pared.Draw(shader);

        // Pizarrón
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(boardTransform));
        pizarron.Draw(shader);

        // Definición de instancias de computadoras
        std::vector<ComputerInstance> computerInstances = {
            // fila 1 (izquierda):
            { glm::vec3(-24.0f, 9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-18.0f, 9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-12.0f, 9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-6.0f,  9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },

            // fila 2 (izquierda:
            { glm::vec3(-24.0f, 9.0f, 10.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-18.0f, 9.0f, 10.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-12.0f, 9.0f, 10.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-6.0f,  9.0f, 10.0f), 180.0f, glm::vec3(3.0f) },

            // fila 3 (izquierda:
            { glm::vec3(-24.0f, 9.0f, -5.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-18.0f, 9.0f, -5.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-12.0f, 9.0f, -5.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-6.0f,  9.0f, -5.0f), 180.0f, glm::vec3(3.0f) },

            // fila 4 (izquierda:
            { glm::vec3(-24.0f, 9.0f, -20.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-18.0f, 9.0f, -20.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-12.0f, 9.0f, -20.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-6.0f,  9.0f, -20.0f), 180.0f, glm::vec3(3.0f) },

            // fila 5 (izquierda:
            { glm::vec3(-24.0f, 9.0f, -35.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-18.0f, 9.0f, -35.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-12.0f, 9.0f, -35.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(-6.0f,  9.0f, -35.0f), 180.0f, glm::vec3(3.0f) },

            // fila 1 (derecha):
            { glm::vec3(16.0f, 9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(22.0f, 9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(28.0f, 9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(34.0f, 9.0f, 25.0f), 180.0f, glm::vec3(3.0f) },

            // fila 2 (derecha):
            { glm::vec3(16.0f, 9.0f, 10.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(22.0f, 9.0f, 10.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(28.0f, 9.0f, 10.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(34.0f, 9.0f, 10.0f), 180.0f, glm::vec3(3.0f) },

            // fila 3 (derecha):
            { glm::vec3(16.0f, 9.0f, -5.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(22.0f, 9.0f, -5.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(28.0f, 9.0f, -5.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(34.0f, 9.0f, -5.0f), 180.0f, glm::vec3(3.0f) },

            // fila 4 (derecha):
            { glm::vec3(16.0f, 9.0f, -20.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(22.0f, 9.0f, -20.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(28.0f, 9.0f, -20.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(34.0f, 9.0f, -20.0f), 180.0f, glm::vec3(3.0f) },

            // fila 5 (derecha):
            { glm::vec3(16.0f, 9.0f, -35.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(22.0f, 9.0f, -35.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(28.0f, 9.0f, -35.0f), 180.0f, glm::vec3(3.0f) },
            { glm::vec3(34.0f, 9.0f, -35.0f), 180.0f, glm::vec3(3.0f) },

        };

        //RenderComputer(shader, globalAnimationTime);
        for (const auto& ci : computerInstances) {
            // 1) monta la matriz padre para ESTA instancia
            glm::mat4 compModel = glm::mat4(1.0f);
            compModel = glm::translate(compModel, ci.position);
            compModel = glm::rotate(compModel,
                glm::radians(ci.rotationY),
                glm::vec3(0.0f, 1.0f, 0.0f));
            compModel = glm::scale(compModel, ci.scale);

            // 2) dibuja todos los componentes bajo esa transformación
            RenderComputer(shader, globalAnimationTime, compModel);
        }


        // Mesas (madera, medio brillo)
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.ambient"),
            0.3f, 0.3f, 0.3f);
        glUniform3f(glGetUniformLocation(shader.Program, "dirLight.diffuse"),
            0.9f, 0.9f, 0.9f);
        glUniform3f(glGetUniformLocation(shader.Program, "material.specular"),
            0.2f, 0.2f, 0.2f);
        glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"),
            30.0f);

        // Render puestos de trabajo
        for (const auto& ws : workstations) {
            RenderInstance(shader, mesa, ws.desk);
            RenderInstance(shader, cpu, ws.cpu1);
            RenderInstance(shader, cpu, ws.cpu2);
            RenderInstance(shader, silla, ws.chair1);
            RenderInstance(shader, silla, ws.chair2);
        }
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