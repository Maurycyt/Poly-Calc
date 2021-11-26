/** @file
 * @brief Implementacja klasy stosu operacji na wielomianach.
 *
 * @author Maurycy Wojda
 * @date 2021
 */

#include "polystack.h"
#include "safealloc.h"
#include "poly.h"
#include <stdio.h>
#include <stdbool.h>

PolyStack PSInit()
{
	PolyStack result;
	result.elems = 0;
	result.size = DEFAULT_SIZE;
	result.stack = safeMalloc(DEFAULT_SIZE * sizeof(Poly));
	return result;
}

void PSPush(PolyStack *s, const Poly p)
{
	if (s->elems == s->size)
	{
		s->size *= 2;
		safeRealloc((void**)&s->stack, s->size * sizeof(Poly));
	}
	s->stack[s->elems++] = p;
}

Poly PSGet(const PolyStack *s, size_t pos)
{
	return s->stack[s->elems - pos];
}

Poly *PSGetPtr(const PolyStack *s, size_t pos)
{
	return s->stack + (s->elems - pos);
}

Poly PSPeek(const PolyStack *s)
{
	return PSGet(s, 1);
}

Poly *PSPeekPtr(const PolyStack *s)
{
	return PSGetPtr(s, 1);
}

Poly PSPop(PolyStack *s)
{
	Poly result = s->stack[--s->elems];

	if (s->elems <= s->size / 4 && s->size > DEFAULT_SIZE)
	{
		s->size /= 2;
		safeRealloc((void**)&s->stack, s->size * sizeof(Poly));
	}

	return result;
}

void PSDestroy(PolyStack *s)
{
	for (size_t i = 0; i < s->elems; i++)
		PolyDestroy(&s->stack[i]);
	free(s->stack);
}