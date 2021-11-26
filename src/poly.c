/** @file
 * @brief Implementacja klasy wielomianów rzadkich wielu zmiennych.
 *
 * @author Maurycy Wojda
 * @date 2021
 */

#include "poly.h"
#include "safealloc.h"
#include <stdio.h>
#include <string.h>

#ifdef DOXYGEN
	#define UNUSED
#else
	#define UNUSED __attribute__((unused))
#endif

/**
 * Zwraca większy wykładnik wielomianowy.
 * @param[in] a : pierwszy wykładnik
 * @param[in] b : drugi wykładnik
 * @return @f$\max(a,b)@f$
 */
static poly_exp_t ExpMax(poly_exp_t a, poly_exp_t b)
{
	return a > b ? a : b;
}

UNUSED
/**
 * Zwraca mniejszy wykładnik wielomianowy.
 * @param[in] a : pierwszy wykładnik
 * @param[in] b : drugi wykładnik
 * @return @f$\min(a,b)@f$
 */
static poly_exp_t ExpMin(poly_exp_t a, poly_exp_t b)
{
	return a < b ? a : b;
}

/**
 * Potęguje współczynnik wielomianowy.
 * @param[in] a : współczynnik
 * @param[in] b : potęga
 * @return @f$a^b@f$
 */
static poly_coeff_t CoeffExp(poly_coeff_t a, poly_exp_t b)
{
	assert(b >= 0);
	poly_coeff_t result = 1;
	while (b)
	{
		if (b&1)
			result *= a;
		a *= a;
		b /= 2;
	}
	return result;
}

/**
 * Porównuje jednomiany na podstawie wykładników.
 * @param[in] MonoA : wskaźnik na pierwszy jednomian
 * @param[in] MonoB : wskaźnik na drugi jednomian
 * @return znormalizowana różnica wykładników jednomianów @f$\in\{-1,0,1\}@f$
 */
static inline int MonoExpCompare(const void *MonoA, const void *MonoB)
{
	assert(MonoA != NULL && MonoB != NULL);
	int diff = (((Mono*)MonoA)->exp - ((Mono*)MonoB)->exp);
	return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
}

void PolyDestroy(Poly *p)
{
	if (p != NULL)
	{
		if (!PolyIsCoeff(p))
			for (size_t i = 0; i < p->size; i++)
				MonoDestroy(&p->arr[i]);
		free(p->arr);
		p->arr = NULL;
	}
}

UNUSED
/*
UNUSED ponieważ ta funkcja jest wykorzystywana
tylko w asercjach i gcc krzyczy dla Release
*/
/**
 * Sprawdza czy wielomian spełnia wymagany standard malejących wykładników.
 * Standard wymaga aby każda funkcja, która zwraca strukturę Mono albo Poly
 * miała w tej strukturze w każdej tablicy `arr` każdej struktury `Poly`
 * posortowane ściśle malejąco po wykładnikach elementy.
 * @param[in] p : wielomian
 * @return `true` jeśli wielomian jest posortowany, `false` w przeciwnym przypadku
 */
static bool PolyIsSorted(const Poly *p)
{
	assert(p != NULL);
	if (PolyIsCoeff(p))
		return true;
	bool result = PolyIsSorted(&p->arr[0].p);
	for (size_t i = 1; i < p->size; i++)
	{
		result &= p->arr[i - 1].exp < p->arr[i].exp;
		result &= PolyIsSorted(&p->arr[i].p);
	}
	return result;
}

Poly PolyClone(const Poly *p)
{
	assert(p != NULL);
	if (PolyIsCoeff(p))
		return (Poly){.coeff = p->coeff, .arr = NULL};

	Poly result;
	result.size = p->size;
	result.arr = safeMalloc(result.size * sizeof(Mono));
	for (size_t i = 0; i < result.size; i++)
		result.arr[i] = MonoClone(&p->arr[i]);

	assert(PolyIsSorted(&result));
	return result;
}

/**
 * Dodaje dwa wielomiany, z których jeden jest współczynnikiem.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] q : wielomian @f$q@f$
 * @return @f$p + q@f$
 */
