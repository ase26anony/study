/* test_error_mark_node.c
 * This program contains various syntactically valid but semantically invalid
 * expressions designed to trigger the error_mark_node return path in expr.cc.
 * Compile with: gcc -fsyntax-only -O0 test_error_mark_node.c
 */

#include <stdio.h>

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) (a + b)
#define BAD_BITWISE(a, b) (a | b)

/* Invalid macro with token concatenation */
#define CONCAT_INVALID(x, y) x##y##_undefined

/* Global scope errors */
int* global_ptr = &42;  /* Error: address of constant */

int main() {
    /* 1. Valid code for context */
    int valid_int = 10;
    printf("Starting...\n");
    
    /* 2. Type mismatch in binary operation (int + string literal) */
    int x = 5 + "string";  /* Error: invalid operands to binary + */
    
    /* 3. Pointer arithmetic with incorrect types */
    float y = &valid_int / 2;  /* Error: pointer/integer mismatch */
    
    /* 4. Bitwise operator on floating-point types */
    double d = 3.14 | 2.5;  /* Error: invalid operands to binary | */
    
    /* 5. Address-of operator on literal */
    int* p = &42;  /* Error: lvalue required */
    
    /* 6. Using macro to generate type mismatch */
    int macro_err = BAD_ADD(5, "text");  /* Expands to: (5 + "text") */
    
    /* 7. Invalid initializer - scalar with multiple values */
    int z = {5, 6};  /* Error: excess elements in scalar initializer */
    
    /* 8. Array with excess initializers */
    int arr[3] = {1, 2, 3, 4};  /* Error: excess elements in array initializer */
    
    /* 9. Undeclared identifier (out of scope) */
    {
        int inner = 100;
    }
    int outer = inner;  /* Error: 'inner' undeclared */
    
    /* 10. Invalid function call argument */
    printf("%d", 5 + "str");  /* Error: invalid operands in argument */
    
    /* 11. Misuse of GNU C statement expression */
    int stmt_expr = ({ int a; a; });  /* Error: statement expression returning void */
    
    /* 12. Invalid __builtin usage */
    int builtin_err = __builtin_ctz("hello");  /* Error: invalid argument type */
    
    /* 13. Token concatenation creating undefined identifier */
    int CONCAT_INVALID(foo, bar) = 5;  /* Error: undeclared identifier */
    
    /* 14. Invalid return expression */
    return main + 1;  /* Error: invalid operands to binary + (function pointer + int) */
    
    /* 15. Control flow with invalid condition */
    if (3.14 & 2.71) {  /* Error: invalid operands to binary & */
        printf("Never reached\n");
    }
    
    /* 16. For loop with invalid increment */
    for (int i = 0; i < 10; i += "step") {  /* Error: invalid operands to += */
        /* loop body */
    }
    
    /* More valid code to ensure parser engagement */
    int final_valid = 99;
    return 0;
}

/* Additional global error */
float* bad_global = &3.14;  /* Error: address of constant */

/* Function with invalid parameter initializer */
void bad_func(int n = {1, 2}) {  /* Error: excess elements in scalar initializer */
    /* function body */
}
