/** @file
 * @brief Implementacja modułu interfejsu użytkownika kalkulatora wielomianów.
 *
 * @author Maurycy Wojda
 * @date 2021
 */

#include "polyui.h"
#include "polystack.h"
#include "safealloc.h"
#include <stdbool.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/**
 * Typ wyliczeniowy do obsługi błędów.
 */
typedef enum ErrorType
{
	NO_ERROR,
	STACK_UNDERFLOW,
	WRONG_POLY,
	WRONG_COMMAND,
	WRONG_ARGUMENT
} ErrorType;

/**
 * Liczba rodzajów błędów.
 */
#define NO_OF_ERRORS WRONG_ARGUMENT

/**
 * Tablica komunikatów o błędach w przypadku
 * błędów nie-argumentowych.
 */
static const char *errorMessages[NO_OF_ERRORS] =
{
	"",
	"STACK UNDERFLOW",
	"WRONG POLY",
	"WRONG COMMAND"
};

/**
 * Struktura przechowująca kontekst wywołania polecenia.
 */
typedef struct ExectionContext
{
	/**
	 * Wskaźnik na stos wielomianowy, na którym polecenie ma być wywołane.
	 */
	PolyStack *stack;
	/**
	 * Opcjonalny argument polecenia.
	 */
	long double arg;
	/**
	 * Wskaźnik na flagę błędu.
	 */
	ErrorType *errType;
} ExecutionContext;

/**
 * Struktura służąca trzymaniu wszystkich informacji, które są 
 * standardowo potrzebne wykonaniu polecenia na stosie wielomianów.
 */
typedef struct CommandInfo
{
	/**
	 * Nazwa polecenia, wpisywana na wejściu.
	 */
	const char *cmndName;
	/**
	 * Funkcja do wykonania na stosie przy obsłudze polecenia.
	 * param[in] context : kontekst wywołania polecenia
	 */
	void (*cmndFunc)(ExecutionContext context);
	/**
	 * Funkcja do czytania argumentu polecenia.
	 * Funkcja wywoływana w celu wczytania argumentu polecenia,
	 * lub NULL, w przypadku gdy polecenie nie przyjmuje argumentu.
	 * @param[in] firstChar : pierwszy znak czytanego wiersza
	 * @param[out] nextChar : wskaźnik na wskaźnik na znak 
	 * w wierszu występujący po ostatnim wczytanym znaku
	 * @param[out] errFlag : wskaźnik na flagę błędu
	 * @return argument polecenia jako dana typu long double
	 */
	long double (*readArgFunc)(char *firstChar, char **nextChar, bool *errFlag);
	/**
	 * Komunikat błedu do wypisanie, jeżeli argument będzie źle podany.
	 */
	const char *argErrMsg;
} CommandInfo;

/**
 * Funkcja do wykonania na stosie przy obsłudze polecenia ZERO.
 * param[in] context : kontekst wywołania polecenia
 */
static void executeZero(ExecutionContext context)
{
	PSPush(context.stack, PolyZero());
}

/**
 * Funkcja do wykonania na stosie przy obsłudze polecenia IS_COEFF.
 * param[in] context : kontekst wywołania polecenia
 */
static void executeIsCoeff(ExecutionContext context)
{
	if (context.stack->elems < 1)
		return (void)(*context.errType = STACK_UNDERFLOW);
	printf("%d\n", PolyIsCoeff(PSPeekPtr(context.stack)) ? 1 : 0);
}

/**
 * Funkcja do wykonania na stosie przy obsłudze polecenia IS_ZERO.
 * param[in] context : kontekst wywołania polecenia
 */
static void executeIsZero(ExecutionContext context)
{
	if (context.stack->elems < 1)
		return (void)(*context.errType = STACK_UNDERFLOW);
	printf("%d\n", PolyIsZero(PSPeekPtr(context.stack)) ? 1 : 0);
}

/**
 * Funkcja do wykonania na stosie przy obsłudze polecenia CLONE.
 * param[in] context : kontekst wywołania polecenia
 */
static void executeClone(ExecutionContext context)
{
	if (context.stack->elems < 1)
		return (void)(*context.errType = STACK_UNDERFLOW);
	PSPush(context.stack, PolyClone(PSPeekPtr(context.stack)));
}

/**
 * Funkcja do wykonania na stosie przy obsłudze polecenia ADD.
 * param[in] context : kontekst wywołania polecenia
 */