static Poly PolyAddCoeff(const Poly *p, const Poly *q)
{
	assert(p != NULL && q != NULL);
	assert(PolyIsCoeff(p) || PolyIsCoeff(q));

	Poly result;

	if (PolyIsCoeff(p) && PolyIsCoeff(q)) // Oba są współczynnikami
	{
		result = PolyFromCoeff(p->coeff + q->coeff);
	}
	else if (PolyIsCoeff(p) && !PolyIsCoeff(q)) // Pierwszy jest współczynnikiem -> zamiana
	{
		result = PolyAddCoeff(q, p);
	}
	else // Drugi jest współczynnikiem -> przypadkologia
	{
		if (PolyIsZero(q)) // Współczynnik zerowy -> zwróć klona
		{
			result = PolyClone(p);
		}
		else if (p->arr[0].exp == 0) // Wielomian zawiera stałą -> dodaj stałe
		{
			Poly atExpZero = PolyAdd(&p->arr[0].p, q);
			if (PolyIsZero(&atExpZero)) // Dodanie stałej do współczynnika daje 0
			{
				result.size = p->size - 1;
				result.arr = safeMalloc(result.size * sizeof(Mono));
				for (size_t i = 1; i < p->size; i++)
					result.arr[i-1] = MonoClone(&p->arr[i]);
			}
			else // Dodanie stałej do współczynnika daje niezerową stałą
			{
				result.size = p->size;
				result.arr = safeMalloc(result.size * sizeof(Mono));
				for (size_t i = 1; i < p->size; i++)
					result.arr[i] = MonoClone(&p->arr[i]);
				result.arr[0].p = atExpZero;
				result.arr[0].exp = 0;
			}
		}
		else // Wielomian nie zawiera stałej -> rozszerz tablicę o stałą
		{
			result.size = p->size + 1;
			result.arr = safeMalloc(result.size * sizeof(Mono));
			for (size_t i = 0; i < p->size; i++)
				result.arr[i + 1] = MonoClone(&p->arr[i]);
			result.arr[0] = MonoFromPoly(q, 0);
		}
	}

	assert(PolyIsSorted(&result));
	return result;
}

/**
 * Dodaje dwa wielomiany, oba nie są współczynnikami.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] q : wielomian @f$q@f$
 * @return @f$p + q@f$
 */
static Poly PolyAddNonCoeffs(const Poly *p, const Poly *q)
{
	assert(p != NULL && q != NULL);

	Poly result;

	result.size = p->size + q->size;
	result.arr = safeMalloc(result.size * sizeof(Mono));
	size_t pi = 0, qi = 0, resi = 0;

	// Pętla wypełniająca tablicę result.arr sumami jednomianów z odpowiednimi wykładnikami.
	while (pi < p->size || qi < q->size)
	{
		if (pi == p->size)
		{
			result.arr[resi++] = MonoClone(&q->arr[qi++]);
		}
		else if (qi == q->size)
		{
			result.arr[resi++] = MonoClone(&p->arr[pi++]);
		}
		else
		{
			if (p->arr[pi].exp < q->arr[qi].exp)
			{
				result.arr[resi++] = MonoClone(&p->arr[pi++]);
			}
			else if (p->arr[pi].exp > q->arr[qi].exp)
			{
				result.arr[resi++] = MonoClone(&q->arr[qi++]);
			}
			else
			{
				Poly polySum = PolyAdd(&p->arr[pi++].p, &q->arr[qi++].p);
				if (!PolyIsZero(&polySum))
					result.arr[resi++] = MonoFromPoly(&polySum, p->arr[pi - 1].exp);
			}
		}
	}

	if (result.size != resi)
	{
		result.size = resi;
		safeRealloc((void**)&result.arr, result.size * sizeof(Mono));
	}

	assert(PolyIsSorted(&result));
	return result;
}

/*
Wyjaśnienie implementacji:
Jeżeli p i q są współczynnikami, to result jest oczywisty.
Jeżeli tylko p jest współczynnikiem, to zamiana miejscami.
Jeżeli tylko q jest współczynnikiem, to klonuję jednomiany
	z wielomianu p i dodaję nowy jednomian odpowiadający
	wielomianowi q ręcznie wykonując odpowiednie obliczenia,
	rozbijając na przypadki gdy w p występuje stała i nie.
Jeżeli zarówno p jak i q są wielomianami, to tworzę tablice
	w której zapisuję resulti dodawania współczynników przy
	kolejnych wykładnikach pomijając te, które się zerują.
Z założenia, PolyAddMonos operuje na wielomianach mniejszej
	liczby zmiennych, więc rekurencja się kiedyś skończy.
*/
Poly PolyAdd(const Poly *p, const Poly *q)
{
	assert(p != NULL && q != NULL);

	Poly result;

	if (PolyIsCoeff(p) || PolyIsCoeff(q))
		result = PolyAddCoeff(p, q);
	else
		result = PolyAddNonCoeffs(p, q);

	// Sprawdzenie, czy wielomian jest samym współczynnikiem
	// przy jedynym, zerowym wykładniku -> uproszczenie
	if (result.arr != NULL && result.size == 1 && 
	    result.arr[0].exp == 0 && PolyIsCoeff(&result.arr[0].p))
	{
		Mono *toFree = result.arr;
		result = result.arr[0].p;
		free(toFree);
	}

	assert(PolyIsSorted(&result));
	return result;
}

