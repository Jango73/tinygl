#include "tinygl_demo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TINYGL_DEMO_NEAR_PLANE 1.0
#define TINYGL_DEMO_FAR_PLANE 64.0
#define TINYGL_DEMO_HALF_FRUSTUM_HEIGHT 0.75
#define TINYGL_DEMO_CAMERA_DISTANCE -6.0f
#define TINYGL_DEMO_ROTATION_X_SPEED 42.0f
#define TINYGL_DEMO_ROTATION_Y_SPEED 75.0f
#define TINYGL_DEMO_SMALL_CUBE_SCALE 0.55f
#define TINYGL_DEMO_SMALL_CUBE_OFFSET_X 0.0f
#define TINYGL_DEMO_SMALL_CUBE_OFFSET_Y 1.0f
#define TINYGL_DEMO_SMALL_CUBE_OFFSET_Z 0.0f

/************************************************************************/

static const GLfloat TINYGL_DEMO_LIGHT_POSITION[4] = {2.0f, 2.0f, 2.0f, 1.0f};
static const GLfloat TINYGL_DEMO_LIGHT_AMBIENT[4] = {0.10f, 0.10f, 0.10f, 1.0f};
static const GLfloat TINYGL_DEMO_LIGHT_DIFFUSE[4] = {0.95f, 0.95f, 0.95f, 1.0f};
static const GLfloat TINYGL_DEMO_LIGHT_SPECULAR[4] = {0.35f, 0.35f, 0.35f, 1.0f};
static const GLfloat TINYGL_DEMO_MATERIAL_AMBIENT[4] = {0.30f, 0.30f, 0.30f, 1.0f};
static const GLfloat TINYGL_DEMO_MATERIAL_DIFFUSE[4] = {0.80f, 0.80f, 0.80f, 1.0f};
static const GLfloat TINYGL_DEMO_MATERIAL_SPECULAR[4] = {0.25f, 0.25f, 0.25f, 1.0f};
static const GLfloat TINYGL_DEMO_MATERIAL_EMISSION[4] = {0.00f, 0.00f, 0.00f, 1.0f};
static const GLfloat TINYGL_DEMO_MATERIAL_SHININESS[1] = {12.0f};

/************************************************************************/

/**
 * @brief Print a TinyGL demo error.
 * @param Message The error message.
 * @return Always returns 0.
 */
static int printDemoError(const char *Message) {
    fprintf(stderr, "tinygl_demo: %s\n", Message);
    return 0;
}

/************************************************************************/

/**
 * @brief Validate a TinyGL control-path result.
 * @param Result The result to validate.
 * @param Message The error message.
 * @return 1 on success, 0 on failure.
 */
static int checkTinyGlResult(TGL_RESULT Result, const char *Message) {
    if (Result == TGL_RESULT_OK) {
        return 1;
    }

    return printDemoError(Message);
}

/************************************************************************/

/**
 * @brief Allocate or replace the host-owned color surface.
 * @param App The demo application state.
 * @param Width The requested width.
 * @param Height The requested height.
 * @return 1 on success, 0 on failure.
 */
static int allocateHostSurface(TINYGL_DEMO_APP *App, GLsizei Width,
                               GLsizei Height) {
    unsigned char *ColorBuffer;
    size_t BufferSize;
    TGL_SURFACE_DESC Surface;

    if (Width <= 0 || Height <= 0) {
        return printDemoError("invalid surface size");
    }

    BufferSize = (size_t)Width * (size_t)Height * 4;
    ColorBuffer = (unsigned char *)malloc(BufferSize);
    if (ColorBuffer == NULL) {
        return printDemoError("unable to allocate color buffer");
    }

    memset(ColorBuffer, 0, BufferSize);

    Surface.Pixels = ColorBuffer;
    Surface.Width = Width;
    Surface.Height = Height;
    Surface.Pitch = Width * 4;
    Surface.PixelFormat = TGL_PIXEL_FORMAT_XRGB8888;

    if (!checkTinyGlResult(tinyglSetSurface(App->Context, &Surface),
                           "unable to attach host surface")) {
        free(ColorBuffer);
        return 0;
    }

    free(App->ColorBuffer);
    App->ColorBuffer = ColorBuffer;
    App->Surface = Surface;
    App->Width = Width;
    App->Height = Height;

    return 1;
}

/************************************************************************/

/**
 * @brief Configure the per-frame render state.
 * @param App The demo application state.
 */
