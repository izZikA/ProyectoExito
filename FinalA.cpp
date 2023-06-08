#include <Windows.h>

#include <glad/glad.h>
#include <glfw3.h>	//main
#include <stdlib.h>
#include <stdio.h>
#include <glm/glm.hpp>	//camera y model
#include <glm/gtc/matrix_transform.hpp>	//camera y model
#include <glm/gtc/type_ptr.hpp>
#include <time.h>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>	//Texture

#define SDL_MAIN_HANDLED
#include <SDL/SDL.h>

#include <shader_m.h>
#include <camera.h>
#include <modelAnim.h>
#include <model.h>
#include <Skybox.h>
#include <iostream>


// --------------- REPRODUCCION MUSICAL --------------------------

// Pragma para reproducir audio .wav o mp3
#pragma comment(lib, "winmm.lib")
bool music = true, // Variable para activar musica
current_song = false; // Cancion, se vuelve True si se reproduce bien

// Funcion para reproducir musica
void play_music() {
	if (music) {
		current_song = PlaySound(L"zoo.wav", NULL, SND_LOOP | SND_ASYNC);
		music = false; // Debe ser activada de nuevo para permitir reproduccion
	}
} // play_music()

// --------------- FIN PARTE MUSICAL ------------------------------


//#pragma comment(lib, "winmm.lib")

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//void my_input(GLFWwindow *window);
void my_input(GLFWwindow* window, int key, int scancode, int action, int mods);
void animate(void);

// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
GLFWmonitor* monitors;

void getResolution(void);

// camera
Camera camera(glm::vec3(0.0f, 10.0f, 90.0f));
float MovementSpeed = 0.1f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
const int FPS = 60;
const int LOOP_TIME = 1000 / FPS; // = 16 milisec // 1000 millisec == 1 sec
double	deltaTime = 0.0f,
lastFrame = 0.0f;

//Lighting
glm::vec3 solPosition(0.0f, 4.0f, -10.0f);
//Dirección en la que llegan la fuente de luz con esa dirección tenemos mi escenario done llega de arriba hacia abajo esto puede cambiarse
//depende mucho de en que dirección queremos la iluminación
glm::vec3 lightDirection(-1.0f, -1.0f, -1.0f);
glm::vec3 myPosition(0.0f, 0.0f, 0.0f);


//Keyframes (Manipulación y dibujo)
float	posX = 0.0f,
posY = 0.0f,
posZ = 0.0f,
rotRodIzq = 0.0f,
MovAlaIzq = 0.0f,
MovAlaDer = 0.0f,
GiroAve = 0.0f,
GiroAve2 = 0.0f;
float	incX = 0.0f,
incY = 0.0f,
incZ = 0.0f,
rotInc = 0.0f,
MovAlaIzqInc = 0.0f,
MovAlaDerInc = 0.0f,
GiroAveInc = 0.0f,
GiroAve2Inc = 0.0f;


#define MAX_FRAMES 25
int i_max_steps = 60;
int i_curr_steps = 0;
typedef struct _frame
{
	//Variables para GUARDAR Key Frames
	float posX;		//Variable para PosicionX
	float posY;		//Variable para PosicionY
	float posZ;		//Variable para PosicionZ
	float 	MovAlaIzq,
		MovAlaDer,
		GiroAve,
		GiroAve2;
}FRAMEAVE;

FRAMEAVE KeyFrame[MAX_FRAMES];
int FrameIndexAve = 0;			//introducir número en caso de tener Key guardados
bool play = false;
int playIndexAve = 0;

void saveFrame(void)
{
	//printf("frameindex %d\n", FrameIndex);
	std::cout << "Frame Index = " << FrameIndexAve << std::endl;

	KeyFrame[FrameIndexAve].posX = posX;
	KeyFrame[FrameIndexAve].posY = posY;
	KeyFrame[FrameIndexAve].posZ = posZ;

	KeyFrame[FrameIndexAve].MovAlaDer = MovAlaDer;
	KeyFrame[FrameIndexAve].MovAlaIzq = MovAlaIzq;
	KeyFrame[FrameIndexAve].GiroAve = GiroAve;
	KeyFrame[FrameIndexAve].GiroAve2 = GiroAve2;

	FrameIndexAve++;
}

