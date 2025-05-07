#include <iostream>
#include <cmath>

// GLEW
#include <GL/glew.h> // Extensiones de OpenGL

// GLFW
#include <GLFW/glfw3.h> // Ventanas y entrada (teclado/ratón)

// Otras librerías
#include "stb_image.h" // Carga de imágenes

// Matemáticas de GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // Para transformaciones: translate, rotate, scale
#include <glm/gtc/type_ptr.hpp>			// Para pasar matrices a shaders

// Carga de modelos
#include "SOIL2/SOIL2.h"

// Archivos de cabecera
#include "Shader.h" // Clase para cargar y compilar shaders
#include "Camera.h" // Clase para controlar la cámara
#include "Model.h"  // Clase para cargar y dibujar modelos OBJ

// Prototipos de funciones
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode); // Entrada de teclado
void MouseCallback(GLFWwindow* window, double xPos, double yPos); // Movimiento del ratón
void DoMovement(); // Mover la cámara según entrada

//Función auxiliar para colocar las luces puntuales
void SetPointLight(int index, glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular,
	float constant, float linear, float quadratic, GLuint shaderProgram) {
	std::string base = "pointLights[" + std::to_string(index) + "]";
	glUniform3f(glGetUniformLocation(shaderProgram, (base + ".position").c_str()), position.x, position.y, position.z);
	glUniform3f(glGetUniformLocation(shaderProgram, (base + ".ambient").c_str()), ambient.r, ambient.g, ambient.b);
	glUniform3f(glGetUniformLocation(shaderProgram, (base + ".diffuse").c_str()), diffuse.r, diffuse.g, diffuse.b);
	glUniform3f(glGetUniformLocation(shaderProgram, (base + ".specular").c_str()), specular.r, specular.g, specular.b);
	glUniform1f(glGetUniformLocation(shaderProgram, (base + ".constant").c_str()), constant);
	glUniform1f(glGetUniformLocation(shaderProgram, (base + ".linear").c_str()), linear);
	glUniform1f(glGetUniformLocation(shaderProgram, (base + ".quadratic").c_str()), quadratic);
}

//Función auxiliar para dibujar los modelos de manera más efectiva
void DrawModel(Model& modelo, glm::vec3 posicion, float rotY, glm::vec3 escala, GLuint modelLoc, Shader& shader) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, posicion);
	model = glm::rotate(model, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, escala);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	modelo.Draw(shader);
}

// Dimensiones de la ventana
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Cámara (posición inicial en (0, 0, 3))
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;
bool usePerspective = true; // true = perspectiva, false = ortogonal

// Atributos de la iluminación
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);   // Posición de la lámpara principal
bool active;							// Control para encender/apagar luz dinámica

// Posiciones de las luces puntuales
glm::vec3 pointLightPositions[] = {
	glm::vec3(32.0f, 10.0f, 32.0f),
	glm::vec3(32.0f, 10.0f, -32.0f),
	glm::vec3(-32.0f, 10.0f, 32.0f),
	glm::vec3(-32.0f, 10.0f, -32.0f)
};