static void configureFrameState(const TINYGL_DEMO_APP *App) {
    GLdouble AspectRatio;
    GLdouble HalfWidth;

    AspectRatio = (GLdouble)App->Width / (GLdouble)App->Height;
    HalfWidth = TINYGL_DEMO_HALF_FRUSTUM_HEIGHT * AspectRatio;

    glViewport(0, 0, App->Width, App->Height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-HalfWidth, HalfWidth, -TINYGL_DEMO_HALF_FRUSTUM_HEIGHT,
              TINYGL_DEMO_HALF_FRUSTUM_HEIGHT, TINYGL_DEMO_NEAR_PLANE,
              TINYGL_DEMO_FAR_PLANE);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, TINYGL_DEMO_CAMERA_DISTANCE);
}

/************************************************************************/

/**
 * @brief Configure the current demo lighting mode.
 * @param App The demo application state.
 */
static void configureLightingState(const TINYGL_DEMO_APP *App) {
    if (App->ViewMode == TINYGL_DEMO_VIEW_MODE_LIT_CUBE) {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);

        glLightfv(GL_LIGHT0, GL_POSITION, TINYGL_DEMO_LIGHT_POSITION);
        glLightfv(GL_LIGHT0, GL_AMBIENT, TINYGL_DEMO_LIGHT_AMBIENT);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, TINYGL_DEMO_LIGHT_DIFFUSE);
        glLightfv(GL_LIGHT0, GL_SPECULAR, TINYGL_DEMO_LIGHT_SPECULAR);

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,
                     TINYGL_DEMO_MATERIAL_AMBIENT);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,
                     TINYGL_DEMO_MATERIAL_DIFFUSE);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,
                     TINYGL_DEMO_MATERIAL_SPECULAR);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,
                     TINYGL_DEMO_MATERIAL_EMISSION);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS,
                     TINYGL_DEMO_MATERIAL_SHININESS);
        return;
    }

    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);
}

/************************************************************************/

/**
 * @brief Apply the cube-local model transform after scene lighting setup.
 * @param App The demo application state.
 */
static void applyCubeTransform(const TINYGL_DEMO_APP *App) {
    glRotatef(App->RotationX, 1.0f, 0.0f, 0.0f);
    glRotatef(App->RotationY, 0.0f, 1.0f, 0.0f);
}

/************************************************************************/

/**
 * @brief Emit one colored triangle.
 * @param Red The red component.
 * @param Green The green component.
 * @param Blue The blue component.
 * @param X1 The first vertex X component.
 * @param Y1 The first vertex Y component.
 * @param Z1 The first vertex Z component.
 * @param X2 The second vertex X component.
 * @param Y2 The second vertex Y component.
 * @param Z2 The second vertex Z component.
 * @param X3 The third vertex X component.
 * @param Y3 The third vertex Y component.
 * @param Z3 The third vertex Z component.
 */
static void drawTriangle(GLfloat Red, GLfloat Green, GLfloat Blue, GLfloat X1,
                         GLfloat Y1, GLfloat Z1, GLfloat X2, GLfloat Y2,
                         GLfloat Z2, GLfloat X3, GLfloat Y3, GLfloat Z3) {
    glColor3f(Red, Green, Blue);
    glVertex3f(X1, Y1, Z1);
    glVertex3f(X2, Y2, Z2);
    glVertex3f(X3, Y3, Z3);
}

/************************************************************************/

/**
 * @brief Emit one cube face triangle with a shared normal.
 * @param NormalX The face normal X component.
 * @param NormalY The face normal Y component.
 * @param NormalZ The face normal Z component.
 * @param Red The red component.
 * @param Green The green component.
 * @param Blue The blue component.
 * @param X1 The first vertex X component.
 * @param Y1 The first vertex Y component.
 * @param Z1 The first vertex Z component.
 * @param X2 The second vertex X component.
 * @param Y2 The second vertex Y component.
 * @param Z2 The second vertex Z component.
 * @param X3 The third vertex X component.
 * @param Y3 The third vertex Y component.
 * @param Z3 The third vertex Z component.
 */
static void drawLitTriangle(GLfloat NormalX, GLfloat NormalY, GLfloat NormalZ,
                            GLfloat Red, GLfloat Green, GLfloat Blue,
                            GLfloat X1, GLfloat Y1, GLfloat Z1, GLfloat X2,
                            GLfloat Y2, GLfloat Z2, GLfloat X3, GLfloat Y3,
                            GLfloat Z3) {
    glNormal3f(NormalX, NormalY, NormalZ);
    drawTriangle(Red, Green, Blue, X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3);
}

/************************************************************************/

/**
 * @brief Draw a colored cube centered at the origin.
 * @param ViewMode The selected view mode.
 */