void resetElements(void)
{
	posX = KeyFrame[0].posX;
	posY = KeyFrame[0].posY;
	posZ = KeyFrame[0].posZ;

	MovAlaDer = KeyFrame[0].MovAlaDer;
	MovAlaIzq = KeyFrame[0].MovAlaIzq;
	GiroAve = KeyFrame[0].GiroAve;
	GiroAve2 = KeyFrame[0].GiroAve2;
}

void interpolation(void)
{
	incX = (KeyFrame[playIndexAve + 1].posX - KeyFrame[playIndexAve].posX) / i_max_steps;
	incY = (KeyFrame[playIndexAve + 1].posY - KeyFrame[playIndexAve].posY) / i_max_steps;
	incZ = (KeyFrame[playIndexAve + 1].posZ - KeyFrame[playIndexAve].posZ) / i_max_steps;

	GiroAveInc = (KeyFrame[playIndexAve + 1].GiroAve - KeyFrame[playIndexAve].GiroAve) / i_max_steps;
	GiroAve2Inc = (KeyFrame[playIndexAve + 1].GiroAve2 - KeyFrame[playIndexAve].GiroAve2) / i_max_steps;
	MovAlaDerInc = (KeyFrame[playIndexAve + 1].MovAlaDer - KeyFrame[playIndexAve].MovAlaDer) / i_max_steps;
	MovAlaIzqInc = (KeyFrame[playIndexAve + 1].MovAlaIzq - KeyFrame[playIndexAve].MovAlaIzq) / i_max_steps;
}

void animate(void)
{

	if (play)
	{
		if (i_curr_steps >= i_max_steps) //end of animation between frames?
		{
			playIndexAve++;
			if (playIndexAve > FrameIndexAve - 2)	//end of total animation?
			{
				std::cout << "Animation ended" << std::endl;
				//printf("termina anim\n");
				playIndexAve = 0;
				play = false;
			}
			else //Next frame interpolations
			{
				i_curr_steps = 0; //Reset counter
								  //Interpolation
				interpolation();
			}
		}
		else
		{
			//Draw animation
			posX += incX;
			posY += incY;
			posZ += incZ;
			MovAlaIzq += MovAlaIzqInc;
			MovAlaDer += MovAlaDerInc;
			GiroAve += GiroAveInc;
			GiroAve2 += GiroAve2Inc;
			i_curr_steps++;
		}
	}

}

void getResolution()
{
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	SCR_WIDTH = mode->width;
	SCR_HEIGHT = (mode->height) - 80;
}


