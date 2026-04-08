#include <stdio.h>
#include <string.h>

#include "../../include/tinygl.h"

#define SMOKE_SURFACE_WIDTH 64
#define SMOKE_SURFACE_HEIGHT 64
#define SMOKE_PIXEL_SIZE 4
#define SMOKE_MIN_COLORED_PIXELS 8

typedef struct SMOKE_PRESENT_STATE {
    int PresentCallCount;
    const void *LastPixels;
    GLsizei LastWidth;
    GLsizei LastHeight;
    GLsizei LastPitch;
} SMOKE_PRESENT_STATE;

/************************************************************************/

/**
 * @brief Print a failure message and return an error code.
 * @param Message The failure description.
 * @return Always returns 1.
 */
static int failSmokeTest(const char *Message) {
    fprintf(stderr, "portable_core_smoke: %s\n", Message);
    return 1;
}

/************************************************************************/

/**
 * @brief Validate a TinyGL result code.
 * @param Result The result to validate.
 * @param Message The failure description.
 * @return 0 on success, 1 on failure.
 */
static int checkTinyGlResult(TGL_RESULT Result, const char *Message) {
    if (Result == TGL_RESULT_OK) {
        return 0;
    }

    return failSmokeTest(Message);
}

/************************************************************************/

/**
 * @brief Validate a TinyGL present result code.
 * @param Result The result to validate.
 * @param Message The failure description.
 * @return 0 on success, 1 on failure.
 */
static int checkPresentResult(TGL_PRESENT_RESULT Result, const char *Message) {
    if (Result == TGL_PRESENT_RESULT_OK) {
        return 0;
    }

    return failSmokeTest(Message);
}

/************************************************************************/

/**
 * @brief Validate the current OpenGL error state.
 * @param Message The failure description.
 * @return 0 on success, 1 on failure.
 */
static int checkGlError(const char *Message) {
    GLenum ErrorCode;

    ErrorCode = glGetError();
    if (ErrorCode == GL_NO_ERROR) {
        return 0;
    }

    fprintf(stderr, "portable_core_smoke: %s (glError=0x%04x)\n", Message,
            (unsigned int)ErrorCode);
    return 1;
}

/************************************************************************/

/**
 * @brief Clear any pending OpenGL errors.
 */
static void clearGlErrors(void) {
    while (glGetError() != GL_NO_ERROR) {
    }
}

/************************************************************************/

/**
 * @brief Count pixels matching a color mask.
 * @param Surface The surface to inspect.
 * @param Mask The bit mask used for comparison.
 * @param ExpectedValue The masked value expected in matching pixels.
 * @return The number of matching pixels.
 */
static int countMatchingPixels(const TGL_SURFACE_DESC *Surface, unsigned int Mask,
                               unsigned int ExpectedValue) {
    int MatchCount;
    GLsizei RowIndex;
    GLsizei ColumnIndex;

    MatchCount = 0;

    for (RowIndex = 0; RowIndex < Surface->Height; RowIndex++) {
        const unsigned char *RowBytes;
        const unsigned int *RowPixels;

        RowBytes = (const unsigned char *)Surface->Pixels +
                   (RowIndex * Surface->Pitch);
        RowPixels = (const unsigned int *)RowBytes;

        for (ColumnIndex = 0; ColumnIndex < Surface->Width; ColumnIndex++) {
            if ((RowPixels[ColumnIndex] & Mask) == ExpectedValue) {
                MatchCount++;
            }
        }
    }

    return MatchCount;
}

/************************************************************************/

/**
 * @brief Count all non-zero pixels in a surface.
 * @param Surface The surface to inspect.
 * @return The number of non-zero pixels.
 */
static int countNonZeroPixels(const TGL_SURFACE_DESC *Surface) {
    int MatchCount;
    GLsizei RowIndex;
    GLsizei ColumnIndex;

    MatchCount = 0;

    for (RowIndex = 0; RowIndex < Surface->Height; RowIndex++) {
        const unsigned char *RowBytes;
        const unsigned int *RowPixels;

        RowBytes = (const unsigned char *)Surface->Pixels +
                   (RowIndex * Surface->Pitch);
        RowPixels = (const unsigned int *)RowBytes;

        for (ColumnIndex = 0; ColumnIndex < Surface->Width; ColumnIndex++) {
            if (RowPixels[ColumnIndex] != 0) {
                MatchCount++;
            }
        }
    }

    return MatchCount;
}

/************************************************************************/

/**
 * @brief Configure a deterministic 2D render state for the smoke test.
 */
