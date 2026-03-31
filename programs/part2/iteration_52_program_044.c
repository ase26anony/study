/* test_expr_error.c
 * This program contains various syntactically valid but semantically invalid
 * expressions designed to trigger the error_mark_node return path in expr.cc.
 * The program will fail to compile during semantic analysis.
 */

#include <stdio.h>

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) (a + b)
#define BAD_BITWISE(a, b) (a | b)

/* Token concatenation to create questionable identifier */
#define CONCAT(a, b) a ## b
int CONCAT(foo, bar); /* This is valid but unused */

/* Invalid global initializer (excess initializers) */
int global_arr[3] = {1, 2, 3, 4};  /* Line 15: excess initializers */

/* Function prototype */
void test_function(int x);

int main(void) {
    /* 1. Valid statement for context */
    int valid = 42;
    printf("Starting...\n");
    
    /* 2. Type mismatch in binary operation (int + string literal) */
    int x = 5 + "string";  /* Line 24: invalid operands to binary + */
    
    /* 3. Invalid operand combination: bitwise OR on floating-point */
    double d = 3.14 | 2.5;  /* Line 27: invalid operands to binary | */
    
    /* 4. Using address-of operator on constant */
    int* p = &42;  /* Line 30: lvalue required as unary & operand */
    
    /* 5. Macro-generated type mismatch */
    float f = BAD_ADD(valid, "text");  /* Line 33: int + string via macro */
    
    /* 6. Statement expression misuse (GCC extension) */
    int se = ({ int a; a; });  /* Line 36: statement expression with no value */
    
    /* 7. Invalid initializer for scalar */
    int z = {5, 6};  /* Line 39: scalar initialized with multiple values */
    
    /* 8. Misuse of __builtin function */
    int bits = __builtin_ctz("hello");  /* Line 42: pointer to int for ctz */
    
    /* 9. Control flow with invalid condition */
    if (main + 1) {  /* Line 45: function pointer arithmetic in condition */
        printf("Never reached\n");
    }
    
    /* 10. Return statement with invalid expression */
    return &valid + "invalid";  /* Line 49: pointer + string literal */
}

/* Helper function with scope issues */
void test_function(int x) {
    /* 11. Use of potentially undeclared identifier (out of order in C89/C99) */
    printf("%d\n", y);  /* Line 55: y undeclared */
    int y = 10;  /* Declaration after use */
    
    /* 12. Invalid array assignment */
    int local_arr[2];
    local_arr = {1, 2};  /* Line 60: invalid assignment */
    
    /* 13. Complex expression with multiple issues */
    int result = (x * 2) & 3.14;  /* Line 63: int & double */
}

/* Global variable with invalid initializer */
struct S {
    int a;
    float b;
};

struct S s1 = {1, 2.0, 3};  /* Line 71: excess initializers for struct */

/* Attempt to take address of a literal in global context */
int* global_ptr = &"constant";  /* Line 74: address of string literal (still invalid as initializer) */
