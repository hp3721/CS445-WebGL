#include "SpinningUAH.h"
#include "GlutShapes.h"
#include <emscripten.h>

#define STRING L"%s"

#define zPlane (-400)

double DXUTGetTime()
{
    return ((double)SDL_GetTicks())/1000.0;
}

glm::mat4* D3DXMatrixPerspectiveFovLH(glm::mat4 *pOut, float fovy, float Aspect, float zn, float zf)
{
    float fw, fh;
    fh = tanf( fovy / 2.0f) * zn;
    fw = fh * Aspect;
    *pOut = glm::frustum(-fw, +fw, -fh, +fh, zn, zf);

    return pOut;
}

enum Shape
{
    SOLID_SPHERE,
    SOLID_CUBE,
    WIRE_CUBE
};

static struct Scene
{
    float YShift = 0;
    float LightIntensity = 0.7F;
    float Angle = 0;
} scene;

static unsigned int U_displayList;
static unsigned int A_displayList;
static unsigned int H_displayList;

uint32_t keyPress = '\0';

extern "C" {
    EMSCRIPTEN_KEEPALIVE void increase_YShift() {
        scene.YShift += 25;
    }

    EMSCRIPTEN_KEEPALIVE void decrease_YShift() {
        scene.YShift -= 25;
    }
}

void draw_spindle()
{
    glNormal3f(0, 0, -1);
    glBegin(GL_TRIANGLES);

    glVertex3f(10, 0, zPlane);
    glVertex3f(10 + 25, 75, zPlane);
    glVertex3f(10 - 25, 75, zPlane);

    glEnd();
    glFlush();
}

void draw_glut_shape(Shape shape, float x, float y)
{
    glPushMatrix();
    glTranslatef(10 + 50 * x, -175 + 50 * y, zPlane);
    if (shape == SOLID_CUBE)
        glutSolidCube(50);
    else if (shape == WIRE_CUBE)
        glutWireCube(50);
    else if (shape == SOLID_SPHERE)
        glutSolidSphere(25.0F, 20, 20);
    glPopMatrix();
}

// Setup display lists for drawing static scene elements 
void setup_display_lists()
{
    U_displayList = glGenLists(1);
    glNewList(U_displayList, GL_COMPILE);

    draw_glut_shape(SOLID_CUBE, -7, 0);
    draw_glut_shape(SOLID_CUBE, -6, 0);
    draw_glut_shape(SOLID_CUBE, -5, 0);
    draw_glut_shape(SOLID_CUBE, -4, 0);
    draw_glut_shape(SOLID_CUBE, -7, 1);
    draw_glut_shape(SOLID_CUBE, -7, 2);
    draw_glut_shape(SOLID_CUBE, -7, 3);
    draw_glut_shape(SOLID_CUBE, -4, 1);
    draw_glut_shape(SOLID_CUBE, -4, 2);
    draw_glut_shape(SOLID_CUBE, -4, 3);

    glFlush();
    glEndList();

    A_displayList = glGenLists(1);
    glNewList(A_displayList, GL_COMPILE);

    draw_glut_shape(WIRE_CUBE, -2, 0);
    draw_glut_shape(WIRE_CUBE, 2, 0);
    draw_glut_shape(WIRE_CUBE, -1.5, 1);
    draw_glut_shape(WIRE_CUBE, -0.5, 1);
    draw_glut_shape(WIRE_CUBE, 0.5, 1);
    draw_glut_shape(WIRE_CUBE, 1.5, 1);
    draw_glut_shape(WIRE_CUBE, -1, 2);
    draw_glut_shape(WIRE_CUBE, 1, 2);
    draw_glut_shape(WIRE_CUBE, -0.5, 3);
    draw_glut_shape(WIRE_CUBE, 0.5, 3);

    glFlush();
    glEndList();

    H_displayList = glGenLists(1);

    glNewList(H_displayList, GL_COMPILE);

    draw_glut_shape(SOLID_CUBE, 4, 0);
    draw_glut_shape(SOLID_CUBE, 5, 1.5);
    draw_glut_shape(SOLID_CUBE, 6, 1.5);
    draw_glut_shape(SOLID_CUBE, 7, 0);
    draw_glut_shape(SOLID_CUBE, 4, 1);
    draw_glut_shape(SOLID_CUBE, 4, 2);
    draw_glut_shape(SOLID_CUBE, 4, 3);
    draw_glut_shape(SOLID_CUBE, 7, 1);
    draw_glut_shape(SOLID_CUBE, 7, 2);
    draw_glut_shape(SOLID_CUBE, 7, 3);

    glFlush();
    glEndList();
}

void lighting_setup()
{
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	// Light properties
	float lightAmb[] = { 0.2, 0.2, 0.2, 1.0 };
	float lightDifAndSpec[] = { 0.7, 0.7, 0.7, 1.0 };
	float lightPos[] = { 0.0, 0.0, 1.0, 0.0 };
	float globAmb[] = { 0.2, 0.2, 0.2, 1.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDifAndSpec);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightDifAndSpec);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	glEnable(GL_LIGHT0);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globAmb); // Global ambient light.

	// Material properties
	float matSpec[] = { 0.7, 0.7, 0.7, 1.0 };
	float matShine[] = { 25.0 };

	// Material properties.
	glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);
	glMaterialfv(GL_FRONT, GL_SHININESS, matShine);

	// Cull back faces.
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glShadeModel(GL_SMOOTH);
}

