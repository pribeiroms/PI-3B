/* Com -std=c11 o MinGW.org antigo esconde as funcoes fora do padrao C, como o
 * _wfopen. Liberar so neste arquivo mantem o resto do projeto em C11 estrito. */
#if defined(__MINGW32__)
#undef __STRICT_ANSI__
#endif

#include "utf8io.h"

#include <stdlib.h>

#ifdef _WIN32

#include <windows.h>

FILE *utf8_fopen(const char *caminho_utf8, const char *modo)
{
    wchar_t *caminho_w;
    wchar_t modo_w[8];
    int tamanho_caminho;
    FILE *arquivo;

    if (caminho_utf8 == NULL || modo == NULL) return NULL;

    tamanho_caminho = MultiByteToWideChar(CP_UTF8, 0, caminho_utf8, -1, NULL, 0);
    if (tamanho_caminho <= 0) return NULL;

    caminho_w = (wchar_t *)malloc((size_t)tamanho_caminho * sizeof(wchar_t));
    if (caminho_w == NULL) return NULL;

    if (MultiByteToWideChar(CP_UTF8, 0, caminho_utf8, -1, caminho_w,
            tamanho_caminho) <= 0) {
        free(caminho_w);
        return NULL;
    }

    if (MultiByteToWideChar(CP_UTF8, 0, modo, -1, modo_w,
            (int)(sizeof(modo_w) / sizeof(modo_w[0]))) <= 0) {
        free(caminho_w);
        return NULL;
    }

    arquivo = _wfopen(caminho_w, modo_w);
    free(caminho_w);
    return arquivo;
}

#else

FILE *utf8_fopen(const char *caminho_utf8, const char *modo)
{
    return fopen(caminho_utf8, modo);
}

#endif