// Cubo creado con ayuda de vértices
float vertices[] = {
	 -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

glm::vec3 Light1 = glm::vec3(0);

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

int main()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	/*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);*/

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Fuentes de luz", nullptr, nullptr);

	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Set the required callback functions
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	// GLFW Options
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	// Define the viewport dimensions
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
	Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");

	// ########## CARGA DE MODELOS ##########
	Model Tablero((char*)"Models/tablero.obj");

	// EQUIPO: Minecraft
	Model Zombie((char*)"Models/zombie.obj");
	Model Steve((char*)"Models/steve.obj");
	Model Alex((char*)"Models/alex.obj");
	Model Esqueleto((char*)"Models/Esqueleto.obj");
	Model Slime((char*)"Models/slime.obj");
	Model Creeper((char*)"Models/creeper.obj");

	// EQUIPO: Plants vs Zombies
	Model Lanzaguisantes((char*)"Models/peonpeashooter.obj");
	Model Girasol((char*)"Models/Reina.obj");
	Model Fred((char*)"Models/Rey.obj");
	Model Cactus((char*)"Models/alfilcactus.obj");
	Model Carnivora((char*)"Models/caballocarni.obj");
	Model Nuez((char*)"Models/torrenuez.obj");

	// Modelos de prueba
	Model Dog((char*)"Models/RedDog.obj");
	Model Lavadora((char*)"Models/44-lavadora.obj");

	// First, set the container's VAO (and VBO)
	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Set texture units
	lightingShader.Use();
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.difuse"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.specular"), 1);

	// Bucle principal de la escena
	while (!glfwWindowShouldClose(window))
	{
		// Calcula el tiempo entre frames (delta time)
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Comprueba si se han activado eventos (tecla presionada, mouse movido, etc.) y llama a las funciones de respuesta correspondientes
		glfwPollEvents();
		DoMovement();

		// Limpia el colorbuffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Opción de profundidad para OpenGL
		glEnable(GL_DEPTH_TEST);

		// Utilice el sombreador correspondiente al configurar uniforms/drawing objects
		lightingShader.Use();

		glUniform1i(glGetUniformLocation(lightingShader.Program, "diffuse"), 0);
		//glUniform1i(glGetUniformLocation(lightingShader.Program, "specular"),1);	//Activación de la luz especular

		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

		// Directional light
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.5f, 0.5f, 0.5f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.0f, 0.0f, 0.0f);

		// Luz puntual (Dinámica)
		glm::vec3 lightColor;
		lightColor.x = abs(sin(glfwGetTime() * Light1.x));
		lightColor.y = abs(sin(glfwGetTime() * Light1.y));
		lightColor.z = sin(glfwGetTime() * Light1.z);

		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.0f);

		// Luces puntuales 2 a 4 (Estáticas)
		for (int i = 1; i <= 3; ++i) {
			SetPointLight(
				i,
				pointLightPositions[i],                       // posición
				glm::vec3(0.0f),                              // ambient
				glm::vec3(0.0f),                              // diffuse
				glm::vec3(1.0f, 1.0f, 1.0f),                  // specular
				1.0f, 0.0f, 0.0f,                             // attenuation: constant, linear, quadratic
				lightingShader.Program                        // shader ID
			);
		}

		// SpotLight
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.0f)));
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(12.0f)));

		// Set material properties
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 1.0f); //brillo

		// Create camera transformations
		glm::mat4 view;
		view = camera.GetViewMatrix();

		glm::mat4 projection;
		if (usePerspective) {
			projection = glm::perspective(glm::radians(camera.GetZoom()), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);
		}
		else {
			float orthoSize = 20.0f;
			projection = glm::ortho(
				-orthoSize * (float)SCREEN_WIDTH / SCREEN_HEIGHT,
				orthoSize * (float)SCREEN_WIDTH / SCREEN_HEIGHT,
				-orthoSize, orthoSize,
				0.1f, 100.0f
			);
		}

		// Get the uniform locations
		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glm::mat4 model(1);

		//Posicionamiento de la cámara
		view = camera.GetViewMatrix();

		// ========== CARGA DE MODELOS ==========
		
		//TABLERO
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.0f, -4.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 1.0f, 0.5f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Tablero.Draw(lightingShader);

		// ########## EQUIPO: Minecraft ##########

		DrawModel(Creeper, glm::vec3(-28.0f, 1.0f, -28.0f), 270.0f, glm::vec3(2.5f), modelLoc, lightingShader);					// TORRE 1
		DrawModel(Slime, glm::vec3(-20.0f, -2.0f, -28.0f), 270.0f, glm::vec3(1.0f), modelLoc, lightingShader);					// CABALLO 1
		DrawModel(Esqueleto, glm::vec3(-12.0f, 1.0f, -28.0f), 270.0f, glm::vec3(2.5f, 3.0f, 2.5f), modelLoc, lightingShader);	// ALFIL 1
		DrawModel(Steve, glm::vec3(-4.0f, -1.6f, -28.0f), 270.0f, glm::vec3(1.0f), modelLoc, lightingShader);					// REY
		DrawModel(Alex, glm::vec3(4.0f, -1.6f, -28.0f), 270.0f, glm::vec3(1.0f), modelLoc, lightingShader);						// REINA
		DrawModel(Esqueleto, glm::vec3(12.0f, 1.0f, -28.0f), 270.0f, glm::vec3(2.5f, 3.0f, 2.5f), modelLoc, lightingShader);	// ALFIL 2
		DrawModel(Slime, glm::vec3(20.0f, -2.0f, -28.0f), 270.0f, glm::vec3(1.0f), modelLoc, lightingShader);					// CABALLO 2
		DrawModel(Creeper, glm::vec3(28.0f, 1.0f, -28.0f), 270.0f, glm::vec3(2.5f), modelLoc, lightingShader);					// TORRE 2

		// PEONES Minecraft (Zombie)
		float peonZ_Mine = -20.0f;
		float peonY_Mine = -1.6f;
		glm::vec3 escalaPeonMine = glm::vec3(1.0f);
		float posicionesX_Mine[] = { -28, -20, -12, -4, 4, 12, 20, 28 };

		for (float x : posicionesX_Mine) {
			DrawModel(Zombie, glm::vec3(x, peonY_Mine, peonZ_Mine), 270.0f, escalaPeonMine, modelLoc, lightingShader);
		}


		// ########## EQUIPO: Plants vs Zombies ##########

		DrawModel(Nuez, glm::vec3(28.0f, -1.8f, 28.0f), 90.0f, glm::vec3(2.0f), modelLoc, lightingShader);          // TORRE 1
		DrawModel(Carnivora, glm::vec3(20.0f, -1.8f, 28.0f), 360.0f, glm::vec3(2.5f), modelLoc, lightingShader);    // CABALLO 1
		DrawModel(Cactus, glm::vec3(12.0f, -2.0f, 28.0f), 270.0f, glm::vec3(2.5f), modelLoc, lightingShader);       // ALFIL 1
		DrawModel(Girasol, glm::vec3(4.0f, -1.8f, 28.0f), 270.0f, glm::vec3(1.0f), modelLoc, lightingShader);       // REINA
		DrawModel(Fred, glm::vec3(-4.0f, -1.8f, 28.0f), 270.0f, glm::vec3(1.0f), modelLoc, lightingShader);         // REY
		DrawModel(Cactus, glm::vec3(-12.0f, -2.0f, 28.0f), 270.0f, glm::vec3(2.5f), modelLoc, lightingShader);      // ALFIL 2
		DrawModel(Carnivora, glm::vec3(-20.0f, -1.8f, 28.0f), 360.0f, glm::vec3(2.5f), modelLoc, lightingShader);   // CABALLO 2
		DrawModel(Nuez, glm::vec3(-28.0f, -1.8f, 28.0f), 45.0f, glm::vec3(2.0f), modelLoc, lightingShader);         // TORRE 2

		// PEONES
		float peonZ = 20.0f;
		float peonY = -1.5f;
		float rotPeon = 270.0f;
		glm::vec3 escalaPeon = glm::vec3(3.0f);
		float posicionesX[] = { 28, 20, 12, 4, -4, -12, -20, -28 };

		for (float x : posicionesX) {
			DrawModel(Lanzaguisantes, glm::vec3(x, peonY, peonZ), rotPeon, escalaPeon, modelLoc, lightingShader);
		}

		// --Modelo de prueba para probar el canal alfa
		//model = glm::mat4(1);
		//glEnable(GL_BLEND);//Avtiva la funcionalidad para trabajar el canal alfa //--Descomentar
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		//glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1.0); //--Descomentar
		//Dog.Draw(lightingShader);
		//glDisable(GL_BLEND);  //Desactiva el canal alfa //--Descomentar
		//glBindVertexArray(0); //Sepa la bola

		// Also draw the lamp object, again binding the appropriate shader
		lampShader.Use();

		// Get location objects for the matrices on the lamp shader (these could be different on a different shader)
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");

		// Set matrices
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		// Draw the light object (using light's vertex attributes)
		for (GLuint i = 0; i < 4; i++)
		{
			model = glm::mat4(1);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindVertexArray(0);

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();

	return 0;
}