bool process_events()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_KEYDOWN)
        {
            keyPress = event.key.keysym.sym;
            switch (keyPress)
            {
            case SDLK_u:
                scene.YShift += 25;
                break;
            case SDLK_n:
                scene.YShift -= 25;
                break;
            case SDLK_k:
                if (scene.LightIntensity < 1.0)
                    scene.LightIntensity += 0.1;
                break;
            case SDLK_l:
                if (scene.LightIntensity > 0.0)
                    scene.LightIntensity -= 0.1;
                break;
            case SDLK_ESCAPE:
                return false;
            default:
                break;
            }
        }
    }
    return true;
}

SDL_Window* window = NULL;

extern "C"
void initialize_gl4es();
double fLastTime = 0.0;
static float lastFrame = 0.0f;

void drawScene()
{
    // Create black background
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float lightDifAndSpec[] = { scene.LightIntensity, scene.LightIntensity, scene.LightIntensity, 1.0 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDifAndSpec);

    glPushMatrix();
    glTranslatef(0, scene.YShift, 0);

    // Draw spindle
    // Dull Gray
    float colorDullGray[] = { 0.6, 0.6, 0.6, 1.0 };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorDullGray);
    draw_spindle();

    glPushMatrix();

    glTranslatef(10, 0, -400);
    glRotatef(scene.Angle, 0, 1, 0);
    glTranslatef(-10, 0, 400);

    // Draw letters from display lists
    // #0077C8 UAH Blue
    float colorUAH[] = { 0.0, 0.467, 0.784, 1.0 };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorUAH);
    glCallList(U_displayList);
    glCallList(H_displayList);

    // #8C8C8F Medium Gray
    float colorMediumGray[] = { 0.549, 0.549, 0.561, 1.0 };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorMediumGray);
    glCallList(A_displayList);

    // #FF0800 Candy Apple Red
    float colorRed[] = { 1.0, 0.031, 0.0, 1.0 };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorRed);
    draw_glut_shape(SOLID_SPHERE, 0, 2);

    glPopMatrix();
    glPopMatrix();

    glFlush();
}

void em_main_loop() {
    static const float targetFrameTime = 0.022f;
    static float accumulatedTime = 0.0f;

    double currentTime = DXUTGetTime();
    float elapsedTime = currentTime - fLastTime;
    fLastTime = currentTime;

    accumulatedTime += elapsedTime;

    if (accumulatedTime >= targetFrameTime) {
        scene.Angle -= 4.0f;

        process_events();
        drawScene();
        SDL_GL_SwapWindow(window);

        accumulatedTime -= targetFrameTime;
    }

    float remainingTime = targetFrameTime - (DXUTGetTime() - currentTime);
    if (remainingTime > 0) {
        SDL_Delay(static_cast<Uint32>(remainingTime * 1000));
    }
}

int main(int argc, const char** argv)
{
    initialize_gl4es();

    char mainTitle[50] = {0};
    sprintf(mainTitle, "University Lights");
    printf("%s\n", mainTitle);

    SDL_GLContext context = NULL;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == -1)
    {
        printf("Could not initialise SDL2: %s\n", SDL_GetError());
        exit(-1);
    }

    atexit(SDL_Quit);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 3);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 3);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 3);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    int flags = 0;
    int screenH, screenW, screenX, screenY;

    flags = SDL_WINDOW_OPENGL;

    screenW = 800;
    screenH = 800;

    window = SDL_CreateWindow(mainTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenW, screenH, flags);
    if (window == NULL)
    {
        printf("Couldn't create Window (%dx%d): %s\n", screenW, screenH, SDL_GetError());
        exit(-2);
    }
    context = SDL_GL_CreateContext(window);
    if (context == NULL)
    {
        printf("Couldn't create OpenGL Context: %s\n", SDL_GetError());
        exit(-3);
    }
    SDL_GetWindowSize(window, &screenW, &screenH);
    SDL_SetWindowTitle(window, mainTitle);

    // automatic guess the scale
    float screenScale = 1.;
    if (screenW / 800. < screenH / 800.)
        screenScale = screenW / 800.;
    else
        screenScale = screenH / 800.;

    screenX = (screenW - 800. * screenScale) / 2.;
    screenY = (screenH - 800. * screenScale) / 2.;
    screenW = 800 * screenScale;
    screenH = 800 * screenScale;

    if (flags & SDL_WINDOW_FULLSCREEN || flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
        SDL_ShowCursor(SDL_DISABLE);

    glViewport(screenX, screenY, screenW, screenH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glm::mat4 matProj;
    D3DXMatrixPerspectiveFovLH(&matProj, 90.0, 1.0, 10.0, 1010.0);
    // matProj = scale(matProj, glm::vec3(1.0f, -1.0f, 1.0f));
    glLoadMatrixf(value_ptr(matProj));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    lighting_setup();
    // Setups display list for drawing static scene features
    setup_display_lists();

    glClearColor(0, 0, 0, 0);

    fLastTime = DXUTGetTime();
    emscripten_set_main_loop(em_main_loop, 0, 1);

    SDL_Quit();

    exit(0);
}
