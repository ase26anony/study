/* test_error_mark_node.c */
#include <stdio.h>

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) (a + b)
#define BAD_BITWISE(a, b) (a | b)

/* Invalid macro with token concatenation */
#define MAKE_BAD_VAR(prefix) prefix##_undeclared_var

/* Global scope errors */
int* bad_global = &42;  /* Address of literal */

int main(void) {
    /* Valid code for context */
    int valid_int = 10;
    float valid_float = 3.14f;
    char valid_str[] = "hello";
    
    /* 1. Type mismatch in binary operation (int + string) */
    int x = 5 + "string";  /* Invalid: adding int and string literal */
    
    /* 2. Pointer arithmetic with incorrect types */
    float* fp = &valid_float;
    float y = fp / 2;  /* Invalid: dividing pointer by integer */
    
    /* 3. Bitwise operator on floating-point types */
    double d = 3.14 | 2.5;  /* Invalid: bitwise OR on doubles */
    
    /* 4. Address-of operator on constant */
    int* p = &(valid_int + 5);  /* Invalid: address of temporary expression */
    
    /* Valid statement to maintain structure */
    printf("Valid value: %d\n", valid_int);
    
    /* 5. Using macro to generate type mismatch */
    int macro_error = BAD_ADD(valid_int, "world");
    
    /* 6. Invalid initializer - scalar with multiple values */
    int z = {5, 6, 7};  /* Invalid: too many initializers for scalar */
    
    /* 7. Statement expression misuse (GCC extension) */
    int stmt_expr = ({ 
        int a; 
        /* Missing return value - block doesn't produce a value */
    });
    
    /* 8. Builtin function with wrong argument type */
    int builtin_error = __builtin_ctz("string");  /* Invalid: string arg to ctz */
    
    /* 9. Array initializer with excess elements */
    int arr[3] = {1, 2, 3, 4, 5};  /* Invalid: too many initializers */
    
    /* 10. Control flow with invalid condition */
    if (valid_float & 0x1) {  /* Invalid: bitwise AND on float */
        printf("This shouldn't compile\n");
    }
    
    /* 11. Return statement with invalid expression */
    return main + 1;  /* Invalid: adding to function pointer */
}
