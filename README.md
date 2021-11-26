# Polynomial Calculator
A project made for the Indiviual Programming Project course during the summer semester of the 2020/21 academic year.
It contains a library implementing the scarce polynomials and operations which can be performed on them, including composition, and a simple command interpreter is also included. 

## The project statment
We were tasked with creating a library implementing scarce multivariate polynomials. In detail greater detail, the definition of such a polynomial was recursive. If we name the variables of the polynomial x_0, x_1, x_2, and so on, then a scarce multivariate polynomial of variable x_0 is defined as the sum of monomials of the form px_0^n, where n is the exponent of the monomial and p is the coefficient of the monomial and also a polynomial of variable x_1. A polynomial of variable x_i is the sum of monomials with coefficients being polynomials of variable x_{i+1}. The recursion stops when the polynomial coefficients are constants.

Documentation in the form of Javadoc comments and Doxygen-generated documentation had to be created. Unfortunately, the provided documentation of the supplied header file was in Polish and I decided to stick to that very non-universal language. Additionally, the project had to be compilable in two versions, Release and Debug with the help of CMake.

### Part One
In the first part we were supplied with a header file, to which we were to provide the implementation. The header file poly.h included the definitions of the structures and declarations of functions meant for the construction of the polynomials, checking basic properties of the polynomials, adding, subtracting and multiplying the polynomials, freeing memory allocated for polynomials and other. We had to check for memory allocation failures.

### Part Two
In the second part we were tasked with creating a primitve user interface for a calculator on the polynomials. The polynomials were stored on a stack and the operations created new polynomials by taking polynomials from the top of the stack, performing the required operation and putting the result on the top of the stack. We had to check for improper input.

### Part Three
Finally, we had to add a couple of extra functionalities to both the library and the calculator, like polynomial composition and two new versions of a previously existing function.