/*
Wyjaśnienie implementacji:
Jeżeli tablica jest pusta, to result oczywiście zerowy.
W przeciwnym przypadku sortuję tablicę po wykładnikach.
W zmiennej PolySum akumuluje sumę wielomianów przy
	jednakowych wykładnikach. Gdy wykrywam zmianę wykładnika
	na inny, to zapisuję uzyskaną sumę wielomianów w kolejnym
	polu tablicy, które wiem, że już nie będzie używane, ale
	tylko, jeżeli uzyskany wielomian nie jest zerowy.
Gdy skończą się jednomiany zmniejszam, o ile można, rozmiar
	tablicy, z której później tworzę wielomian, lecz najpierw,
	trzeba dopilnować kilku szczegółów:
Po pierwsze, jeżeli uzyskana tablica jest pusta, to znaczy,
	że wszystkie jednomiany się wyzerowały.
Po drugie, jeżeli jedynym wykładnikiem w resultowej tablicy
	jest wykładnik zerowy, to wielomian jest tożsamościowo
	równy odpowiedniemu współczynnikowi. Należy wtedy tak
	go przedstawiać w pamięci.
Jeżeli żaden z powyższych warunków nie zachodzi, to result
	jest tworzony wprost z uzyskanej tablicy.
*/
Poly PolyOwnMonos(size_t count, Mono *monos)
{
	if (count == 0 || monos == NULL)
		return PolyZero();

	qsort(monos, count, sizeof(Mono), MonoExpCompare);
	size_t newMonoIndex = 0;
	Poly polySum = monos[0].p, newPoly;

	for (size_t i = 1; i < count; i++)
	{
		if (monos[i - 1].exp == monos[i].exp)
		{
			newPoly = PolyAdd(&polySum, &monos[i].p);
			PolyDestroy(&polySum);
			MonoDestroy(&monos[i]);
			polySum = newPoly;
		}
		else
		{
			if (!PolyIsZero(&polySum))
				monos[newMonoIndex++] = MonoFromPoly(&polySum, monos[i - 1].exp);
			polySum = monos[i].p;
		}
	}
	if (!PolyIsZero(&polySum))
		monos[newMonoIndex++] = MonoFromPoly(&polySum, monos[count - 1].exp);

	safeRealloc((void**)&monos, newMonoIndex * sizeof(Mono));

	Poly result;
	if (newMonoIndex == 0)
	{
		result = PolyZero();
		free(monos);
	}
	else if (newMonoIndex == 1 && monos[0].exp == 0 && PolyIsCoeff(&monos[0].p))
	{
		result = monos[0].p;
		free(monos);
	}
	else
	{
		result = (Poly){.size = newMonoIndex, .arr = monos};
	}

	assert(PolyIsSorted(&result));
	return result;
}

Poly PolyAddMonos(size_t count, const Mono monosIn[])
{
	if (count == 0 || monosIn == NULL)
		return PolyZero();

	Mono *monos = safeMalloc(count * sizeof(Mono));
	memcpy(monos, monosIn, count * sizeof(Mono));
	return PolyOwnMonos(count, monos);
}

Poly PolyCloneMonos(size_t count, const Mono monosIn[])
{
	if (count == 0 || monosIn == NULL)
		return PolyZero();

	Mono *monos = safeMalloc(count * sizeof(Mono));
	for (size_t i = 0; i < count; i++)
		monos[i] = MonoClone(&monosIn[i]);
	return PolyOwnMonos(count, monos);
}

