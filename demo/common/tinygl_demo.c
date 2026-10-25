#include "tinygl_demo.h"
#include "tinygl_demo_overlay.h"

#include <math.h>
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
#define TINYGL_DEMO_MANY_CUBES_SEED 0x1A2B
#define TINYGL_DEMO_MANY_CUBES_MIN_SCALE 0.12f
#define TINYGL_DEMO_MANY_CUBES_MAX_SCALE 0.30f
#define TINYGL_DEMO_MANY_CUBES_MIN_POSITION_X -7.0f
#define TINYGL_DEMO_MANY_CUBES_MAX_POSITION_X 7.0f
#define TINYGL_DEMO_MANY_CUBES_MIN_POSITION_Y -4.0f
#define TINYGL_DEMO_MANY_CUBES_MAX_POSITION_Y 4.0f
#define TINYGL_DEMO_MANY_CUBES_MIN_POSITION_Z -12.0f
#define TINYGL_DEMO_MANY_CUBES_MAX_POSITION_Z 3.0f
#define TINYGL_DEMO_MANY_CUBES_MIN_SPEED 15.0f
#define TINYGL_DEMO_MANY_CUBES_MAX_SPEED 110.0f
#define TINYGL_DEMO_MANY_CUBES_FULL_ANGLE 360.0f
#define TINYGL_DEMO_MANY_CUBES_GOLDEN_RATIO 0.618034f
#define TINYGL_DEMO_MANY_CUBES_SATURATION 0.80f
#define TINYGL_DEMO_MANY_CUBES_VALUE 0.95f
#define TINYGL_DEMO_MANY_MATERIAL_AMBIENT_SCALE 0.55f
#define TINYGL_DEMO_MANY_MATERIAL_EMISSION_SCALE 0.28f
#define TINYGL_DEMO_MANY_NOISE_MIN_AMPLITUDE 0.25f
#define TINYGL_DEMO_MANY_NOISE_MAX_AMPLITUDE 0.70f
#define TINYGL_DEMO_MANY_NOISE_MIN_FREQUENCY 0.30f
#define TINYGL_DEMO_MANY_NOISE_MAX_FREQUENCY 1.10f
#define TINYGL_DEMO_MANY_NOISE_FULL_PHASE 6.283185f

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
static const GLfloat TINYGL_DEMO_MANY_KEY_AMBIENT[4] = {0.38f, 0.38f, 0.38f, 1.0f};
static const GLfloat TINYGL_DEMO_MANY_KEY_DIFFUSE[4] = {1.0f, 1.0f, 1.0f, 1.0f};
static const GLfloat TINYGL_DEMO_MANY_FILL_POSITION[4] = {-4.0f, -2.0f, 2.0f, 1.0f};
static const GLfloat TINYGL_DEMO_MANY_FILL_AMBIENT[4] = {0.18f, 0.18f, 0.18f, 1.0f};
static const GLfloat TINYGL_DEMO_MANY_FILL_DIFFUSE[4] = {0.60f, 0.60f, 0.60f, 1.0f};
static const GLfloat TINYGL_DEMO_MANY_FILL_SPECULAR[4] = {0.00f, 0.00f, 0.00f, 1.0f};

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
 * @brief Draw a random float inside a closed range.
 * @param MinValue The minimum value.
 * @param MaxValue The maximum value.
 * @return The random value.
 */
static GLfloat randomFloatInRange(GLfloat MinValue, GLfloat MaxValue) {
    GLfloat UnitValue;

    UnitValue = (GLfloat)rand() / (GLfloat)RAND_MAX;
    return MinValue + UnitValue * (MaxValue - MinValue);
}

/************************************************************************/

/**
 * @brief Sample a smooth pseudo noise value in the [-1, 1] range.
 * @param Frequency The oscillation frequency.
 * @param Phase The random phase offset.
 * @param ElapsedSeconds The elapsed demo time in seconds.
 * @return The noise value.
 */
static GLfloat sampleNoiseComponent(GLfloat Frequency, GLfloat Phase,
                                    GLfloat ElapsedSeconds) {
    GLfloat FirstHarmonic;
    GLfloat SecondHarmonic;
    GLfloat ThirdHarmonic;

    FirstHarmonic = sinf(Frequency * ElapsedSeconds + Phase);
    SecondHarmonic = sinf(2.33f * Frequency * ElapsedSeconds + 1.70f * Phase);
    ThirdHarmonic = sinf(4.11f * Frequency * ElapsedSeconds + 2.30f * Phase);

    return 0.60f * FirstHarmonic + 0.28f * SecondHarmonic +
           0.12f * ThirdHarmonic;
}