static void drawCube(TINYGL_DEMO_VIEW_MODE ViewMode) {
    glBegin(GL_TRIANGLES);

    if (ViewMode == TINYGL_DEMO_VIEW_MODE_LIT_CUBE) {
        drawLitTriangle(0.0f, 0.0f, 1.0f, 0.90f, 0.20f, 0.18f, -1.0f, -1.0f,
                        1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        drawLitTriangle(0.0f, 0.0f, 1.0f, 0.90f, 0.20f, 0.18f, -1.0f, -1.0f,
                        1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f);

        drawLitTriangle(0.0f, 0.0f, -1.0f, 0.18f, 0.50f, 0.92f, -1.0f, -1.0f,
                        -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f);
        drawLitTriangle(0.0f, 0.0f, -1.0f, 0.18f, 0.50f, 0.92f, -1.0f, -1.0f,
                        -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f);

        drawLitTriangle(-1.0f, 0.0f, 0.0f, 0.25f, 0.78f, 0.35f, -1.0f, -1.0f,
                        -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f);
        drawLitTriangle(-1.0f, 0.0f, 0.0f, 0.25f, 0.78f, 0.35f, -1.0f, -1.0f,
                        -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f);

        drawLitTriangle(1.0f, 0.0f, 0.0f, 0.94f, 0.80f, 0.24f, 1.0f, -1.0f,
                        -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f);
        drawLitTriangle(1.0f, 0.0f, 0.0f, 0.94f, 0.80f, 0.24f, 1.0f, -1.0f,
                        -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f);

        drawLitTriangle(0.0f, 1.0f, 0.0f, 0.84f, 0.36f, 0.88f, -1.0f, 1.0f,
                        -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        drawLitTriangle(0.0f, 1.0f, 0.0f, 0.84f, 0.36f, 0.88f, -1.0f, 1.0f,
                        -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f);

        drawLitTriangle(0.0f, -1.0f, 0.0f, 0.20f, 0.78f, 0.82f, -1.0f, -1.0f,
                        -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f);
        drawLitTriangle(0.0f, -1.0f, 0.0f, 0.20f, 0.78f, 0.82f, -1.0f, -1.0f,
                        -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    } else {
        drawTriangle(0.90f, 0.20f, 0.18f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
                     1.0f, 1.0f, 1.0f, 1.0f);
        drawTriangle(0.90f, 0.20f, 0.18f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
                     1.0f, -1.0f, 1.0f, 1.0f);

        drawTriangle(0.18f, 0.50f, 0.92f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,
                     -1.0f, 1.0f, 1.0f, -1.0f);
        drawTriangle(0.18f, 0.50f, 0.92f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
                     -1.0f, 1.0f, -1.0f, -1.0f);

        drawTriangle(0.25f, 0.78f, 0.35f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
                     1.0f, -1.0f, 1.0f, 1.0f);
        drawTriangle(0.25f, 0.78f, 0.35f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,
                     1.0f, -1.0f, 1.0f, -1.0f);

        drawTriangle(0.94f, 0.80f, 0.24f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
                     1.0f, 1.0f, -1.0f, 1.0f);
        drawTriangle(0.94f, 0.80f, 0.24f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
                     -1.0f, 1.0f, 1.0f, 1.0f);

        drawTriangle(0.84f, 0.36f, 0.88f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
                     1.0f, 1.0f, 1.0f, 1.0f);
        drawTriangle(0.84f, 0.36f, 0.88f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
                     1.0f, 1.0f, 1.0f, -1.0f);

        drawTriangle(0.20f, 0.78f, 0.82f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
                     1.0f, -1.0f, -1.0f, 1.0f);
        drawTriangle(0.20f, 0.78f, 0.82f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
                     -1.0f, 1.0f, -1.0f, 1.0f);
    }

    glEnd();
}

/************************************************************************/

/**
 * @brief Draw the demo cube pair used to validate depth buffering.
 * @param App The demo application state.
 */
static void drawDepthValidationScene(const TINYGL_DEMO_APP *App) {
    glPushMatrix();
    applyCubeTransform(App);
    drawCube(App->ViewMode);
    glPopMatrix();

    glPushMatrix();
    applyCubeTransform(App);
    glTranslatef(TINYGL_DEMO_SMALL_CUBE_OFFSET_X,
                 TINYGL_DEMO_SMALL_CUBE_OFFSET_Y,
                 TINYGL_DEMO_SMALL_CUBE_OFFSET_Z);
    glScalef(TINYGL_DEMO_SMALL_CUBE_SCALE, TINYGL_DEMO_SMALL_CUBE_SCALE,
             TINYGL_DEMO_SMALL_CUBE_SCALE);
    drawCube(App->ViewMode);
    glPopMatrix();
}

/************************************************************************/

/**
 * @brief Initialize the common TinyGL demo state.
 * @param App The demo application state.
 * @param Width The initial width.
 * @param Height The initial height.
 * @return 1 on success, 0 on failure.
 */
int tinyglDemoInitialize(TINYGL_DEMO_APP *App, GLsizei Width, GLsizei Height) {
    TGL_CONTEXT_DESC ContextDesc;

    if (App == NULL) {
        return printDemoError("demo state is null");
    }

    memset(App, 0, sizeof(*App));

    ContextDesc.MaxWidth = Width;
    ContextDesc.MaxHeight = Height;
    ContextDesc.HasDepthBuffer = GL_TRUE;
    ContextDesc.HasColorBuffer = GL_TRUE;

    if (!checkTinyGlResult(tinyglCreateContext(&ContextDesc, &App->Context),
                           "unable to create TinyGL context")) {
        return 0;
    }

    if (!checkTinyGlResult(tinyglMakeCurrent(App->Context),
                           "unable to make TinyGL context current")) {
        tinyglDestroyContext(App->Context);
        App->Context = NULL;
        return 0;
    }

    if (!allocateHostSurface(App, Width, Height)) {
        tinyglDestroyContext(App->Context);
        App->Context = NULL;
        return 0;
    }

    return 1;
}

/************************************************************************/

/**
 * @brief Release the common TinyGL demo state.
 * @param App The demo application state.
 */
void tinyglDemoShutdown(TINYGL_DEMO_APP *App) {
    if (App == NULL) {
        return;
    }

    free(App->ColorBuffer);
    App->ColorBuffer = NULL;

    if (App->Context != NULL) {
        tinyglDestroyContext(App->Context);
        App->Context = NULL;
    }

    memset(&App->Surface, 0, sizeof(App->Surface));
    App->Width = 0;
    App->Height = 0;
    App->RotationX = 0.0f;
    App->RotationY = 0.0f;
    App->ViewMode = TINYGL_DEMO_VIEW_MODE_UNLIT_CUBE;
}

/************************************************************************/

/**
 * @brief Resize the host-owned render surface.
 * @param App The demo application state.
 * @param Width The new width.
 * @param Height The new height.
 * @return 1 on success, 0 on failure.
 */
int tinyglDemoResize(TINYGL_DEMO_APP *App, GLsizei Width, GLsizei Height) {
    if (App == NULL || App->Context == NULL) {
        return printDemoError("demo is not initialized");
    }

    return allocateHostSurface(App, Width, Height);
}

/************************************************************************/

/**
 * @brief Render one animation frame.
 * @param App The demo application state.
 * @param DeltaSeconds The frame delta time in seconds.
 * @return 1 on success, 0 on failure.
 */
int tinyglDemoRenderFrame(TINYGL_DEMO_APP *App, GLfloat DeltaSeconds) {
    if (App == NULL || App->Context == NULL) {
        return printDemoError("demo is not initialized");
    }

    App->RotationX += DeltaSeconds * TINYGL_DEMO_ROTATION_X_SPEED;
    App->RotationY += DeltaSeconds * TINYGL_DEMO_ROTATION_Y_SPEED;

    configureFrameState(App);
    configureLightingState(App);
    drawDepthValidationScene(App);
    glFlush();

    if (glGetError() != GL_NO_ERROR) {
        return printDemoError("rendering failed");
    }

    return 1;
}

/************************************************************************/

/**
 * @brief Change the current demo view mode.
 * @param App The demo application state.
 * @param ViewMode The new view mode.
 */
void tinyglDemoSetViewMode(TINYGL_DEMO_APP *App,
                           TINYGL_DEMO_VIEW_MODE ViewMode) {
    if (App == NULL) {
        return;
    }

    switch (ViewMode) {
    case TINYGL_DEMO_VIEW_MODE_UNLIT_CUBE:
    case TINYGL_DEMO_VIEW_MODE_LIT_CUBE: {
        App->ViewMode = ViewMode;
    } break;

    default:
        break;
    }
}

/************************************************************************/

/**
 * @brief Get the current demo view mode.
 * @param App The demo application state.
 * @return The active view mode.
 */
TINYGL_DEMO_VIEW_MODE tinyglDemoGetViewMode(const TINYGL_DEMO_APP *App) {
    if (App == NULL) {
        return TINYGL_DEMO_VIEW_MODE_UNLIT_CUBE;
    }

    return App->ViewMode;
}

/************************************************************************/

/**
 * @brief Get the current color buffer pointer.
 * @param App The demo application state.
 * @return The color buffer pointer, or null.
 */
const void *tinyglDemoGetPixels(const TINYGL_DEMO_APP *App) {
    if (App == NULL) {
        return NULL;
    }

    return App->Surface.Pixels;
}

/************************************************************************/

/**
 * @brief Get the current color buffer pitch.
 * @param App The demo application state.
 * @return The pitch in bytes, or 0.
 */
GLsizei tinyglDemoGetPitch(const TINYGL_DEMO_APP *App) {
    if (App == NULL) {
        return 0;
    }

    return App->Surface.Pitch;
}
