/** @file
 * @brief Implementacja funkcji usprawniających alokację pamięci.
 *
 * @author Maurycy Wojda
 * @date 2021
 */

#include "safealloc.h"

/**
 * Kod błędu, z którym program ma się zakończyć, jeżeli
 * zabraknie mu pamięci.
 */
#define MEM_PROBLEM_CODE 1

void *safeMalloc(size_t size)
{
	if (size == 0)
		return NULL;
	void *pointer = malloc(size);
	if (pointer == NULL && size != 0)
		exit(MEM_PROBLEM_CODE);
	else
		return pointer;
}

void safeRealloc(void **bufferPointer, size_t newSize)
{
	if (newSize == 0)
	{
		free(*bufferPointer);
		*bufferPointer = NULL;
	}
	else
	{
		*bufferPointer = realloc(*bufferPointer, newSize);
	}
	if (*bufferPointer == NULL && newSize != 0)
		exit(MEM_PROBLEM_CODE);
}