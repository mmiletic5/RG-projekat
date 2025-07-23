#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/camera.h>
#include <learnopengl/filesystem.h>
#include <learnopengl/model.h>
#include <learnopengl/shader.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void processInput(GLFWwindow* window);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

auto loadCubemap(vector<std::string> faces) -> unsigned int;

auto loadTexture(const char* path) -> unsigned int;

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight
{
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct ProgramState
{
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 garyPosition = glm::vec3(1.0f, -19.2f, -5.0f);
    float garyScale = 0.005f;

    glm::vec3 housePosition = glm::vec3(1.0f, -20.0f, 1.0f);
    float houseScale = 5.0f;
    float houseRotationX = 0.0f;
    float houseRotationY = 90.0f;
    float houseRotationZ = 0.0f;

    glm::vec3 patrickPosition = glm::vec3(1.0f, -19.20f, 15.0f);
    float patrickScale = 4.0f;
    float patrickRotationX = 0.0f;
    float patrickRotationY = 90.0f;
    float patrickRotationZ = 0.0f;

    glm::vec3 squidPosition = glm::vec3(1.0f, -16.70f, 7.0f);
    float squidScale = 0.01f;
    float squidRotationX = 0.0f;
    float squidRotationY = 90.0f;
    float squidRotationZ = 0.0f;

    glm::vec3 spongePosition = glm::vec3(-4.0f, -19.4f, -9.0f);
    float spongeScale = 4.0f;

    /*glm::vec3 krustyPosition = glm::vec3(28.5f,-19.5f,5.0f);
    float krustyScale = 0.5f;
*/
    glm::vec3 krabsPosition = glm::vec3(28.5f, -19.5f, 15.0f);
    float krabsScale = 2.0f;

    glm::vec3 karenPosition = glm::vec3(22.5f, -19.5f, 15.0f);
    float karenScale = 0.5f;

    // light settings
    bool blinn = false;
    bool blinnKeyPressed = false;

    PointLight pointLight;
    ProgramState() : camera(glm::vec3(25.0f, -16.0f, 32.0f)),pointLight() {}

    void SaveToFile(std::string filename) const;

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) const
{
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename)
{
    std::ifstream in(filename);
    if (in)
    {
        in >> clearColor.r >> clearColor.g >> clearColor.b >> ImGuiEnabled >> camera.Position.x >>
            camera.Position.y >> camera.Position.z >> camera.Front.x >> camera.Front.y >>
            camera.Front.z;
    }
}

ProgramState* programState;

void DrawImGui(ProgramState* programState);

auto main() -> int
{
    // glfw: initialize and configure

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    if (programState->ImGuiEnabled)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state

    glEnable(GL_DEPTH_TEST);

    // blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // facecull
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CW);

    // build and compile shaders
    Shader ourShader(
        "resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader blendingShader("resources/shaders/blending.vs", "resources/shaders/blending.fs");

    // load models
    Model ourModel("resources/objects/gary/gary.obj");
    ourModel.SetShaderTextureNamePrefix("material.");

    stbi_set_flip_vertically_on_load(false);

    Model house("resources/objects/house/house.obj");
    house.SetShaderTextureNamePrefix("material.");

    stbi_set_flip_vertically_on_load(true);
    Model patrick("resources/objects/patrick/patrick.obj");
    patrick.SetShaderTextureNamePrefix("material.");

    Model squid("resources/objects/squid/squid.obj");
    squid.SetShaderTextureNamePrefix("material.");

    Model sponge("resources/objects/sponge/sponge.obj");
    sponge.SetShaderTextureNamePrefix("material.");

    /*    stbi_set_flip_vertically_on_load(false);
        Model krusty("resources/objects/krusty/krusty.obj");
        krusty.SetShaderTextureNamePrefix("material.");
        stbi_set_flip_vertically_on_load(true);
    */
    Model krabs("resources/objects/krabs/krabs.obj");
    krabs.SetShaderTextureNamePrefix("material.");

    Model karen("resources/objects/karen/karenbyanto.obj");
    karen.SetShaderTextureNamePrefix("material.");

    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(4.0f, 4.0, 0.0);
    pointLight.ambient = glm::vec3(2.0f, 2.0f, 2.0f);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    float transparentVertices[] = {
        // positions         // texture Coords (swapped y coordinates because texture is flipped
        // upside down)
        0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, -0.5f, 0.0f, 1.0f, 1.0f,

        0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.5f,  0.0f, 1.0f, 0.0f};

    // transparent VAO
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    unsigned int transparentTexture =
        loadTexture(FileSystem::getPath("resources/textures/kelp.png").c_str());

    vector<glm::vec3> vegetation{
        glm::vec3(18.0f, -12.0f, 25.0f), glm::vec3(18.1f, -12.1f, 25.1f),
        glm::vec3(18.2f, -12.2f, 25.2f), glm::vec3(18.3f, -12.3f, 25.3f),
        glm::vec3(18.4f, -12.4f, 25.4f)};

    blendingShader.use();
    blendingShader.setInt("texture1", 0);

    // skybox
    float skyboxVertices[] = {// positions
                              -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
                              1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

                              -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
                              -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

                              1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

                              -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

                              -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

                              -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
                              1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);

    stbi_set_flip_vertically_on_load(false);

    vector<std::string> faces{
        FileSystem::getPath("resources/textures/skybox/right.jpg"),
        FileSystem::getPath("resources/textures/skybox/left.jpg"),
        FileSystem::getPath("resources/textures/skybox/up.jpg"),
        FileSystem::getPath("resources/textures/skybox/down.jpg"),
        FileSystem::getPath("resources/textures/skybox/front.jpg"),
        FileSystem::getPath("resources/textures/skybox/back.jpg")};
    unsigned int cubemapTexture = loadCubemap(faces);

    // skyboxShader.use();
    // skyboxShader.setInt("skybox", 0);

    // render loop

    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input

        processInput(window);

        // render

        glClearColor(
            programState->clearColor.r, programState->clearColor.g, programState->clearColor.b,
            1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        ourShader.use();
        pointLight.position = glm::vec3(4.0 * cos(currentFrame), 4.0f, 4.0 * sin(currentFrame));
        ourShader.setVec3("pointLight.position", pointLight.position);
        ourShader.setVec3("pointLight.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight.specular", pointLight.specular);
        ourShader.setFloat("pointLight.constant", pointLight.constant);
        ourShader.setFloat("pointLight.linear", pointLight.linear);
        ourShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);
        ourShader.setVec3("lightPos", programState->pointLight.position);
        // view/projection transformations
        glm::mat4 projection = glm::perspective(
            glm::radians(programState->camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f,
            100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // directional light
        ourShader.setVec3("dirLight.direction", glm::vec3(0.5f, 0.7f, 0.4f));
        ourShader.setVec3("dirLight.ambient", glm::vec3(0.3f));
        ourShader.setVec3("dirLight.diffuse", glm::vec3(0.4f));
        ourShader.setVec3("dirLight.specular", glm::vec3(0.2f));

        ourShader.setBool("blinn", programState->blinn);

        glDisable(GL_CULL_FACE);
        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(
            model,
            programState->garyPosition); // translate it down so it's at the center of the scene
        model = glm::scale(
            model,
            glm::vec3(
                programState->garyScale)); // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        // render the house model
        model = glm::mat4(1.0f);
        model = glm::translate(
            model,
            programState->housePosition); // translate it down so it's at the center of the scene
        model = glm::rotate(
            model, glm::radians(programState->houseRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(
            model, glm::radians(programState->houseRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(
            model, glm::radians(programState->houseRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(
            model,
            glm::vec3(
                programState->houseScale)); // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        house.Draw(ourShader);

        // render the patrick model
        model = glm::mat4(1.0f);
        model = glm::translate(
            model,
            programState->patrickPosition); // translate it down so it's at the center of the scene
        model = glm::rotate(
            model, glm::radians(programState->patrickRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(
            model, glm::radians(programState->patrickRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(
            model, glm::radians(programState->patrickRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(
            model,
            glm::vec3(
                programState->patrickScale)); // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        patrick.Draw(ourShader);

        // render the squid model
        model = glm::mat4(1.0f);
        model = glm::translate(
            model,
            programState->squidPosition); // translate it down so it's at the center of the scene
        model = glm::rotate(
            model, glm::radians(programState->squidRotationX), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(
            model, glm::radians(programState->squidRotationY), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(
            model, glm::radians(programState->squidRotationZ), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(
            model,
            glm::vec3(
                programState->squidScale)); // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        squid.Draw(ourShader);

        glEnable(GL_CULL_FACE);
        // render the sponge model
        model = glm::mat4(1.0f);
        model = glm::translate(
            model,
            programState->spongePosition); // translate it down so it's at the center of the scene
        model = glm::scale(
            model,
            glm::vec3(
                programState->spongeScale)); // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        sponge.Draw(ourShader);
        glDisable(GL_CULL_FACE);

        /*// render the krusty model
        model = glm::mat4(1.0f);
        model = glm::translate(model,programState->krustyPosition); // translate it down so it's at
        the center of the scene model = glm::scale(model, glm::vec3(programState->krustyScale)); //
        it's a bit too big for our scene, so scale it down ourShader.setMat4("model", model);
        krusty.Draw(ourShader);
*/
        // render the krabs model
        model = glm::mat4(1.0f);
        model = glm::translate(
            model,
            programState->krabsPosition); // translate it down so it's at the center of the scene
        model = glm::scale(
            model,
            glm::vec3(
                programState->krabsScale)); // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        krabs.Draw(ourShader);

        // render the karen model
        model = glm::mat4(1.0f);
        model = glm::translate(
            model,
            programState->karenPosition); // translate it down so it's at the center of the scene
        model = glm::scale(
            model,
            glm::vec3(
                programState->karenScale)); // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        karen.Draw(ourShader);

        blendingShader.use();
        blendingShader.setMat4("projection", projection);
        blendingShader.setMat4("view", view);
        glBindVertexArray(transparentVAO);
        glBindTexture(GL_TEXTURE_2D, transparentTexture);
        for (auto & i : vegetation)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, i);
            blendingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // draw skybox
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL); // change depth function so depth test passes when values are equal
                                // to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(
            programState->camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS); // set depth function back to default

        if (programState->ImGuiEnabled)
            DrawImGui(programState);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteVertexArrays(1, &skyboxVBO);
    glDeleteVertexArrays(1, &transparentVAO);
    glDeleteVertexArrays(1, &transparentVBO);

    glDeleteTextures(1, &cubemapTexture);
    glDeleteTextures(1, &transparentTexture);

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react
// accordingly

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called

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

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState* programState)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        static float f = 0.0f;
        ImGui::Begin("Hello window");
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float*)&programState->clearColor);
        ImGui::DragFloat3("Gary position", (float*)&programState->garyPosition);
        ImGui::DragFloat("Gary scale", &programState->garyScale, 0.05, 0.1, 4.0);

        ImGui::DragFloat3("House position", (float*)&programState->housePosition);
        ImGui::DragFloat("House scale", &programState->houseScale, 0.05, 0.1, 4.0);

        ImGui::DragFloat3("Patrick position", (float*)&programState->patrickPosition);
        ImGui::DragFloat("Patrick scale", &programState->patrickScale, 0.05, 0.1, 4.0);

        ImGui::DragFloat3("Squid position", (float*)&programState->squidPosition);
        ImGui::DragFloat("Squid scale", &programState->squidScale, 0.05, 0.1, 4.0);

        ImGui::DragFloat3("Sponge position", (float*)&programState->spongePosition);
        ImGui::DragFloat("Sponge scale", &programState->spongeScale, 0.05, 0.1, 4.0);

        /* ImGui::DragFloat3("Krusty position", (float*)&programState->krustyPosition);
         ImGui::DragFloat("Krusty scale", &programState->krustyScale, 0.05, 0.1, 4.0);
 */
        ImGui::DragFloat3("Krabs position", (float*)&programState->krabsPosition);
        ImGui::DragFloat("Krabs scale", &programState->krabsScale, 0.05, 0.1, 4.0);

        ImGui::DragFloat3("Karen position", (float*)&programState->karenPosition);
        ImGui::DragFloat("Karen scale", &programState->karenScale, 0.05, 0.1, 4.0);

        ImGui::DragFloat("pointLight.constant", &programState->pointLight.constant, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.linear", &programState->pointLight.linear, 0.05, 0.0, 1.0);
        ImGui::DragFloat(
            "pointLight.quadratic", &programState->pointLight.quadratic, 0.05, 0.0, 1.0);
        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled)
        {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS and !programState->blinnKeyPressed)
    {
        programState->blinn = !programState->blinn;
        programState->blinnKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
    {
        programState->blinnKeyPressed = false;
    }
}

auto loadCubemap(vector<std::string> faces) -> unsigned int
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); ++i)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB,
                GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

auto loadTexture(char const* path) -> unsigned int
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGB; // izmenio
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
            format == GL_RGBA
                ? GL_CLAMP_TO_EDGE
                : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent
                              // borders. Due to interpolation it takes texels from next repeat
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
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
