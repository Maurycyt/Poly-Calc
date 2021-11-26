/** @file
 * @brief Interfejs klasy stosu operacji na wielomianach.
 *
 * @author Maurycy Wojda
 * @date 2021
 */

#ifndef __POLY_STACK_H__
#define __POLY_STACK_H__

#include "poly.h"
#include <stdlib.h>

/**
 * Struktura przechowująca stos wielomianów.
 */
typedef struct PolyStack
{
	/**
	 * Liczba elementów na stosie.
	 */
	size_t elems;
	/**
	 * Liczba elementów, na które jest zaalokowana pamięć.
	 */
	size_t size;
	/**
	 * Tablica na której implementowany jest stos.
	 */
	Poly *stack;
} PolyStack;

/**
 * Tworzy pusty stos wielomianowy.
 * @return : nowy, pusty stos.
 */
PolyStack PSInit();

/**
 * Dodaje wielomian @f$p@f$ na wierzch stosu @f$s@f$.
 * @param[in] s : wskaźnik na stos @f$s@f$
 * @param[in] p : wielomian @f$p@f$
 */
void PSPush(PolyStack *s, const Poly p);

/**
 * Zwraca wielomian na pozycji @f$pos@f$ od wierzchu stosu @f$s@f$.
 * Nie obsługuje błędu `index out of bounds`.
 * @param[in] s : wskaźnik na stos @f$s@f$
 * @param[in] pos : pozycja od wierzchu stosu
 * @return : @f$pos@f$-ty element od wierzchu stosu @f$s@f$.
 */
Poly PSGet(const PolyStack *s, size_t pos);

/**
 * Zwraca wskaźnik na wielomian na pozycji @f$pos@f$ od wierzchu stosu @f$s@f$.
 * Nie obsługuje błędu `index out of bounds`.
 * @param[in] s : wskaźnik na stos @f$s@f$
 * @param[in] pos : pozycja od wierzchu stosu
 * @return : wskaźnik na @f$pos@f$-ty element od wierzchu stosu @f$s@f$.
 */
Poly *PSGetPtr(const PolyStack *s, size_t pos);

/**
 * Zwraca wielomian z wierzchu stosu @f$s@f$.
 * Nie zawiera obsługi błędu przy próbie czytania z pustego stosu.
 * @param[in] s : wskaźnik na stos @f$s@f$
 * @return : wielomian z wierzchu stosu
 */
Poly PSPeek(const PolyStack *s);

/**
 * Zwraca wskaźnik na wielomian z wierzchu stosu @f$s@f$.
 * Nie zawiera obsługi błędu przy próbie czytania z pustego stosu.
 * @param[in] s : wskaźnik na stos @f$s@f$
 * @return : wskaźnik na wielomian z wierzchu stosu
 */
Poly *PSPeekPtr(const PolyStack *s);

/**
 * Zwraca wielomian z wierzchu stosu @f$s@f$ i usuwa go ze stosu.
 * Nie zawiera obsługi błędu przy próbie czytania z pustego stosu.
 * @param[in] s : wskaźnik na stos @f$s@f$
 * @return : wielomian z wierzchu stosu
 */
Poly PSPop(PolyStack *s);

/**
 * Niszczy stos i wszystkie jego elementy, zwalniając pamięć.
 * @param[in] s : wskaźnik na stos do zniszczenia.
 */
void PSDestroy(PolyStack *s);

#endif /* __POLY_STACK_H__ */