static void executeAdd(ExecutionContext context)
{
	if (context.stack->elems < 2)
		return (void)(*context.errType = STACK_UNDERFLOW);
	Poly p = PSPop(context.stack);
	Poly q = PSPop(context.stack);
	PSPush(context.stack, PolyAdd(&p, &q));
	PolyDestroy(&p);
	PolyDestroy(&q);
}

/**
 * Funkcja do wykonania na stosie przy obsłudze polecenia MUL.
 * param[in] context : kontekst wywołania polecenia
 */
static void executeMul(ExecutionContext context)
{
	if (context.stack->elems < 2)
		return (void)(*context.errType = STACK_UNDERFLOW);
	Poly p = PSPop(context.stack);
	Poly q = PSPop(context.stack);
	PSPush(context.stack, PolyMul(&p, &q));
	PolyDestroy(&p);
	PolyDestroy(&q);
}

/**
 * Funkcja do wykonania na stosie przy obsłudze polecenia NEG.
 * param[in] context : kontekst wywołania polecenia
 */
static void executeNeg(ExecutionContext context)
{
	if (context.stack->elems < 1)
		return (void)(*context.errType = STACK_UNDERFLOW);
	PolyNegInPlace(PSPeekPtr(context.stack));
}

/**
 * Funkcja do wykonania na stosie przy obsłudze polecenia SUB.
 * param[in] context : kontekst wywołania polecenia
 */
static void executeSub(ExecutionContext context)
{
	if (context.stack->elems < 2)
		return (void)(*context.errType = STACK_UNDERFLOW);
	Poly p = PSPop(context.stack);
	Poly q = PSPop(context.stack);
	PSPush(context.stack, PolySub(&p, &q));
	PolyDestroy(&p);
	PolyDestroy(&q);
}

/**
 * Funkcja do wykonania na stosie przy obsłudze polecenia IS_EQ.
 * param[in] context : kontekst wywołania polecenia
 */
static void executeIsEq(ExecutionContext context)
{
	if (context.stack->elems < 2)
		return (void)(*context.errType = STACK_UNDERFLOW);
	Poly *p = PSGetPtr(context.stack, 1);
	Poly *q = PSGetPtr(context.stack, 2);
	printf("%d\n", PolyIsEq(p, q) ? 1 : 0);
}

/**
 * Funkcja do wykonania na stosie przy obsłudze polecenia DEG.
 * param[in] context : kontekst wywołania polecenia
 */
static void executeDeg(ExecutionContext context)
{
	if (context.stack->elems < 1)
		return (void)(*context.errType = STACK_UNDERFLOW);
	printf("%d\n", PolyDeg(PSPeekPtr(context.stack)));
}

/**
 * Funkcja do wykonania na stosie przy obsłudze polecenia DEG_BY.
 * param[in] context : kontekst wywołania polecenia
 */
static void executeDegBy(ExecutionContext context)
{
	if (context.stack->elems < 1)
		return (void)(*context.errType = STACK_UNDERFLOW);
	printf("%d\n", PolyDegBy(PSPeekPtr(context.stack), (size_t)context.arg));
}

/**
 * Funkcja do wykonania na stosie przy obsłudze polecenia AT.
 * param[in] context : kontekst wywołania polecenia
 */
static void executeAt(ExecutionContext context)
{
	if (context.stack->elems < 1)
		return (void)(*context.errType = STACK_UNDERFLOW);
	Poly p = PSPop(context.stack);
	PSPush(context.stack, PolyAtInPlace(&p, (poly_coeff_t)context.arg));
}

/**
 * Funkcja do wykonania na stosie przy obsłudze polecenia PRINT.
 * param[in] context : kontekst wywołania polecenia
 */
static void executePrint(ExecutionContext context)
{
	if (context.stack->elems < 1)
		return (void)(*context.errType = STACK_UNDERFLOW);
	PolyPrintln(PSPeekPtr(context.stack));
}

/**
 * Funkcja do wykonania na stosie przy obsłudze polecenia POP.
 * param[in] context : kontekst wywołania polecenia
 */
static void executePop(ExecutionContext context)
{
	if (context.stack->elems < 1)
		return (void)(*context.errType = STACK_UNDERFLOW);
	PolyDestroy(PSPeekPtr(context.stack));
	PSPop(context.stack);
}