// Moves/alters the camera positions based on user input
void DoMovement()
{

	// Camera controls
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP]){
		camera.ProcessKeyboard(FORWARD, deltaTime*2.0);
	}

	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN]){
		camera.ProcessKeyboard(BACKWARD, deltaTime*2.0);
	}

	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]){
		camera.ProcessKeyboard(LEFT, deltaTime*2.0);
	}

	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]){
		camera.ProcessKeyboard(RIGHT, deltaTime*2.0);
	}

	if (keys[GLFW_KEY_T]){
		pointLightPositions[0].x += 0.01f;
	}

	if (keys[GLFW_KEY_G]){
		pointLightPositions[0].x -= 0.01f;
	}

	if (keys[GLFW_KEY_Y]){
		pointLightPositions[0].y += 0.01f;
	}

	if (keys[GLFW_KEY_H]){
		pointLightPositions[0].y -= 0.01f;
	}
	
	if (keys[GLFW_KEY_U]){
		pointLightPositions[0].z -= 0.01f;
	}

	if (keys[GLFW_KEY_J]){
		pointLightPositions[0].z += 0.01f;
	}

}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action){
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024){
		if (action == GLFW_PRESS){
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE){
			keys[key] = false;
		}
	}

	if (keys[GLFW_KEY_SPACE]){
		active = !active;
		if (active){
			Light1 = glm::vec3(1.0f, 1.0f, 0.0f);
		}
		else{
			Light1 = glm::vec3(0);//Cuado es solo un valor en los 3 vectores pueden dejar solo una componente
		}
	}

	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		usePerspective = true;
	}

	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		usePerspective = false;
	}

}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (firstMouse){
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}