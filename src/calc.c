/** @file
 * @brief Implementacja głównej funkcji kalkulatora wielomianów.
 * 
 * @author Maurycy Wojda
 * @date 2021
 */

#include "polystack.h"
#include "polyui.h"

/**
 * Funkcja główna kalkulatora wielomianów.
 */
int main()
{
	PolyStack stack = PSInit();
	PolyUIInit();

	while (!checkEOF())
		handleLine(&stack);

	PSDestroy(&stack);
}