int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	/*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	// --------------------
	monitors = glfwGetPrimaryMonitor();
	getResolution();

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CGeIHC", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetWindowPos(window, 0, 30);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, my_input);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader staticShader("Shaders/shader_Lights.vs", "Shaders/shader_Lights_mod.fs");
	Shader skyboxShader("Shaders/skybox.vs", "Shaders/skybox.fs");
	Shader animShader("Shaders/anim.vs", "Shaders/anim.fs");

	vector<std::string> faces
	{
		"resources/skybox/right.jpg",
		"resources/skybox/left.jpg",
		"resources/skybox/top.jpg",
		"resources/skybox/bottom.jpg",
		"resources/skybox/front.jpg",
		"resources/skybox/back.jpg"
	};

	Skybox skybox = Skybox(faces);

	// Shader configuration
	// --------------------
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	// load models
	// -----------

	
	
	Model Circuito("resources/objects/Circui/Circui.obj");
	Model Parque("resources/objects/Parque/Parque.obj");
	Model Balle("resources/objects/Balle/Balle.obj");
	Model Estacion("resources/objects/Estacionamiento/Estacionamiento.obj");
	Model Edi("resources/objects/Edificios/Edi.obj");
	Model Plano("resources/objects/Plano/Plano.obj");
	Model Contenido("resources/objects/Contenido/Contenido.obj");
	Model Habits("resources/objects/Habits/Habits.obj");
	Model Barda("resources/objects/Barda/Barda.obj");
	Model Area3("resources/objects/Area3Lite/area3Lite.obj");
	Model Area2("resources/objects/Area2Lite/area2Lite.obj");
	Model Area1("resources/objects/Area1Lite/area1Lite.obj");
	Model CuerpoFalcon("resources/objects/CuerpoFalcon/CuerpoFalcon.obj");
	Model AlaDer("resources/objects/AlaDer/AlaDer.obj");
	Model AlaIzq("resources/objects/AlaIzq/AlaIzq.obj");
	Model Vegetacion("resources/objects/Vegetacion/Vegetacion.obj");
	Model Maqui1("resources/objects/Maqui1/Maqui1.obj");
	ModelAnim Tiburon("resources/objects/Shark/Tiburon.fbx");
	Model Trans("resources/objects/ObjTransp/objTrans.obj");
	Model Ventanas("resources/objects/Ventanas/Ventanas.obj");
	// asignar un identificador para diferenciarlo de todo lo demas que pueda tener
	Tiburon.initShaders(animShader.ID);

	ModelAnim Oso("resources/objects/Oso/Oso.fbx");
	// asignar un identificador para diferenciarlo de todo lo demas que pueda tener
	Oso.initShaders(animShader.ID);

	ModelAnim pingui("resources/objects/pingui/pingui.fbx");
	// asignar un identificador para diferenciarlo de todo lo demas que pueda tener
	pingui.initShaders(animShader.ID);

	ModelAnim PezB("resources/objects/NF2/NF2.dae");
	PezB.initShaders(animShader.ID);

	/*ModelAnim animacionPersonaje("resources/objects/Personaje1/PersonajeBrazo.dae");
	animacionPersonaje.initShaders(animShader.ID);


	ModelAnim ninja("resources/objects/ZombieWalk/ZombieWalk.dae");
	ninja.initShaders(animShader.ID);*/

	//Inicialización de KeyFrames
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		KeyFrame[i].posX = 0;
		KeyFrame[i].posY = 0;
		KeyFrame[i].posZ = 0;
		KeyFrame[i].MovAlaDer = 0;
		KeyFrame[i].MovAlaIzq = 0;
		KeyFrame[i].GiroAve = 0;
		KeyFrame[i].GiroAve2 = 0;
	}

	FrameIndexAve = 18;
	KeyFrame[0].posX = 0.000000;
	KeyFrame[0].posY = 0.000000;
	KeyFrame[0].posZ = 0.000000;
	KeyFrame[0].MovAlaDer = -15.000000;
	KeyFrame[0].MovAlaIzq = 15.000000;
	KeyFrame[0].GiroAve = 0.000000;
	KeyFrame[0].GiroAve2 = 0.000000;
	KeyFrame[1].posX = 0.000000;
	KeyFrame[1].posY = 0.000000;
	KeyFrame[1].posZ = 0.000000;
	KeyFrame[1].MovAlaDer = 6.000000;
	KeyFrame[1].MovAlaIzq = -6.000000;
	KeyFrame[1].GiroAve = 0.000000;
	KeyFrame[1].GiroAve2 = 0.000000;
	KeyFrame[2].posX = 0.000000;
	KeyFrame[2].posY = 5.000000;
	KeyFrame[2].posZ = 0.000000;
	KeyFrame[2].MovAlaDer = -11.999995;
	KeyFrame[2].MovAlaIzq = 11.999995;
	KeyFrame[2].GiroAve = 0.000000;
	KeyFrame[2].GiroAve2 = 0.000000;
	KeyFrame[3].posX = 0.000000;
	KeyFrame[3].posY = 5.000000;
	KeyFrame[3].posZ = 14.000000;
	KeyFrame[3].MovAlaDer = 6.000005;
	KeyFrame[3].MovAlaIzq = -6.000005;
	KeyFrame[3].GiroAve = 0.000000;
	KeyFrame[3].GiroAve2 = 0.000000;
	KeyFrame[4].posX = 5.000000;
	KeyFrame[4].posY = 5.000000;
	KeyFrame[4].posZ = 26.000004;
	KeyFrame[4].MovAlaDer = -8.999985;
	KeyFrame[4].MovAlaIzq = 11.999985;
	KeyFrame[4].GiroAve = 36.000000;
	KeyFrame[4].GiroAve2 = -30.000000;
	KeyFrame[5].posX = 14.000000;
	KeyFrame[5].posY = 5.000000;
	KeyFrame[5].posZ = 34.000038;
	KeyFrame[5].MovAlaDer = 6.000022;
	KeyFrame[5].MovAlaIzq = -6.000024;
	KeyFrame[5].GiroAve = 87.000000;
	KeyFrame[5].GiroAve2 = -30.000000;
	KeyFrame[6].posX = 27.000000;
	KeyFrame[6].posY = 5.000000;
	KeyFrame[6].posZ = 34.000038;
	KeyFrame[6].MovAlaDer = -11.999978;
	KeyFrame[6].MovAlaIzq = 14.999976;
	KeyFrame[6].GiroAve = 129.000000;
	KeyFrame[6].GiroAve2 = -30.000000;
	KeyFrame[7].posX = 31.000008;
	KeyFrame[7].posY = 5.000000;
	KeyFrame[7].posZ = 26.000061;
	KeyFrame[7].MovAlaDer = -20.999981;
	KeyFrame[7].MovAlaIzq = 23.999969;
	KeyFrame[7].GiroAve = 164.999741;
	KeyFrame[7].GiroAve2 = -30.000000;
	KeyFrame[8].posX = 31.000008;
	KeyFrame[8].posY = 5.000000;
	KeyFrame[8].posZ = 16.000061;
	KeyFrame[8].MovAlaDer = 12.000019;
	KeyFrame[8].MovAlaIzq = -9.000031;
	KeyFrame[8].GiroAve = 167.999741;
	KeyFrame[8].GiroAve2 = -30.000000;
	KeyFrame[9].posX = 31.000008;
	KeyFrame[9].posY = 5.000000;
	KeyFrame[9].posZ = 2.000061;
	KeyFrame[9].MovAlaDer = -11.999981;
	KeyFrame[9].MovAlaIzq = 14.999969;
	KeyFrame[9].GiroAve = 245.999741;
	KeyFrame[9].GiroAve2 = 33.000000;
	KeyFrame[10].posX = 23.000061;
	KeyFrame[10].posY = 5.000000;
	KeyFrame[10].posZ = -3.999870;
	KeyFrame[10].MovAlaDer = -17.999954;
	KeyFrame[10].MovAlaIzq = 23.999950;
	KeyFrame[10].GiroAve = 245.999557;
	KeyFrame[10].GiroAve2 = -39.000019;
	KeyFrame[11].posX = 15.000069;
	KeyFrame[11].posY = 5.000000;
	KeyFrame[11].posZ = 7.000198;
	KeyFrame[11].MovAlaDer = 9.000061;
	KeyFrame[11].MovAlaIzq = -3.000072;
	KeyFrame[11].GiroAve = 296.999573;
	KeyFrame[11].GiroAve2 = -6.000057;
	KeyFrame[12].posX = 20.000069;
	KeyFrame[12].posY = 5.000000;
	KeyFrame[12].posZ = 24.000198;
	KeyFrame[12].MovAlaDer = -8.999939;
	KeyFrame[12].MovAlaIzq = 14.999928;
	KeyFrame[12].GiroAve = 371.999573;
	KeyFrame[12].GiroAve2 = -6.000057;
	KeyFrame[13].posX = 20.000069;
	KeyFrame[13].posY = -1.000000;
	KeyFrame[13].posZ = 24.000198;
	KeyFrame[13].MovAlaDer = 6.000061;
	KeyFrame[13].MovAlaIzq = -0.000072;
	KeyFrame[13].GiroAve = 371.999573;
	KeyFrame[13].GiroAve2 = -6.000057;
	KeyFrame[14].posX = 20.000103;
	KeyFrame[14].posY = -0.999998;
	KeyFrame[14].posZ = 24.000259;
	KeyFrame[14].MovAlaDer = -14.999935;
	KeyFrame[14].MovAlaIzq = 20.999899;
	KeyFrame[14].GiroAve = 305.999908;
	KeyFrame[14].GiroAve2 = -6.000103;
	KeyFrame[15].posX = 20.000103;
	KeyFrame[15].posY = -0.999998;
	KeyFrame[15].posZ = 24.000259;
	KeyFrame[15].MovAlaDer = -2.999935;
	KeyFrame[15].MovAlaIzq = 2.999899;
	KeyFrame[15].GiroAve = 227.999908;
	KeyFrame[15].GiroAve2 = -6.000103;
	KeyFrame[16].posX = 20.000103;
	KeyFrame[16].posY = -0.999994;
	KeyFrame[16].posZ = 24.000374;
	KeyFrame[16].MovAlaDer = -14.999949;
	KeyFrame[16].MovAlaIzq = 14.999885;
	KeyFrame[16].GiroAve = 227.999939;
	KeyFrame[16].GiroAve2 = -0.000160;
	KeyFrame[17].posX = 20.000103;
	KeyFrame[17].posY = -0.999990;
	KeyFrame[17].posZ = 24.000488;
	KeyFrame[17].MovAlaDer = 0.000050;
	KeyFrame[17].MovAlaIzq = -3.000126;
	KeyFrame[17].GiroAve = 227.999939;
	KeyFrame[17].GiroAve2 = -0.000221;

	// Asignacion de KeyFrames

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		skyboxShader.setInt("skybox", 0);

		// per-frame time logic
		// --------------------
		lastFrame = SDL_GetTicks();

		// input
		// -----
		//my_input(window);
		animate();

		// render
		// ------
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		staticShader.use();
		//Setup Advanced Lights
		staticShader.setVec3("viewPos", camera.Position);
		//Dirección en  la cual llegan los rayos luminosos a mi imagen
		staticShader.setVec3("dirLight.direction", lightDirection);
		//Asociadas a una fuente de luz si ponemos en 0,0,0 no va a generar luz 0: ausencia de color, 1: maxima intensidad
		//Luz que existe en el ambiente, asociadas a las caras menos iluminadas 
		staticShader.setVec3("dirLight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		//Fuentes de luz en el escenario: lamparas, focos, etc. agregar una diferencia a la intencidad luminosa.
		staticShader.setVec3("dirLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		// Efecto de brillo sobre una superfice 
		staticShader.setVec3("dirLight.specular", glm::vec3(0.0f, 0.0f, 0.0f));

		/*--------- Fuentes de luz posicionales o puntuales ---------------*/
		//Tipo de lus puntual (tiene atenuación lineal): si un objeto esta cerca de la fuente de luz se va a ver más iluminado.
		//Posición de mi luz

		/*--------------------    Luz inciso A   --------------------*/

		staticShader.setVec3("pointLight[0].position", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[0].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[0].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[0].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		//Tipo de atenuación
		// potencia de los rayos luminosos (la intensidad)
		staticShader.setFloat("pointLight[0].constant", 1.0f);
		//Lo mismo que la cuadratica pero más fino
		staticShader.setFloat("pointLight[0].linear", 0.0009f);
		//Me permite hacer que los rayos luminosos viajen una mayor distancia en mi escenario o viseversa entre más pequeño sea el valor mas van a viajar los rayos luminosos
		staticShader.setFloat("pointLight[0].quadratic", 0.0000032f);

		/*------------------   Luz inciso B ----------------------------*/

		staticShader.setVec3("pointLight[1].position", solPosition);
		staticShader.setVec3("pointLight[1].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[1].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[1].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setFloat("pointLight[1].constant", 0.3f);
		staticShader.setFloat("pointLight[1].linear", 0.0008f);
		staticShader.setFloat("pointLight[1].quadratic", 0.0000032f);

		/*-------------------       Luz inciso C (Echo en clase) */
		staticShader.setVec3("pointLight[2].position", glm::vec3(-80.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[2].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[2].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[2].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setFloat("pointLight[2].constant", 1.0f);
		staticShader.setFloat("pointLight[2].linear", 0.0009f);
		staticShader.setFloat("pointLight[2].quadratic", 0.0032f);

		/*---------  Luz reflector -------*/
		//mover nuestra lampara
		staticShader.setVec3("spotLight[0].position", glm::vec3(camera.Position.x, camera.Position.y, camera.Position.z));
		//hacia donde va a apuntar nuestra lamapara
		staticShader.setVec3("spotLight[0].direction", glm::vec3(camera.Front.x, camera.Front.y, camera.Front.z));
		staticShader.setVec3("spotLight[0].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("spotLight[0].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("spotLight[0].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		//Angulo de apertura  es el diametro de nuestra luz
		staticShader.setFloat("spotLight[0].cutOff", glm::cos(glm::radians(10.0f)));
		staticShader.setFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(20.0f)));
		staticShader.setFloat("spotLight[0].constant", 0.1f);
		staticShader.setFloat("spotLight[0].linear", 0.0009f);
		staticShader.setFloat("spotLight[0].quadratic", 0.0005f);
		staticShader.setFloat("material_shininess", 32.0f);

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 modelA = glm::mat4(1.0f);
		glm::mat4 tmp = glm::mat4(1.0f);
		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		staticShader.setMat4("projection", projection);
		staticShader.setMat4("view", view);

		//// Light
		glm::vec3 lightColor = glm::vec3(0.6f);
		glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.75f);


		// -------------------------------------------------------------------------------------------------------------------------
		// Personaje Animacion
		// -------------------------------------------------------------------------------------------------------------------------
		//Remember to activate the shader with the animation
		animShader.use();
		animShader.setMat4("projection", projection);
		animShader.setMat4("view", view);

		animShader.setVec3("material.specular", glm::vec3(0.8f));
		animShader.setFloat("material.shininess", 32.0f);
		animShader.setVec3("light.ambient", ambientColor);
		animShader.setVec3("light.diffuse", diffuseColor);
		animShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		animShader.setVec3("light.direction", lightDirection);
		animShader.setVec3("viewPos", camera.Position);



		// -------------------------------------------------------------------------------------------------------------------------
		// Personaje Animacion
		// -------------------------------------------------------------------------------------------------------------------------



		modelA = glm::translate(glm::mat4(1.0f), glm::vec3(250.0f, -5.0f, -160.0f));
		modelA = glm::scale(modelA, glm::vec3(0.001f));
		animShader.setMat4("model", modelA);
		Tiburon.Draw(animShader);


		modelA = glm::translate(glm::mat4(1.0f), glm::vec3(-240.0f, -3.0f, -141.0f));
		modelA = glm::scale(modelA, glm::vec3(0.001f));
		animShader.setMat4("model", modelA);
		pingui.Draw(animShader);

		model = glm::mat4(1.0f);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(220.0f, -5.0f, -145.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.01f));	// it's a bit too big for our scene, so scale it down
		animShader.setMat4("model", model);
		PezB.Draw(animShader);

		model = glm::mat4(1.0f);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(220.0f, -6.0f, -145.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.01f));	// it's a bit too big for our scene, so scale it down
		animShader.setMat4("model", model);
		PezB.Draw(animShader);

		model = glm::mat4(1.0f);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(220.0f, -6.0f, -147.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.01f));	// it's a bit too big for our scene, so scale it down
		animShader.setMat4("model", model);
		PezB.Draw(animShader);

		modelA = glm::translate(glm::mat4(1.0f), glm::vec3(-30.0f, 1.0f, -170.0f));
		modelA = glm::scale(modelA, glm::vec3(0.0035f));
		animShader.setMat4("model", modelA);
		Oso.Draw(animShader);


		// -------------------------------------------------------------------------------------------------------------------------
		// Escenario
		// -------------------------------------------------------------------------------------------------------------------------
		staticShader.use();
		staticShader.setMat4("projection", projection);
		staticShader.setMat4("view", view);





		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.01f));
		staticShader.setMat4("model", model);
		//Area3.Draw(staticShader);

		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.01f));
		staticShader.setMat4("model", model);
		Circuito.Draw(staticShader);

		model = glm::mat4(1.0f);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f));
		model = glm::scale(model, glm::vec3(0.01f));
		staticShader.setMat4("model", model);
		Estacion.Draw(staticShader);


		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.01f));
		staticShader.setMat4("model", model);
		Edi.Draw(staticShader);



		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.01f));
		staticShader.setMat4("model", model);
		Maqui1.Draw(staticShader);



		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.01f));
		staticShader.setMat4("model", model);
		Habits.Draw(staticShader);

		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.01f));
		staticShader.setMat4("model", model);
		Contenido.Draw(staticShader);

		model = glm::mat4(1.0f);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -20.0f, -10.0f));
		model = glm::scale(model, glm::vec3(0.01f));
		staticShader.setMat4("model", model);
		Balle.Draw(staticShader);

		

		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.01f));
		staticShader.setMat4("model", model);
		Area2.Draw(staticShader);

		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.01f));
		staticShader.setMat4("model", model);
		Area3.Draw(staticShader);

		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.01f));
		staticShader.setMat4("model", model);
		Barda.Draw(staticShader);


		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.01f));
		staticShader.setMat4("model", model);
		Vegetacion.Draw(staticShader);

		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.01f));
		staticShader.setMat4("model", model);
		Parque.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(22.0f, 12.0f, -143.0f));
		model = glm::translate(model, glm::vec3(posX, posY, posZ));
		model = glm::scale(model, glm::vec3(0.08f));
		model = glm::rotate(model, glm::radians(GiroAve), glm::vec3(0.0f, 1.0f, 0.0));
		tmp = model = glm::rotate(model, glm::radians(GiroAve2), glm::vec3(0.0f, 0.0f, 1.0f));
		staticShader.setMat4("model", model);
		CuerpoFalcon.Draw(staticShader);
		//Ala derecha Halcon
		model = glm::translate(tmp, glm::vec3(1.5f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(MovAlaDer), glm::vec3(0.0f, 0.0f, 1.0));
		staticShader.setMat4("model", model);
		AlaDer.Draw(staticShader);

		//Ala izquierda Halcon

		model = glm::translate(tmp, glm::vec3(-1.5f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(MovAlaIzq), glm::vec3(0.0f, 0.0f, 1.0));
		staticShader.setMat4("model", model);
		AlaIzq.Draw(staticShader);


		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.01f));
		staticShader.setMat4("model", model);
		Area1.Draw(staticShader);
		// -------------------------------------------------------------------------------------------------------------------------
		// Carro
		// -------------------------------------------------------------------------------------------------------------------------

		//llanta.Draw(staticShader);	//Der trasera

		//llanta.Draw(staticShader);	//Izq trase
		// -------------------------------------------------------------------------------------------------------------------------
		// Personaje
		// -------------------------------------------------------------------------------------------------------------------------
		
		// -------------------------------------------------------------------------------------------------------------------------
		// Caja Transparente --- Siguiente Práctica
		// -------------------------------------------------------------------------------------------------------------------------
		model = glm::mat4(1.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		model = glm::scale(model, glm::vec3(0.01f));
		staticShader.setMat4("model", model);
		Trans.Draw(staticShader);
		glEnable(GL_BLEND);

		model = glm::mat4(1.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		model = glm::scale(model, glm::vec3(0.01f));
		staticShader.setMat4("model", model);
		Ventanas.Draw(staticShader);
		glEnable(GL_BLEND);
		// -------------------------------------------------------------------------------------------------------------------------
		// Termina Escenario
		// -------------------------------------------------------------------------------------------------------------------------

		//-------------------------------------------------------------------------------------
		// draw skybox as last
		// -------------------
		skyboxShader.use();
		skybox.Draw(skyboxShader, view, projection, camera);

		// Limitar el framerate a 60
		deltaTime = SDL_GetTicks() - lastFrame; // time for full 1 loop
		//std::cout <<"frame time = " << frameTime << " milli sec"<< std::endl;
		if (deltaTime < LOOP_TIME)
		{
			SDL_Delay((int)(LOOP_TIME - deltaTime));
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	skybox.Terminate();

	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void my_input(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, (float)deltaTime);
	// Reproduccion Musical
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		play_music();

	//To Configure Model
	//Car animation
	//To play KeyFrame animation 
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		if (play == false && (FrameIndexAve > 1))
		{
			std::cout << "Play animation" << std::endl;
			resetElements();
			//First Interpolation				
			interpolation();
			play = true;
			playIndexAve = 0;
			i_curr_steps = 0;
		}
		else
		{
			play = false;
			std::cout << "Not enough Key Frames" << std::endl;
		}
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