/************************************************************************/

/**
 * @brief Convert a hue value to an opaque RGB color.
 * @param Hue The hue value in the [0, 1) range.
 * @param OutRed The red destination.
 * @param OutGreen The green destination.
 * @param OutBlue The blue destination.
 */
static void convertHueToRgb(GLfloat Hue, GLfloat *OutRed, GLfloat *OutGreen,
                            GLfloat *OutBlue) {
    GLfloat Saturation;
    GLfloat Value;
    GLfloat Sector;
    GLfloat Fraction;
    GLfloat MixA;
    GLfloat MixB;
    GLfloat MixC;
    int SectorIndex;

    Saturation = TINYGL_DEMO_MANY_CUBES_SATURATION;
    Value = TINYGL_DEMO_MANY_CUBES_VALUE;

    while (Hue < 0.0f) {
        Hue += 1.0f;
    }
    while (Hue >= 1.0f) {
        Hue -= 1.0f;
    }

    Sector = Hue * 6.0f;
    SectorIndex = (int)Sector;
    Fraction = Sector - (GLfloat)SectorIndex;
    MixA = Value * (1.0f - Saturation);
    MixB = Value * (1.0f - Saturation * Fraction);
    MixC = Value * (1.0f - Saturation * (1.0f - Fraction));

    switch (SectorIndex) {
    case 0: {
        *OutRed = Value;
        *OutGreen = MixC;
        *OutBlue = MixA;
    } break;

    case 1: {
        *OutRed = MixB;
        *OutGreen = Value;
        *OutBlue = MixA;
    } break;

    case 2: {
        *OutRed = MixA;
        *OutGreen = Value;
        *OutBlue = MixC;
    } break;

    case 3: {
        *OutRed = MixA;
        *OutGreen = MixB;
        *OutBlue = Value;
    } break;

    case 4: {
        *OutRed = MixC;
        *OutGreen = MixA;
        *OutBlue = Value;
    } break;

    default: {
        *OutRed = Value;
        *OutGreen = MixA;
        *OutBlue = MixB;
    } break;
    }
}

/************************************************************************/

/**
 * @brief Initialize the 200 cubes with random transforms and colors.
 * @param App The demo application state.
 */
static void initializeManyCubes(TINYGL_DEMO_APP *App) {
    int Index;

    srand(TINYGL_DEMO_MANY_CUBES_SEED);
    App->ElapsedSeconds = 0.0f;

    for (Index = 0; Index < TINYGL_DEMO_MANY_CUBES_COUNT; Index++) {
        TINYGL_DEMO_CUBE *Cube;
        GLfloat Hue;
        GLfloat SpeedSign;

        Cube = &App->Cubes[Index];
        Cube->BasePositionX = randomFloatInRange(
            TINYGL_DEMO_MANY_CUBES_MIN_POSITION_X,
            TINYGL_DEMO_MANY_CUBES_MAX_POSITION_X);
        Cube->BasePositionY = randomFloatInRange(
            TINYGL_DEMO_MANY_CUBES_MIN_POSITION_Y,
            TINYGL_DEMO_MANY_CUBES_MAX_POSITION_Y);
        Cube->BasePositionZ = randomFloatInRange(
            TINYGL_DEMO_MANY_CUBES_MIN_POSITION_Z,
            TINYGL_DEMO_MANY_CUBES_MAX_POSITION_Z);
        Cube->NoisePhaseX = randomFloatInRange(
            0.0f, TINYGL_DEMO_MANY_NOISE_FULL_PHASE);
        Cube->NoisePhaseY = randomFloatInRange(
            0.0f, TINYGL_DEMO_MANY_NOISE_FULL_PHASE);
        Cube->NoisePhaseZ = randomFloatInRange(
            0.0f, TINYGL_DEMO_MANY_NOISE_FULL_PHASE);
        Cube->NoiseFrequencyX = randomFloatInRange(
            TINYGL_DEMO_MANY_NOISE_MIN_FREQUENCY,
            TINYGL_DEMO_MANY_NOISE_MAX_FREQUENCY);
        Cube->NoiseFrequencyY = randomFloatInRange(
            TINYGL_DEMO_MANY_NOISE_MIN_FREQUENCY,
            TINYGL_DEMO_MANY_NOISE_MAX_FREQUENCY);
        Cube->NoiseFrequencyZ = randomFloatInRange(
            TINYGL_DEMO_MANY_NOISE_MIN_FREQUENCY,
            TINYGL_DEMO_MANY_NOISE_MAX_FREQUENCY);
        Cube->NoiseAmplitudeX = randomFloatInRange(
            TINYGL_DEMO_MANY_NOISE_MIN_AMPLITUDE,
            TINYGL_DEMO_MANY_NOISE_MAX_AMPLITUDE);
        Cube->NoiseAmplitudeY = randomFloatInRange(
            TINYGL_DEMO_MANY_NOISE_MIN_AMPLITUDE,
            TINYGL_DEMO_MANY_NOISE_MAX_AMPLITUDE);
        Cube->NoiseAmplitudeZ = randomFloatInRange(
            TINYGL_DEMO_MANY_NOISE_MIN_AMPLITUDE,
            TINYGL_DEMO_MANY_NOISE_MAX_AMPLITUDE);
        Cube->RotationX =
            randomFloatInRange(0.0f, TINYGL_DEMO_MANY_CUBES_FULL_ANGLE);
        Cube->RotationY =
            randomFloatInRange(0.0f, TINYGL_DEMO_MANY_CUBES_FULL_ANGLE);
        Cube->Scale = randomFloatInRange(TINYGL_DEMO_MANY_CUBES_MIN_SCALE,
                                         TINYGL_DEMO_MANY_CUBES_MAX_SCALE);

        SpeedSign = (randomFloatInRange(0.0f, 1.0f) < 0.5f) ? -1.0f : 1.0f;
        Cube->SpeedX = SpeedSign *
                       randomFloatInRange(TINYGL_DEMO_MANY_CUBES_MIN_SPEED,
                                          TINYGL_DEMO_MANY_CUBES_MAX_SPEED);
        SpeedSign = (randomFloatInRange(0.0f, 1.0f) < 0.5f) ? -1.0f : 1.0f;
        Cube->SpeedY = SpeedSign *
                       randomFloatInRange(TINYGL_DEMO_MANY_CUBES_MIN_SPEED,
                                          TINYGL_DEMO_MANY_CUBES_MAX_SPEED);

        Hue = (GLfloat)Index * TINYGL_DEMO_MANY_CUBES_GOLDEN_RATIO;
        Hue += randomFloatInRange(0.0f, 0.08f);
        convertHueToRgb(Hue, &Cube->Red, &Cube->Green, &Cube->Blue);
    }
}

