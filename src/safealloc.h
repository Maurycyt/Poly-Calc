/** @file
 * @brief Interfejs funkcji usprawniających alokację pamięci.
 *
 * @author Maurycy Wojda
 * @date 2021
 */

#ifndef __SAFE_ALLOC_H__
#define __SAFE_ALLOC_H__

#include <stdlib.h>

/**
 * Stała określająca domyślny rozmiar tablic 
 * w dynamicznie alokowanych strukturach.
 */
#define DEFAULT_SIZE 16

/**
 * Alokuje pamięć z obsługą błędu alokacji.
 * Dokonuje próby alokacji pamięci za pomocą `malloc`. Jeżeli
 * próba się nie powiedzie, to kończy program z kodem 1.
 * @param[in] size : liczba bajtów do alokacji
 * @return : wskaźnik na zaalokowany blok pamięci
 */
void *safeMalloc(size_t size);

/**
 * Realokuje pamięć tablicy z obsługą błędu alokacji.
 * Dokonuje próby realokacji pamięci za pomocą `realloc`. Jeżeli
 * próba się nie powiedzie, to kończy program z kodem 1.
 * W przeciwnym przypadku automatycznie podmienia podany wskaźnik.
 * @param[in,out] bufferPointer : wskaźnik na wskaźnik na bufor (tablicę)
 * @param[in] newSize : nowy rozmiar bufora (tablicy) w liczbie bajtach
 */
void safeRealloc(void **bufferPointer, size_t newSize);

#endif /* __SAFE_ALLOC_H__ */