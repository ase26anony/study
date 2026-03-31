/* test_error_mark_node.c
 * This program contains various syntactically valid but semantically invalid
 * expressions designed to trigger the error_mark_node return path in expr.cc
 * during GCC compilation. The program will fail to compile.
 */

#include <stdio.h>

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) ((a) + (b))
#define BAD_BITWISE(a, b) ((a) | (b))

/* Token concatenation to create questionable identifier */
#define CONCAT(a, b) a ## b
int CONCAT(foo, bar); /* This is actually valid declaration */

/* Invalid macro usage */
#define INVALID_INIT {1, 2, 3, 4, 5}

/* Global scope errors */
int* global_ptr = &42;  /* ERROR: Address of constant literal */

int main(void) {
    /* Some valid code for context */
    int valid_int = 42;
    float valid_float = 3.14f;
    printf("Starting program (will not execute)\n");
    
    /* 1. Type mismatch in binary operation using macro */
    int x = BAD_ADD(5, "string");  /* ERROR: int + string literal */
    
    /* 2. Invalid pointer arithmetic */
    int* ptr = &valid_int;
    float y = ptr / 2;  /* ERROR: pointer divided by integer */
    
    /* 3. Bitwise operator on floating-point types */
    double d = 3.14159;
    double result = BAD_BITWISE(d, 2.5);  /* ERROR: bitwise OR on doubles */
    
    /* 4. Invalid initializer with excess elements */
    int arr[3] = INVALID_INIT;  /* ERROR: too many initializers */
    
    /* 5. Scalar initialized with multiple values */
    int z = {5, 6};  /* ERROR: scalar with multiple initializers */
    
    /* 6. Address-of operator on non-lvalue */
    int* p = &(valid_int + 1);  /* ERROR: address of temporary result */
    
    /* 7. Invalid use of GNU statement expression */
    int stmt_expr = ({
        int a;
        a;  /* ERROR: missing return value, 'a' uninitialized */
    });
    
    /* 8. Misuse of __builtin function */
    int trailing_zeros = __builtin_ctz("hello");  /* ERROR: string argument */
    
    /* 9. Undeclared identifier (out of scope) */
    {
        int inner_var = 100;
    }
    int outer_use = inner_var;  /* ERROR: inner_var out of scope */
    
    /* 10. Invalid function address arithmetic */
    void* func_ptr = &main + 1;  /* ERROR: arithmetic on function pointer */
    
    /* 11. Another valid statement for contrast */
    int ok = valid_int * 2;
    
    /* 12. Invalid conditional expression types */
    int cond = (valid_int > valid_float) ? "true" : 0.5;  /* ERROR: type mismatch */
    
    /* 13. Invalid compound literal */
    int* bad_literal = &(int){1, 2};  /* ERROR: compound literal with multiple values */
    
    /* 14. Misplaced label as expression */
    int label_value = &&here;  /* ERROR: label address in wrong context */
    here:
    
    /* 15. Array with negative size via expression */
    int size = -5;
    int negative_arr[size];  /* ERROR: variable-length array with negative size */
    
    return 0;
}

/* Additional global errors */
int global_array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};  /* Valid */

/* Function with invalid return expression */
int bad_function(void) {
    return main + 1;  /* ERROR: function pointer arithmetic */
}

/* Invalid nested initializer */
struct Point {
    int x;
    int y;
};

struct Point points[2] = {
    {1, 2, 3},  /* ERROR: too many initializers for struct */
    {4, 5}
};