/**
 * Funkcja do wykonania na stosie przy obsłudze polecenia COMPOSE.
 * param[in] context : kontekst wywołania polecenia
 */
static void executeCompose(ExecutionContext context)
{
	size_t k = (size_t)context.arg;
	if (context.stack->elems <= k)
		return (void)(*context.errType = STACK_UNDERFLOW);

	Poly p = PSPop(context.stack);
	Poly *q = safeMalloc(k * sizeof(Poly));
	for (size_t i = 0; i < k; i++)
		q[k - i - 1] = PSPop(context.stack);

	Poly result = PolyCompose(&p, k, q);

	PolyDestroy(&p);
	for (size_t i = 0; i < k; i++)
		PolyDestroy(&q[i]);
	free(q);

	PSPush(context.stack, result);
}

/**
 * Statyczna flaga przechowująca informacje o tym, czy
 * koniec pliku na standardowym wejściu został osiągnięty.
 */
static bool EOF_FLAG = false;

bool checkEOF()
{
	return EOF_FLAG;
}

/**
 * Stwierdza, czy znak jest cyfrą.
 * @param[in] c : znak
 * @return : wynik
 */
static bool isDigit(char c)
{
	return ('0' <= c && c <= '9');
}

/**
 * Stwierdza, czy znak jest cyfrą lub znakiem minusa.
 * @param[in] c : znak
 * @return : wynik
 */
static bool isDigitMinus(char c)
{
	return (isDigit(c) || c == '-');
}

/**
 * Stwierdza, czy znak jest nieoczekiwanym białym znakiem.
 * @param[in] c : znak
 * @return : wynik
 */
static bool isBadWhite(char c)
{
	return (c == '\t' || c == '\r' || c == '\v' || c == '\f');
}

/**
 * Stwierdza, czy znak jest literą.
 * @param[in] c : znak
 * @return : wynik
 */
