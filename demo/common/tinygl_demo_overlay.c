#include "tinygl_demo_overlay.h"

#include <stdio.h>

#define TINYGL_DEMO_OVERLAY_GLYPH_WIDTH 5
#define TINYGL_DEMO_OVERLAY_GLYPH_HEIGHT 7
#define TINYGL_DEMO_OVERLAY_SCALE 2
#define TINYGL_DEMO_OVERLAY_MARGIN 8
#define TINYGL_DEMO_OVERLAY_SPACING 1
#define TINYGL_DEMO_OVERLAY_UPDATE_INTERVAL 0.5f
#define TINYGL_DEMO_OVERLAY_TEXT_COLOR 0x00FFFFFF
#define TINYGL_DEMO_OVERLAY_BACKGROUND_COLOR 0x00000000
#define TINYGL_DEMO_OVERLAY_TEXT_LENGTH 16

/************************************************************************/

static const unsigned char TINYGL_DEMO_OVERLAY_GLYPHS[15][7] = {
    {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E},
    {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E},
    {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F},
    {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E},
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02},
    {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E},
    {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E},
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08},
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E},
    {0x0E, 0x11, 0x11, 0x1F, 0x01, 0x02, 0x0C},
    {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10},
    {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10},
    {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E},
    {0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

/************************************************************************/

/**
 * @brief Map an overlay character to its 5x7 glyph rows.
 * @param Character The character to map.
 * @return The glyph rows, or the space glyph.
 */
static const unsigned char *getOverlayGlyphRows(char Character) {
    if (Character >= '0' && Character <= '9') {
        return TINYGL_DEMO_OVERLAY_GLYPHS[Character - '0'];
    }

    if (Character == 'F') {
        return TINYGL_DEMO_OVERLAY_GLYPHS[10];
    }

    if (Character == 'P') {
        return TINYGL_DEMO_OVERLAY_GLYPHS[11];
    }

    if (Character == 'S') {
        return TINYGL_DEMO_OVERLAY_GLYPHS[12];
    }

    if (Character == ':') {
        return TINYGL_DEMO_OVERLAY_GLYPHS[13];
    }

    return TINYGL_DEMO_OVERLAY_GLYPHS[14];
}

/************************************************************************/

/**
 * @brief Update the smoothed frames-per-second counter.
 * @param App The demo application state.
 * @param DeltaSeconds The frame delta time in seconds.
 */
void tinyglDemoUpdateFpsCounter(TINYGL_DEMO_APP *App, GLfloat DeltaSeconds) {
    if (DeltaSeconds <= 0.0f) {
        return;
    }

    App->FpsAccumulator += DeltaSeconds;
    App->FpsFrameCount++;

    if (App->FpsAccumulator >= TINYGL_DEMO_OVERLAY_UPDATE_INTERVAL) {
        App->CurrentFps =
            (int)((GLfloat)App->FpsFrameCount / App->FpsAccumulator + 0.5f);
        App->FpsAccumulator = 0.0f;
        App->FpsFrameCount = 0;
    }
}

/************************************************************************/

/**
 * @brief Draw one scaled glyph row into the overlay box.
 * @param Pixels The color buffer pixels.
 * @param RowStride The row stride in pixels.
 * @param GlyphX The glyph origin X.
 * @param GlyphY The glyph origin Y.
 * @param PixelX The glyph column.
 * @param PixelY The glyph row.
 */
static void drawOverlayPixel(unsigned int *Pixels, int RowStride, int GlyphX,
                             int GlyphY, int PixelX, int PixelY) {
    int ScaleX;
    int ScaleY;

    for (ScaleY = 0; ScaleY < TINYGL_DEMO_OVERLAY_SCALE; ScaleY++) {
        for (ScaleX = 0; ScaleX < TINYGL_DEMO_OVERLAY_SCALE; ScaleX++) {
            int TargetX;
            int TargetY;

            TargetX = GlyphX + PixelX * TINYGL_DEMO_OVERLAY_SCALE + ScaleX;
            TargetY = GlyphY +
                      (TINYGL_DEMO_OVERLAY_GLYPH_HEIGHT - 1 - PixelY) *
                          TINYGL_DEMO_OVERLAY_SCALE +
                      ScaleY;
            Pixels[TargetY * RowStride + TargetX] =
                TINYGL_DEMO_OVERLAY_TEXT_COLOR;
        }
    }
}

/************************************************************************/

/**
 * @brief Draw the frames-per-second readout in the bottom-right corner.
 * @param App The demo application state.
 */
void tinyglDemoDrawFpsOverlay(TINYGL_DEMO_APP *App) {
    char Text[TINYGL_DEMO_OVERLAY_TEXT_LENGTH];
    int TextLength;
    int BoxWidth;
    int BoxHeight;
    int BoxX;
    int BoxY;
    int CharIndex;
    int PixelX;
    int PixelY;
    unsigned int *Pixels;
    int RowStride;

    snprintf(Text, sizeof(Text), "FPS: %d", App->CurrentFps);
    Text[sizeof(Text) - 1] = '\0';
    TextLength = 0;
    while (Text[TextLength] != '\0') {
        TextLength++;
    }

    if (TextLength <= 0 || App->Width <= 0 || App->Height <= 0) {
        return;
    }

    BoxWidth = TextLength *
                   (TINYGL_DEMO_OVERLAY_GLYPH_WIDTH +
                    TINYGL_DEMO_OVERLAY_SPACING) *
                   TINYGL_DEMO_OVERLAY_SCALE +
               TINYGL_DEMO_OVERLAY_MARGIN;
    BoxHeight = TINYGL_DEMO_OVERLAY_GLYPH_HEIGHT *
                    TINYGL_DEMO_OVERLAY_SCALE +
                TINYGL_DEMO_OVERLAY_MARGIN;

    if (BoxWidth + TINYGL_DEMO_OVERLAY_MARGIN > App->Width ||
        BoxHeight + TINYGL_DEMO_OVERLAY_MARGIN > App->Height) {
        return;
    }

    BoxX = App->Width - BoxWidth - TINYGL_DEMO_OVERLAY_MARGIN;
    BoxY = TINYGL_DEMO_OVERLAY_MARGIN;

    Pixels = (unsigned int *)App->ColorBuffer;
    RowStride = App->Surface.Pitch / 4;

    for (PixelY = BoxY; PixelY < BoxY + BoxHeight; PixelY++) {
        for (PixelX = BoxX; PixelX < BoxX + BoxWidth; PixelX++) {
            Pixels[PixelY * RowStride + PixelX] =
                TINYGL_DEMO_OVERLAY_BACKGROUND_COLOR;
        }
    }

    for (CharIndex = 0; CharIndex < TextLength; CharIndex++) {
        const unsigned char *GlyphRows;
        int GlyphX;
        int GlyphY;

        GlyphRows = getOverlayGlyphRows(Text[CharIndex]);
        GlyphX = BoxX + TINYGL_DEMO_OVERLAY_MARGIN / 2 +
                 CharIndex *
                     (TINYGL_DEMO_OVERLAY_GLYPH_WIDTH +
                      TINYGL_DEMO_OVERLAY_SPACING) *
                     TINYGL_DEMO_OVERLAY_SCALE;
        GlyphY = BoxY + TINYGL_DEMO_OVERLAY_MARGIN / 2;

        for (PixelY = 0; PixelY < TINYGL_DEMO_OVERLAY_GLYPH_HEIGHT; PixelY++) {
            for (PixelX = 0; PixelX < TINYGL_DEMO_OVERLAY_GLYPH_WIDTH;
                 PixelX++) {
                if ((GlyphRows[PixelY] >>
                     (TINYGL_DEMO_OVERLAY_GLYPH_WIDTH - 1 - PixelX)) &
                    0x01) {
                    drawOverlayPixel(Pixels, RowStride, GlyphX, GlyphY, PixelX,
                                     PixelY);
                }
            }
        }
    }
}
