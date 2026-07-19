#include "physique.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <vector>
#include <iostream>

// On crée un vertex shader pour dire à OpenGL ou placer chaque point
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;

uniform vec2 offset;
uniform float scale;

void main() {
    gl_Position = vec4(aPos * scale + offset, 0.0, 1.0);
}
)";

// On crée un fragment shader pour dire à OpenGL de quelle couleur on doit afficher les pixels
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

uniform vec3 color;

void main() {
    FragColor = vec4(color, 1.0);
}
)";

// Fonction pour compiler un shader et vérifier qu'y a pas d'erreurs
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int succes;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &succes);

    if (!succes) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cout << "Erreur de compilation du shader:\n" << log << std::endl; 
    }
    return shader;
}

int main() {
    const int WIDTH = 800, HEIGHT = 600;

    const double G = 6.674e-11, SCALE = 4e-12;
    const int dt = 86400;

    std::vector<Corps> listeCorps = {
        Corps("soleil", 0.0, 0.0, 0.0, 0.0, 1.989e30, 0.1f, Vector3(1.0f, 1.0f, 0.0f)),
        Corps("mercure", 5.79e10, 0.0, 0.0, 47362.0, 3.285e23, 0.03f, Vector3(0.6f, 0.6f, 0.6f)),
        Corps("venus", 1.082e11, 0.0, 0.0, 35025.0, 4.867e24, 0.04f, Vector3(0.9f, 0.7f, 0.3f)),
        Corps("terre", 1.496e11, 0.0, 0.0, 29290.0, 5.972e24, 0.05f, Vector3(0.0f, 0.4f, 1.0f)),
        Corps("mars", 2.279e11, 0.0, 0.0, 24130.0, 6.418e23, 0.04f, Vector3(1.0f, 0.3f, 0.0f))
};

    Univers univers(listeCorps, G, dt);

    // Chargement de GLFW qui sert à créer la fenêtre
    if (!glfwInit()) {
        std::cout << "Erreur: impossible d'utiliser GLFW..." << std::endl;
        return -1;
    }

    // Version d'OpenGL qu'on veut utiliser (3.3 Core) jsp pourquoi celle là
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Création de la fenêtre (c'est le même bordel qu'avec SFML)
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Simulateur N-Corps", nullptr, nullptr);
    if (!window) {
        std::cout << "Erreur: impossible d'ouvrir la fenêtre..." << std::endl;
        glfwTerminate();
        return -1;
    }

    // Definition de la fenêtre qu'on veut utiliser (on doit le dire à OpenGL)
    glfwMakeContextCurrent(window);

    // Chargement des fonctions OpenGL (c'est GLAD qui le fait askip)
    if (!gladLoadGL(glfwGetProcAddress)) return -1;

    // On donne à OpenGL la vraie surface sur laquelle il peut dessiner 
    // pour éviter les décallages (selon la résolution de l'écran surtout)
    int frameBufferHeight, frameBufferWidth;
    glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
    glViewport(0, 0, frameBufferWidth, frameBufferHeight);

    // Initialisation de ImGUI pour la création de l'interface
    // et connection avec GLFW et OpenGL
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io; 

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Compiler les shaders et les rassembler dans un programme
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    //  Quand on les a mis dans le programme on a plus besoin de les avoir individuellement
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // On récupère la variable qui definit le 'décallage' entre chaque corps (cf. vertex shader)
    GLint offsetLocation = glGetUniformLocation(shaderProgram, "offset");
    GLint colorLocation = glGetUniformLocation(shaderProgram, "color");
    GLint scaleLocation = glGetUniformLocation(shaderProgram, "scale");

    // On va générer les points du cercle qu'on veut afficher
    std::vector<float> sommets;
    int segments = 64;
    float rayon = 1.0f;

    sommets.push_back(0.0f);
    sommets.push_back(0.0f);

    for (int i = 0; i <= segments; i++) {
        // Calcul de l'angle du segment
        float angle = 2.0f * M_PI * i / segments;
        
        // Calcul de la position de chaque sommet
        sommets.push_back(rayon * std::cos(angle) * HEIGHT * (1.0f / WIDTH));
        sommets.push_back(rayon * std::sin(angle));
    }

    // Maintenant on peut envoyer les points à la carte graphique
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sommets.size() * sizeof(float), sommets.data(), GL_STATIC_DRAW);

    // Après on doit dire à OpenGL comment interprêter les données qu'on a stocké
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Boucle principale tant que la fenêtre est pas fermée
    while (!glfwWindowShouldClose(window)) {

        // Definition de la couleur d'arrière-plan
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Création d'une nouvelle fenêtre d'interface
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Là on modifie notre fenêtre avec ce qu'on veut
        ImGui::Begin("Infos de simulation");
        
        ImGui::Text("Nombre de Corps: %d", (int)listeCorps.size());
        ImGui::Text("Pas de temps (dt): %d secondes", dt);
        ImGui::Separator();

        static int solveur = 0;
        ImGui::Text("Choix du solveur:");
        ImGui::RadioButton("Euler", &solveur, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Leapfrog", &solveur, 1);
        ImGui::Separator();

        for (Corps& corps : listeCorps) {
            ImGui::Text("%s", corps.nom.c_str());
            ImGui::Text("  Distance soleil: %2e m", corps.position.norme());
            ImGui::Text("  Vitesse: %.2e m/s",  corps.vitesse.norme());
        }

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // On appelle les fonctions de simulation pour mettre à jour la position des corps
        if (solveur == 0) univers.EulerStep();
        else univers.LeapfrogStep();

        // On active nos shaders et nos points, et on les dessine
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        for (Corps& corps : listeCorps) {
            Vector2 offset = corps.position * SCALE;
            glUniform1f(scaleLocation, corps.rayon);
            glUniform2f(offsetLocation, offset.x, offset.y);
            glUniform3f(colorLocation, corps.couleur.x, corps.couleur.y, corps.couleur.z);
            glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);

        }

        // On affiche tout ce qu'on a créé et on gère les évènements clavier/souris
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}