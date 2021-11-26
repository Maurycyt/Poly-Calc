/** @file
 * @brief Interfejs modułu interfejsu użytkownika kalkulatora wielomianów.
 *
 * @author Maurycy Wojda
 * @date 2021
 */

#ifndef __POLY_UI_H__
#define __POLY_UI_H__

#include "polystack.h"
#include <stdbool.h>

/**
 * Zwraca wartość flagi, która oznacza dotarcie do końca pliku.
 * @return : `true`, jeżeli plik na wejściu standardowym się skończył,
 * `false` w przeciwnym przypadku.
 */
bool checkEOF();

/**
 * Przygotowuje interfejs użytkownika do przyjmowania wejścia.
 */
void PolyUIInit();

/**
 * Czyta następny wiersz wejścia standardowego i w pełni go obsługuje.
 * @param[in] s : wskaźnik na stos wielomianowy, na którym ma być
 * wykonane polecenie.
 */
void handleLine(PolyStack *s);

#endif /* __POLY_UI_H__ */