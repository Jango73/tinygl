#include <stdio.h>

#include <tinygl.h>

/************************************************************************/

/**
 * @brief Report a failure in the external linkage smoke test.
 * @param Message The failure message.
 * @return Always returns 1.
 */
static int failExternalSmoke(const char *Message) {
    fprintf(stderr, "external_linkage_smoke: %s\n", Message);
    return 1;
}

/************************************************************************/

/**
 * @brief Validate installed TinyGL headers and linkage.
 * @return 0 on success, 1 on failure.
 */
int main(void) {
    TGL_CONTEXT_DESC ContextDesc;
    TGLContext Context;
    TGL_RESULT Result;
    TGL_VERSION Version;

    Result = tinyglGetVersion(&Version);
    if (Result != TGL_RESULT_OK) {
        return failExternalSmoke("tinyglGetVersion failed");
    }

    if (Version.Major != TINYGL_VERSION_MAJOR ||
        Version.Minor != TINYGL_VERSION_MINOR ||
        Version.Patch != TINYGL_VERSION_PATCH) {
        return failExternalSmoke("version API does not match public defines");
    }

    ContextDesc.MaxWidth = 32;
    ContextDesc.MaxHeight = 32;
    ContextDesc.HasDepthBuffer = GL_FALSE;
    ContextDesc.HasColorBuffer = GL_TRUE;

    Context = NULL;
    Result = tinyglCreateContext(&ContextDesc, &Context);
    if (Result != TGL_RESULT_OK || Context == NULL) {
        return failExternalSmoke("tinyglCreateContext failed");
    }

    if (tinyglMakeCurrent(Context) != TGL_RESULT_OK) {
        tinyglDestroyContext(Context);
        return failExternalSmoke("tinyglMakeCurrent failed");
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    if (glGetError() != GL_NO_ERROR) {
        tinyglDestroyContext(Context);
        return failExternalSmoke("unexpected OpenGL error state");
    }

    if (tinyglDestroyContext(Context) != TGL_RESULT_OK) {
        return failExternalSmoke("tinyglDestroyContext failed");
    }

    return 0;
}