static bool isLetter(char c)
{
	return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

static long double readArgULongAsLDbl(char *firstChar, char **nextChar, bool *errFlag);

static long double readCoeffAsLDbl(char *firstChar, char **nextChar, bool *errFlag);

/**
 * Tablica struktur z informacjami na temat poleceń.
 * Tablica struktur z nazwami poleceń, funkcjami do
 * ich wywoływania, ograniczeniami na argumenty i
 * komunikatami błędu dla niepoprawnego argumetu.
 */
static CommandInfo const commandList[] =
{
	{"ZERO", executeZero, NULL, ""},
	{"SUB", executeSub, NULL, ""},
	{"PRINT", executePrint, NULL, ""},
	{"POP", executePop, NULL, ""},
	{"NEG", executeNeg, NULL, ""},
	{"MUL", executeMul, NULL, ""},
	{"IS_ZERO", executeIsZero, NULL, ""},
	{"IS_EQ", executeIsEq, NULL, ""},
	{"IS_COEFF", executeIsCoeff, NULL, ""},
	{"DEG_BY", executeDegBy, readArgULongAsLDbl, "DEG BY WRONG VARIABLE"},
	{"DEG", executeDeg, NULL, ""},
	{"COMPOSE", executeCompose, readArgULongAsLDbl, "COMPOSE WRONG PARAMETER"},
	{"CLONE", executeClone, NULL, ""},
	{"AT", executeAt, readCoeffAsLDbl, "AT WRONG VALUE"},
	{"ADD", executeAdd, NULL, ""}
};

/**
 * Liczba poleceń.
 */
#define NO_OF_COMMANDS (size_t)(sizeof(commandList)/sizeof(CommandInfo))

/**
 * Błędny numer polecenia.
 */
#define ERROR_COMMAND NO_OF_COMMANDS

/**
 * Tablica długości nazw poleceń.
 */
static size_t commandNameLengths[NO_OF_COMMANDS] = {};

void PolyUIInit()
{
	for (size_t i = 0; i < NO_OF_COMMANDS; i++)
		commandNameLengths[i] = strlen(commandList[i].cmndName);
}

/**
 * Zwraca stringa z jednym wierszem z wejścia.
 * Ustawia wartość wskazanej zmiennej na liczbę znaków w wierszu.
 * Ignoruje komentarze i podnosi wskazaną flagę jeżeli wczytany
 * wiersz jest komentarzem.
 * @param[out] charsRead : wskaźnik na zmienną, w której ma być
 * zapisana liczba znaków w wierszu.
 * @param[out] isComment : wskaźnik na flagę oznaczającą wczytanie komentarza.
 * @return : string z jednym wierszem z wejścia
 */
static char *readLine(size_t *charsRead, bool *isComment)
{
	*charsRead = 0;
	*isComment = false;
	size_t bufferSize = DEFAULT_SIZE;
	char *buffer, newChar = '\0';
	buffer = (char*)safeMalloc(bufferSize * sizeof(char));

	while (scanf("%c", &newChar) != -1 && newChar != '\n')
	{
		if (!*isComment)
		{
			if (*charsRead == bufferSize)
			{
				bufferSize *= 2;
				safeRealloc((void**)&buffer, bufferSize * sizeof(char));
			}

			buffer[(*charsRead)++] = newChar; // Fun fact: bez nawiasów jest problem.

			*isComment = (buffer[0] == '#');
		}
	}

	if (newChar != '\n')
		EOF_FLAG = true;

	safeRealloc((void**)&buffer, (*charsRead + 1) * sizeof(char));
	buffer[*charsRead] = '\0';
	return buffer;
}

/**
 * Zwraca stałą typu poly_coeff_t zapisaną w wierszu.
 * @param[in] firstChar : pierwszy znak czytanego wiersza
 * @param[out] nextChar : wskaźnik na wskaźnik na znak 
 * w wierszu występujący po ostatnim wczytanym znaku
 * @param[out] errFlag : wskaźnik na flagę błędu
 * @return : wczytana stała
 */
static poly_coeff_t readCoeff(char *firstChar, char **nextChar, bool *errFlag)
{
	errno = 0;
	poly_coeff_t result = strtol(firstChar, nextChar, 10);

	*errFlag = (errno != 0 || !isDigitMinus(*firstChar));

	return result;
}

/**
 * Zwraca stałą typu long double zapisaną w wierszu w formacie poly_coeff_t.
 * @param[in] firstChar : pierwszy znak czytanego wiersza
 * @param[out] nextChar : wskaźnik na wskaźnik na znak 
 * w wierszu występujący po ostatnim wczytanym znaku
 * @param[out] errFlag : wskaźnik na flagę błędu
 * @return : wczytana stała
 */
static long double readCoeffAsLDbl(char *firstChar, char **nextChar, bool *errFlag)
{
	return (long double)readCoeff(firstChar, nextChar, errFlag);
}

/**
 * Zwraca stałą typu poly_exp_t zapisaną w wierszu.
 * @param[in] firstChar : pierwszy znak czytanego wiersza
 * @param[out] nextChar : wskaźnik na wskaźnik na znak 
 * w wierszu występujący po ostatnim wczytanym znaku
 * @param[out] errFlag : wskaźnik na flagę błędu
 * @return : wczytana stała
 */
static poly_exp_t readExp(char *firstChar, char **nextChar, bool *errFlag)
{
	errno = 0;
	long result = strtol(firstChar, nextChar, 10);

	*errFlag = (errno != 0 || !isDigit(*firstChar) ||
	            POLY_EXP_T_MAX < result);

	return (poly_exp_t)result;
}

/**
 * Zwraca stałą typu unsigned long zapisaną w wierszu.
 * @param[in] firstChar : pierwszy znak czytanego wiersza
 * @param[out] nextChar : wskaźnik na wskaźnik na znak 
 * w wierszu występujący po ostatnim wczytanym znaku
 * @param[out] errFlag : wskaźnik na flagę błędu
 * @return : wczytana stała
 */
static unsigned long readArgULong(char *firstChar, char **nextChar, bool *errFlag)
{
	errno = 0;
	unsigned long result = strtoul(firstChar, nextChar, 10);

	*errFlag = (errno != 0 || !isDigit(*firstChar));

	return result;
}

/**
 * Zwraca stałą typu long double zapisaną w wierszu w formacie unsigned long.
 * @param[in] firstChar : pierwszy znak czytanego wiersza
 * @param[out] nextChar : wskaźnik na wskaźnik na znak 
 * w wierszu występujący po ostatnim wczytanym znaku
 * @param[out] errFlag : wskaźnik na flagę błędu
 * @return : wczytana stała
 */
static long double readArgULongAsLDbl(char *firstChar, char **nextChar, bool *errFlag)
{
	return (long double)readArgULong(firstChar, nextChar, errFlag);
}

static Poly readPoly(char *firstChar, char **nextChar, ErrorType *errType);

/**
 * Zwraca jednomian zapisany w wierszu.
 * @param[in] firstChar : pierwszy znak czytanego wiersza
 * @param[out] nextChar : wskaźnik na wskaźnik na znak 
 * w wierszu występujący po ostatnim wczytanym znaku
 * @param[out] errType : wskaźnik na flagę błędu
 * @return : wczytany jednomian
 */
static Mono readMono(char *firstChar, char **nextChar, ErrorType *errType)
{
	bool errFlag = false;
	Mono result = {.p = PolyZero(), .exp = 0};

	if (*(firstChar++) != '(')
	{
		*errType = WRONG_POLY;
		return result;
	}

	result.p = readPoly(firstChar, nextChar, errType);

	firstChar = *nextChar;
	if (*(firstChar++) != ',' || *errType != NO_ERROR)
	{
		*errType = WRONG_POLY;
		return result;
	}

	result.exp = readExp(firstChar, nextChar, &errFlag);

	if (**nextChar != ')' || errFlag)
		*errType = WRONG_POLY;
	else
		(*nextChar)++;

	return result;
}

/**
 * Zwraca wielomian zapisany w wierszu.
 * @param[in] firstChar : pierwszy znak czytanego wiersza
 * @param[out] nextChar : wskaźnik na wskaźnik na znak 
 * w wierszu występujący po ostatnim wczytanym znaku
 * @param[out] errType : wskaźnik na flagę błędu
 * @return : wczytany wielomian
 */
static Poly readPoly(char *firstChar, char **nextChar, ErrorType *errType)
{
	Poly result = PolyZero();

	if (isDigitMinus(*firstChar))
	{
		bool errFlag = false;
		result.arr = NULL;
		result.coeff = readCoeff(firstChar, nextChar, &errFlag);
		
		if (errFlag)
			*errType = WRONG_POLY;
	}
	else
	{
		size_t noOfMonos = 0, bufferSize = DEFAULT_SIZE;
		Mono *monoBuffer = (Mono*)safeMalloc(bufferSize * sizeof(Mono));

		monoBuffer[noOfMonos++] = readMono(firstChar, nextChar, errType);

		while (**nextChar == '+' && *errType == NO_ERROR)
		{
			firstChar = *nextChar + 1;
			if (noOfMonos == bufferSize)
			{
				bufferSize *= 2;
				safeRealloc((void**)&monoBuffer, bufferSize * sizeof(Mono));
			}
			monoBuffer[noOfMonos++] = readMono(firstChar, nextChar, errType);
		}

		safeRealloc((void**)&monoBuffer, noOfMonos * sizeof(Mono));

		result = PolyAddMonos(noOfMonos, monoBuffer);
		free(monoBuffer);
	}

	return result;
}

/**
 * Sprawdza czy string jest prefiksem innego stringa.
 * @param[in] prefix : string kandydujący na bycie prefiksem
 * @param[in] line : string, którego prefiksem ma być `prefiks`
 * @return : `true`, jeżeli `prefiks` jest prefiksem `line`a,
 * albo `false` w przeciwnym przypadku.
 */
static bool isPrefix(const char *prefix, const char *line)
{
	while (*prefix != '\0' && *line != '\0')
	{
		if (*prefix != *line)
			return false;
		prefix++;
		line++;
	}

	return (*prefix == '\0' || *line != '\0');
}

/**
 * Rozpoznaje rodzaj polecenia na podstawie stringa.
 * @param[in] line : string, którego poleceni ma być rozpoznane
 * @return : numer operacji do przeprowadzenia na stosie
 * odpowiadający wykrytemu poleceniu, lub wartość ERROR_COMMAND
 * w przypadku, gdy żadne polecenie nie pasuje.
 */
static size_t detectCommand(const char *line)
{
	size_t op;
	for (op = 0; op < ERROR_COMMAND; op++)
		if (isPrefix(commandList[op].cmndName, line))
			break;
	return op;
}

/**
 * Wczytuje wielomian z wejścia standardowego i wrzuca na stos,
 * jeżeli wczytanie odbędzie się bez błędu.
 * param[in] s : wskaźnik na stos
 * param[in] line : string z wejścia
 * param[in] noOfChars : liczba znaków w lineu
 * param[out] errType : wskaźnik na flagę błędu
 */
static void handlePoly(PolyStack *s, char *line,
	                          size_t noOfChars, ErrorType *errType)
{
	char *nextChar = line;
	Poly polynomial = readPoly(line, &nextChar, errType);

	if (nextChar != line + noOfChars)
		*errType = WRONG_POLY;

	if (*errType == NO_ERROR)
		PSPush(s, polynomial);
	else
		PolyDestroy(&polynomial);
}

/**
 * Wczytuje polecenie z wejścia standardowego i je parse'uje oraz
 * wykonuje, jeżeli wczytanie odbędzie się bez błędu. Zwraca również
 * rodzaj wykrytej operacji i rodzaj błędu w celu obsługi błędów.
 * param[in] s : wskaźnik na stos
 * param[in] line : string z wejścia
 * param[in] noOfChars : liczba znaków w wierszu
 * param[out] errType : wskaźnik na flagę błędu
 * param[out] op : wskaźnik na rodzaj wykrytej operacji
 */
static void handleCommand(PolyStack *s, char *line, size_t noOfChars,
	                        ErrorType *errType, size_t *op)
{
	*op = detectCommand(line);

	if (*op == ERROR_COMMAND)
	{
		*errType = WRONG_COMMAND;
		return;
	}

	char *firstChar = line + commandNameLengths[*op];
	char *nextChar = firstChar;
	bool errFlag = false;
	ExecutionContext context = {s, 0, errType};

	if (commandList[*op].readArgFunc != NULL)
	{
		if (isBadWhite(*firstChar) || commandNameLengths[*op] == noOfChars)
		{
			*errType = WRONG_ARGUMENT;
			return;
		}
		if (*firstChar++ != ' ')
		{
			*errType = WRONG_COMMAND;
			return;
		}

		context.arg = commandList[*op].readArgFunc(firstChar, &nextChar, &errFlag);

		if (errFlag || nextChar != line + noOfChars)
			*errType = WRONG_ARGUMENT;
	}
	else
	{
		if (*firstChar != '\0' || nextChar != line + noOfChars)
			*errType = WRONG_COMMAND;
	}

	if (*errType == NO_ERROR)
		commandList[*op].cmndFunc(context);
}

/**
 * Wypisuje komunikat o błędzie.
 * param[in] errType : rodzaj błędu, który został wykryty
 * param[in] op : numer polecenia, którrgo błąd dotyczy
 * param[in] lineNumber : numer wiersza, którego błąd dotyczy
 */
static void printError(ErrorType errType, size_t op, int lineNumber)
{
	switch(errType)
	{
		case WRONG_ARGUMENT:
			fprintf(stderr, "ERROR %d %s\n", lineNumber, commandList[op].argErrMsg);
			break;
		default:
			fprintf(stderr, "ERROR %d %s\n", lineNumber, errorMessages[errType]);
			break;
	}
}

/*
Wyjaśnienie implementacji:
Funkcja wczytuje wiersz, jeżeli jest pusty lub jest komentarzem,
	to zwalnia pamięć i kończy działanie.
Jeżeli wiersz okazuje się zaczynać literą, to wykrywany jest rodzaj
	polecenia o który chodzi. Jeżeli polecenie jest inne niż DEG_BY lub AT,
	to jest ono wykonywane, zwracając uwagę jedynie na Stack Underflow i prosty
	warunek końca linea.
Jeżeli polecenie przyjmuje argument, to uważnie są sprawdzane kolejne znaki
	w celu zwrócenia, jeśli wystąpi, odpowiedniego komunikatu o błędzie. Po
	przeczytaniu argumentu sprawdzane jest czy
		1) nie był spoza przedziału (sprawdzanie errno i pierwszego znaku)
		2) był w ogóle wczytany
		3) nic nie zostało po argumencie
Jeżeli pierwszym znakiem nie jest litera, to czytany jest wielomian
	i dodawany na stos, jeżeli nie wystąpił błąd.
*/
void handleLine(PolyStack *s)
{
	static int lineNumber = 0;

	lineNumber++;

	bool isComment;
	ErrorType errType = NO_ERROR;
	size_t charsRead, op = ERROR_COMMAND;
	char *line = readLine(&charsRead, &isComment);

	if (charsRead == 0 || isComment)
	{
		free(line);
		return;
	}

	if (isLetter(line[0]))
		handleCommand(s, line, charsRead, &errType, &op);
	else
		handlePoly(s, line, charsRead, &errType);

	if (errType != NO_ERROR)
		printError(errType, op, lineNumber);

	free(line);
}