static void configureSmokeRenderState(void) {
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, SMOKE_SURFACE_WIDTH, SMOKE_SURFACE_HEIGHT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/************************************************************************/

/**
 * @brief Render a colored triangle into the current surface.
 * @param Red The red component.
 * @param Green The green component.
 * @param Blue The blue component.
 * @param LeftX The left vertex X coordinate.
 * @param BottomY The bottom vertex Y coordinate.
 * @param RightX The right vertex X coordinate.
 * @param TopY The top vertex Y coordinate.
 */
static void renderSmokeTriangle(GLfloat Red, GLfloat Green, GLfloat Blue,
                                GLfloat LeftX, GLfloat BottomY,
                                GLfloat RightX, GLfloat TopY) {
    GLfloat Emission[4];

    configureSmokeRenderState();

    Emission[0] = Red;
    Emission[1] = Green;
    Emission[2] = Blue;
    Emission[3] = 1.0f;

    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Emission);
    glBegin(GL_TRIANGLES);
    glVertex2f(LeftX, BottomY);
    glVertex2f(RightX, BottomY);
    glVertex2f(0.0f, TopY);
    glEnd();
}

/************************************************************************/

/**
 * @brief Track bridge present calls during the smoke test.
 * @param UserData The callback user data.
 * @param Surface The presented surface.
 * @return The TinyGL present status.
 */
static TGL_PRESENT_RESULT APIENTRY smokePresentCallback(
    void *UserData, const TGL_SURFACE_DESC *Surface) {
    SMOKE_PRESENT_STATE *PresentState;

    if (UserData == NULL || Surface == NULL || Surface->Pixels == NULL) {
        return TGL_PRESENT_RESULT_FAILED;
    }

    PresentState = (SMOKE_PRESENT_STATE *)UserData;
    PresentState->PresentCallCount++;
    PresentState->LastPixels = Surface->Pixels;
    PresentState->LastWidth = Surface->Width;
    PresentState->LastHeight = Surface->Height;
    PresentState->LastPitch = Surface->Pitch;

    return TGL_PRESENT_RESULT_OK;
}

/************************************************************************/

/**
 * @brief Validate host-provided surface rendering.
 * @param Context The TinyGL context to validate.
 * @param PresentState The bridge callback state.
 * @return 0 on success, 1 on failure.
 */
static int runHostSurfaceSmoke(TGLContext Context,
                               SMOKE_PRESENT_STATE *PresentState) {
    TGL_SURFACE_DESC Surface;
    int BluePixelCount;
    int GreenPixelCount;
    int NonZeroPixelCount;
    unsigned int Pixels[SMOKE_SURFACE_WIDTH * SMOKE_SURFACE_HEIGHT];
    int RedPixelCount;

    memset(Pixels, 0, sizeof(Pixels));
    memset(&Surface, 0, sizeof(Surface));

    Surface.Pixels = Pixels;
    Surface.Width = SMOKE_SURFACE_WIDTH;
    Surface.Height = SMOKE_SURFACE_HEIGHT;
    Surface.Pitch = SMOKE_SURFACE_WIDTH * SMOKE_PIXEL_SIZE;
    Surface.PixelFormat = TGL_PIXEL_FORMAT_XRGB8888;

    if (checkTinyGlResult(tinyglSetSurface(Context, &Surface),
                          "failed to attach the host surface")) {
        return 1;
    }

    if (tinyglGetSurfaceMode(Context) != TGL_SURFACE_MODE_HOST_PROVIDED) {
        return failSmokeTest("host surface mode was not selected");
    }

    clearGlErrors();
    renderSmokeTriangle(1.0f, 0.0f, 0.0f, -0.75f, -0.75f, 0.75f, 0.75f);
    if (checkGlError("OpenGL reported an error on the host surface")) {
        return 1;
    }

    RedPixelCount = countMatchingPixels(&Surface, 0x00FF0000, 0x00FF0000);
    GreenPixelCount = countMatchingPixels(&Surface, 0x0000FF00, 0x0000FF00);
    BluePixelCount = countMatchingPixels(&Surface, 0x000000FF, 0x000000FF);
    NonZeroPixelCount = countNonZeroPixels(&Surface);
    if (RedPixelCount < SMOKE_MIN_COLORED_PIXELS) {
        fprintf(stderr,
                "portable_core_smoke: host surface counts red=%d green=%d blue=%d nonzero=%d\n",
                RedPixelCount, GreenPixelCount, BluePixelCount,
                NonZeroPixelCount);
        return failSmokeTest("host surface render did not modify enough pixels");
    }

    if (checkPresentResult(tinyglPresent(Context),
                           "present failed on the host surface")) {
        return 1;
    }

    if (PresentState->PresentCallCount != 1 ||
        PresentState->LastPixels != Surface.Pixels ||
        PresentState->LastWidth != Surface.Width ||
        PresentState->LastHeight != Surface.Height ||
        PresentState->LastPitch != Surface.Pitch) {
        return failSmokeTest("host surface present callback did not receive the expected surface");
    }

    return 0;
}

/************************************************************************/