/************************************************************************/

/**
 * @brief Advance cube rotations and the shared oscillation clock.
 * @param App The demo application state.
 * @param DeltaSeconds The frame delta time in seconds.
 */
static void updateManyCubes(TINYGL_DEMO_APP *App, GLfloat DeltaSeconds) {
    int Index;

    App->ElapsedSeconds += DeltaSeconds;

    for (Index = 0; Index < TINYGL_DEMO_MANY_CUBES_COUNT; Index++) {
        TINYGL_DEMO_CUBE *Cube;

        Cube = &App->Cubes[Index];
        Cube->RotationX += Cube->SpeedX * DeltaSeconds;
        Cube->RotationY += Cube->SpeedY * DeltaSeconds;

        while (Cube->RotationX >= TINYGL_DEMO_MANY_CUBES_FULL_ANGLE) {
            Cube->RotationX -= TINYGL_DEMO_MANY_CUBES_FULL_ANGLE;
        }
        while (Cube->RotationX < 0.0f) {
            Cube->RotationX += TINYGL_DEMO_MANY_CUBES_FULL_ANGLE;
        }
        while (Cube->RotationY >= TINYGL_DEMO_MANY_CUBES_FULL_ANGLE) {
            Cube->RotationY -= TINYGL_DEMO_MANY_CUBES_FULL_ANGLE;
        }
        while (Cube->RotationY < 0.0f) {
            Cube->RotationY += TINYGL_DEMO_MANY_CUBES_FULL_ANGLE;
        }
    }
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
    if (App->ViewMode == TINYGL_DEMO_VIEW_MODE_MANY_CUBES) {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHT1);

        glLightfv(GL_LIGHT0, GL_POSITION, TINYGL_DEMO_LIGHT_POSITION);
        glLightfv(GL_LIGHT0, GL_AMBIENT, TINYGL_DEMO_MANY_KEY_AMBIENT);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, TINYGL_DEMO_MANY_KEY_DIFFUSE);
        glLightfv(GL_LIGHT0, GL_SPECULAR, TINYGL_DEMO_LIGHT_SPECULAR);

        glLightfv(GL_LIGHT1, GL_POSITION, TINYGL_DEMO_MANY_FILL_POSITION);
        glLightfv(GL_LIGHT1, GL_AMBIENT, TINYGL_DEMO_MANY_FILL_AMBIENT);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, TINYGL_DEMO_MANY_FILL_DIFFUSE);
        glLightfv(GL_LIGHT1, GL_SPECULAR, TINYGL_DEMO_MANY_FILL_SPECULAR);

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

    if (App->ViewMode == TINYGL_DEMO_VIEW_MODE_LIT_CUBE) {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glDisable(GL_LIGHT1);

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

    glDisable(GL_LIGHT1);
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
 * @brief Apply a per-cube lighted material color.
 * @param Red The red component.
 * @param Green The green component.
 * @param Blue The blue component.
 */
static void applyCubeMaterial(GLfloat Red, GLfloat Green, GLfloat Blue) {
    GLfloat AmbientColor[4];
    GLfloat DiffuseColor[4];
    GLfloat EmissionColor[4];

    AmbientColor[0] = Red * TINYGL_DEMO_MANY_MATERIAL_AMBIENT_SCALE;
    AmbientColor[1] = Green * TINYGL_DEMO_MANY_MATERIAL_AMBIENT_SCALE;
    AmbientColor[2] = Blue * TINYGL_DEMO_MANY_MATERIAL_AMBIENT_SCALE;
    AmbientColor[3] = 1.0f;
    DiffuseColor[0] = Red;
    DiffuseColor[1] = Green;
    DiffuseColor[2] = Blue;
    DiffuseColor[3] = 1.0f;
    EmissionColor[0] = Red * TINYGL_DEMO_MANY_MATERIAL_EMISSION_SCALE;
    EmissionColor[1] = Green * TINYGL_DEMO_MANY_MATERIAL_EMISSION_SCALE;
    EmissionColor[2] = Blue * TINYGL_DEMO_MANY_MATERIAL_EMISSION_SCALE;
    EmissionColor[3] = 1.0f;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, AmbientColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, DiffuseColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, EmissionColor);
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

    if (ViewMode != TINYGL_DEMO_VIEW_MODE_UNLIT_CUBE) {
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
 * @brief Draw 200 cubes oscillating around their spawn points.
 * @param App The demo application state.
 */
static void drawManyCubesScene(const TINYGL_DEMO_APP *App) {
    int Index;

    for (Index = 0; Index < TINYGL_DEMO_MANY_CUBES_COUNT; Index++) {
        const TINYGL_DEMO_CUBE *Cube;
        GLfloat CurrentX;
        GLfloat CurrentY;
        GLfloat CurrentZ;

        Cube = &App->Cubes[Index];
        CurrentX =
            Cube->BasePositionX +
            Cube->NoiseAmplitudeX *
                sampleNoiseComponent(Cube->NoiseFrequencyX, Cube->NoisePhaseX,
                                     App->ElapsedSeconds);
        CurrentY =
            Cube->BasePositionY +
            Cube->NoiseAmplitudeY *
                sampleNoiseComponent(Cube->NoiseFrequencyY, Cube->NoisePhaseY,
                                     App->ElapsedSeconds);
        CurrentZ =
            Cube->BasePositionZ +
            Cube->NoiseAmplitudeZ *
                sampleNoiseComponent(Cube->NoiseFrequencyZ, Cube->NoisePhaseZ,
                                     App->ElapsedSeconds);
        glPushMatrix();
        glTranslatef(CurrentX, CurrentY, CurrentZ);
        glRotatef(Cube->RotationX, 1.0f, 0.0f, 0.0f);
        glRotatef(Cube->RotationY, 0.0f, 1.0f, 0.0f);
        glScalef(Cube->Scale, Cube->Scale, Cube->Scale);
        applyCubeMaterial(Cube->Red, Cube->Green, Cube->Blue);
        drawCube(App->ViewMode);
        glPopMatrix();
    }
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

    App->ViewMode = TINYGL_DEMO_VIEW_MODE_UNLIT_CUBE;
    initializeManyCubes(App);

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
    App->ElapsedSeconds = 0.0f;
    App->FpsAccumulator = 0.0f;
    App->FpsFrameCount = 0;
    App->CurrentFps = 0;
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
    updateManyCubes(App, DeltaSeconds);
    tinyglDemoUpdateFpsCounter(App, DeltaSeconds);

    configureFrameState(App);
    configureLightingState(App);

    if (App->ViewMode == TINYGL_DEMO_VIEW_MODE_MANY_CUBES) {
        drawManyCubesScene(App);
    } else {
        drawDepthValidationScene(App);
    }
    glFlush();

    if (glGetError() != GL_NO_ERROR) {
        return printDemoError("rendering failed");
    }

    tinyglDemoDrawFpsOverlay(App);

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
    case TINYGL_DEMO_VIEW_MODE_LIT_CUBE:
    case TINYGL_DEMO_VIEW_MODE_MANY_CUBES: {
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