/*
Wyjaśnienie implementacji:
Jeżeli p i q są współczynnikami, to result jest oczywisty.
Jeżeli tylko p jest współczynnikiem, to zamiana miejscami.
Jeżeli tylko q jest współczynnikiem, to tworzę nową tablicę
	jednomianów, przemnażając każdy przez q, potem zwracam result.
Jeżeli zarówno p jak i q są wielomianami, to tworzę nową
	tablicę jednomianów, przemnażając każdy jednomian z p z każdym
	jednomianem z q, dodając wykładniki, potem zwracam result.
*/
Poly PolyMul(const Poly *p, const Poly *q)
{
	assert(p != NULL && q != NULL);
	Poly result;

	if (PolyIsCoeff(p) && PolyIsCoeff(q))
	{
		result = PolyFromCoeff(p->coeff * q->coeff);
	}
	else if (PolyIsCoeff(p) && !PolyIsCoeff(q))
	{
		result = PolyMul(q, p);
	}
	else if (PolyIsCoeff(q))
	{
		Mono *monos = safeMalloc((p->size) * sizeof(Mono));

		for(size_t i = 0; i < p->size; i++)
		{
			monos[i].p = PolyMul(&p->arr[i].p, q);
			monos[i].exp = p->arr[i].exp;
		}

		result = PolyOwnMonos(p->size, monos);
	}
	else
	{
		Mono *monos = safeMalloc((p->size * q->size) * sizeof(Mono));

		for (size_t i = 0; i < p->size; i++)
			for (size_t j = 0; j < q->size; j++)
			{
				monos[i * q->size + j].p = PolyMul(&p->arr[i].p, &q->arr[j].p);
				monos[i * q->size + j].exp = p->arr[i].exp + q->arr[j].exp;
			}

		result = PolyOwnMonos(p->size * q->size, monos);
	}

	assert(PolyIsSorted(&result));
	return result;
}

void PolyMulByCoeffInPlace(Poly *p, poly_coeff_t c)
{
	if (PolyIsCoeff(p))
		p->coeff *= c;
	else
		for (size_t i = 0; i < p->size; i++)
			PolyMulByCoeffInPlace(&p->arr[i].p, c);
	assert(PolyIsSorted(p));
}

Poly PolyExp(const Poly *p, poly_exp_t e)
{
	Poly multiplier = PolyClone(p);
	Poly result = PolyFromCoeff(1);
	Poly temp;

	while(e)
	{
		if (e&1)
		{
			temp = PolyMul(&result, &multiplier);
			PolyDestroy(&result);
			result = temp;
		}
		temp = PolyMul(&multiplier, &multiplier);
		PolyDestroy(&multiplier);
		multiplier = temp;
		e /= 2;
	}

	PolyDestroy(&multiplier);
	assert(PolyIsSorted(&result));
	return result;
}

/*
Wyjaśnienie implementacji:
Jeżeli p jest współczynnikiem, to result jest oczywisty.
W przeciwnym wypadku tworzę płytką kopię tablicy jednomianów
	i każdy z nich zastępuję jego nowo stworzoną przeciwnością.
	Stanowi to oczywiście tablicę jednomianów w wielomianie
	przeciwnym, więc zwracam result.
*/
Poly PolyNeg(const Poly *p)
{
	assert(p != NULL);
	if (PolyIsCoeff(p))
		return PolyFromCoeff(-p->coeff);

	Poly result;
	result.size = p->size;
	result.arr = (Mono*)safeMalloc(p->size * sizeof(Mono));
	memcpy(result.arr, p->arr, p->size * sizeof(Mono));

	for (size_t i = 0; i < result.size; i++)
		result.arr[i].p = PolyNeg(&result.arr[i].p);

	assert(PolyIsSorted(&result));
	return result;
}

void PolyNegInPlace(Poly *p)
{
	assert(p != NULL);
	if (PolyIsCoeff(p))
		return (void)(p->coeff = -p->coeff);

	for (size_t i = 0; i < p->size; i++)
		PolyNegInPlace(&p->arr[i].p);

	assert(PolyIsSorted(p));
}

Poly PolySub(const Poly *p, const Poly *q)
{
	Poly qNeg = PolyNeg(q);
	Poly result = PolyAdd(p, &qNeg);
	PolyDestroy(&qNeg);

	assert(PolyIsSorted(&result));
	return result;
}

poly_exp_t PolyDegBy(const Poly *p, size_t var_idx)
{
	assert(p != NULL);
	if (PolyIsZero(p))
		return -1;
	poly_exp_t result = 0;
	if (!PolyIsCoeff(p))
		for (size_t i = 0; i < p->size; i++)
			result = ExpMax(result, var_idx == 0 ? 
			                        p->arr[i].exp : 
			                        PolyDegBy(&p->arr[i].p, var_idx - 1));
	return result;
}

poly_exp_t PolyDeg(const Poly *p)
{
	assert(p != NULL);
	if (PolyIsZero(p))
		return -1;
	poly_exp_t result = 0;
	if (p->arr != NULL)
		for (size_t i = 0; i < p->size; i++)
			result = ExpMax(result, PolyDeg(&p->arr[i].p) + p->arr[i].exp);
	return result;
}

