#ifndef UTF8IO_H
#define UTF8IO_H

#include <stdio.h>

/* Abre um arquivo cujo caminho esta codificado em UTF-8.
 *
 * No Windows, fopen() interpreta o caminho na code page ANSI do sistema,
 * nao em UTF-8 -- caminhos com acentos (ex.: "Area de Trabalho") podem
 * abrir o arquivo errado ou falhar mesmo quando o arquivo existe. Esta
 * funcao converte o caminho para UTF-16 e usa _wfopen, que sempre
 * funciona independente da code page configurada no sistema.
 *
 * Em outras plataformas, apenas repassa para fopen() normalmente.
 */
FILE *utf8_fopen(const char *caminho_utf8, const char *modo);

#endif