/**
 * @brief Validate TinyGL-owned internal surface rendering.
 * @param Context The TinyGL context to validate.
 * @param PresentState The bridge callback state.
 * @return 0 on success, 1 on failure.
 */
static int runInternalSurfaceSmoke(TGLContext Context,
                                   SMOKE_PRESENT_STATE *PresentState) {
    int BluePixelCount;
    int NonZeroPixelCount;
    int RedPixelCount;
    TGL_SURFACE_DESC Surface;
    int GreenPixelCount;

    if (checkTinyGlResult(
            tinyglCreateInternalSurface(Context, SMOKE_SURFACE_WIDTH,
                                        SMOKE_SURFACE_HEIGHT,
                                        TGL_PIXEL_FORMAT_XRGB8888),
            "failed to create the internal surface")) {
        return 1;
    }

    if (tinyglGetSurfaceMode(Context) != TGL_SURFACE_MODE_INTERNAL) {
        return failSmokeTest("internal surface mode was not selected");
    }

    memset(&Surface, 0, sizeof(Surface));
    if (checkTinyGlResult(tinyglGetSurface(Context, &Surface),
                          "failed to query the internal surface")) {
        return 1;
    }

    memset(Surface.Pixels, 0, Surface.Pitch * Surface.Height);

    clearGlErrors();
    renderSmokeTriangle(0.0f, 1.0f, 0.0f, -0.75f, -0.25f, 0.75f, 0.85f);
    if (checkGlError("OpenGL reported an error on the internal surface")) {
        return 1;
    }

    GreenPixelCount = countMatchingPixels(&Surface, 0x0000FF00, 0x0000FF00);
    RedPixelCount = countMatchingPixels(&Surface, 0x00FF0000, 0x00FF0000);
    BluePixelCount = countMatchingPixels(&Surface, 0x000000FF, 0x000000FF);
    NonZeroPixelCount = countNonZeroPixels(&Surface);
    if (GreenPixelCount < SMOKE_MIN_COLORED_PIXELS) {
        fprintf(stderr,
                "portable_core_smoke: internal surface counts red=%d green=%d blue=%d nonzero=%d\n",
                RedPixelCount, GreenPixelCount, BluePixelCount,
                NonZeroPixelCount);
        return failSmokeTest("internal surface render did not modify enough pixels");
    }

    if (checkPresentResult(tinyglPresent(Context),
                           "present failed on the internal surface")) {
        return 1;
    }

    if (PresentState->PresentCallCount != 2 ||
        PresentState->LastPixels != Surface.Pixels) {
        return failSmokeTest("internal surface present callback did not receive the expected surface");
    }

    if (checkTinyGlResult(tinyglDestroyInternalSurface(Context),
                          "failed to destroy the internal surface")) {
        return 1;
    }

    return 0;
}

/************************************************************************/

/**
 * @brief Entry point for the portable core smoke test.
 * @return 0 on success, 1 on failure.
 */
int main(void) {
    TGL_CONTEXT_DESC ContextDesc;
    TGL_BRIDGE_CALLBACKS BridgeCallbacks;
    SMOKE_PRESENT_STATE PresentState;
    TGLContext Context;

    memset(&ContextDesc, 0, sizeof(ContextDesc));
    memset(&BridgeCallbacks, 0, sizeof(BridgeCallbacks));
    memset(&PresentState, 0, sizeof(PresentState));

    ContextDesc.MaxWidth = SMOKE_SURFACE_WIDTH;
    ContextDesc.MaxHeight = SMOKE_SURFACE_HEIGHT;
    ContextDesc.HasDepthBuffer = GL_TRUE;
    ContextDesc.HasColorBuffer = GL_TRUE;

    Context = NULL;
    if (checkTinyGlResult(tinyglCreateContext(&ContextDesc, &Context),
                          "failed to create a TinyGL context")) {
        return 1;
    }

    if (checkTinyGlResult(tinyglMakeCurrent(Context),
                          "failed to make the TinyGL context current")) {
        return 1;
    }

    BridgeCallbacks.Present = smokePresentCallback;
    BridgeCallbacks.UserData = &PresentState;
    if (checkTinyGlResult(tinyglSetBridgeCallbacks(Context, &BridgeCallbacks),
                          "failed to register bridge callbacks")) {
        return 1;
    }

    if (runHostSurfaceSmoke(Context, &PresentState)) {
        return 1;
    }

    if (runInternalSurfaceSmoke(Context, &PresentState)) {
        return 1;
    }

    if (checkTinyGlResult(tinyglMakeCurrent(NULL),
                          "failed to clear the current TinyGL context")) {
        return 1;
    }

    if (checkTinyGlResult(tinyglDestroyContext(Context),
                          "failed to destroy the TinyGL context")) {
        return 1;
    }

    return 0;
}