/*
Wyjaśnienie implementacji:
Ze względu na to, że każdy wielomian tworzony przez moje
	funkcje spełnia standard, który zakłada:
1. Posortowaną malejąco po wykłądnikach tablicę `arr`
2. Utożsamienie wielomianów, w których wykładnik przy
	jedynym jednomianie jest zerowy z wielomianem współczynnikowym
To dwa wielomiany są równe wtedy i tylko wtedy, gdy albo są
	równymi sobie współczynnikami, albo mają tablice jednomianów
	tej samej długości i każdy jednomian z p jest równy odpowiedniemu
	jednomianowi z q ze względu na wykładnik i współczynnik wielomianowy.
*/
bool PolyIsEq(const Poly *p, const Poly *q)
{
	assert(p != NULL && q != NULL);
	assert(PolyIsSorted(p));
	assert(PolyIsSorted(q));

	if (PolyIsCoeff(p) && PolyIsCoeff(q))
		return p->coeff == q->coeff;
	if ((PolyIsCoeff(p) ^ PolyIsCoeff(q)) || p->size != q->size)
		return false;
	bool result = true;
	for (size_t i = 0; i < p->size && result; i++)
	{
		result &= p->arr[i].exp == q->arr[i].exp;
		result &= PolyIsEq(&p->arr[i].p, &q->arr[i].p);
	}
	return result;
}

/*
Wyjaśnienie implementacji:
Dla każdego jednomianu tworzę wielomian polyProd równy iloczynowi
	współczynnika i odpowiedniej potęgi podanej wartości x.
Dodaję otrzymane polyProd do wielomianu polySum, który stanie
	się resultiem, jednak muszę pamiętać o zniszczeniu poprzedniego
	polySum, stąd używam tempPoly.
*/
Poly PolyAt(const Poly *p, poly_coeff_t x)
{
	assert(p != NULL);
	if (PolyIsCoeff(p))
		return PolyClone(p);

	Poly powerPoly, polyProd, tempPoly, polySum = PolyZero();
	for (size_t i = 0; i < p->size; i++)
	{
		powerPoly = PolyFromCoeff(CoeffExp(x, p->arr[i].exp));
		polyProd = PolyMul(&p->arr[i].p, &powerPoly);
		tempPoly = PolyAdd(&polySum, &polyProd);
		PolyDestroy(&polyProd);
		PolyDestroy(&polySum);
		polySum = tempPoly;
	}

	assert(PolyIsSorted(&polySum));
	return polySum;
}

Poly PolyAtInPlace(Poly *p, poly_coeff_t x)
{
	if (PolyIsCoeff(p))
		return *p;

	for (size_t i = 0; i < p->size; i++)
		PolyMulByCoeffInPlace(&p->arr[i].p, CoeffExp(x, p->arr[i].exp));
	Poly result = p->arr[0].p, polySum;
	for (size_t i = 1; i < p->size; i++)
	{
		polySum = PolyAdd(&result, &p->arr[i].p);
		PolyDestroy(&result);
		//PolyDestroy(&p->arr[i].p);
		result = polySum;
	}

	return result;
}

Poly PolyCompose(const Poly *p, size_t k, const Poly q[])
{
	if (PolyIsCoeff(p))
		return *p;

	Poly result = PolyZero(), compPoly, expPoly, mulPoly, addPoly;
	for (size_t i = 0; i < p->size; i++)
	{
		if (p->arr[i].exp != 0 && k == 0)
			break;

		compPoly = PolyCompose(&p->arr[i].p, k == 0 ? 0 : k - 1, q + 1);
		if (p->arr[i].exp == 0)
			expPoly = PolyFromCoeff(1);
		else
			expPoly = PolyExp(q, p->arr[i].exp);
		mulPoly = PolyMul(&compPoly, &expPoly);
		PolyDestroy(&compPoly);
		PolyDestroy(&expPoly);

		addPoly = PolyAdd(&result, &mulPoly);
		PolyDestroy(&result);
		PolyDestroy(&mulPoly);
		result = addPoly;
	}

	assert(PolyIsSorted(&result));
	return result;
}

void PolyPrint(const Poly *p)
{
	assert(p != NULL);

	if (PolyIsCoeff(p))
	{
		printf("%ld", p->coeff);
		return;
	}

	printf("(");
	PolyPrint(&p->arr[0].p);
	printf(",%d)", p->arr[0].exp);

	for (size_t i = 1; i < p->size; i++)
	{
		printf("+(");
		PolyPrint(&p->arr[i].p);
		printf(",%d)", p->arr[i].exp);
	}
}

void PolyPrintln(const Poly *p)
{
	PolyPrint(p);
	printf("\n");
}