/* test_expr_error.c
 * This program contains various syntactically valid but semantically invalid
 * expressions designed to trigger the error_mark_node return path in expr.cc.
 * The program will fail to compile during semantic analysis.
 */

/* Global scope errors */
#define BAD_MACRO(x, y) (x & y)
#define CONCAT(a, b) a##b + b##a

/* Invalid global initializer - excess initializers */
int global_arr[3] = {1, 2, 3, 4, 5};

/* Function prototype */
void some_function(int param);

/* Another macro that generates type mismatches */
#define TYPE_MISMATCH(a, b) (a + b)

int main(void) {
    /* Valid code to provide context */
    int valid_int = 42;
    float valid_float = 3.14f;
    
    /* 1. Type mismatch in binary operation - int + string literal */
    int x = 5 + "string";
    
    /* 2. Invalid operand combination - bitwise OR on floating point */
    double d = 3.14 | 2.5;
    
    /* More valid code */
    if (valid_int > 0) {
        int inner_scope = 10;
    }
    
    /* 3. Using address-of operator on constant */
    int* p = &42;
    
    /* 4. Invalid initializer - scalar with multiple values in braces */
    int z = {5, 6, 7};
    
    /* 5. Undeclared identifier (out of scope) */
    /* The variable 'inner_scope' was declared inside the if block above */
    int y = inner_scope * 2;
    
    /* 6. Macro-generated type mismatch */
    float f = TYPE_MISMATCH(5, "text");
    
    /* 7. Invalid pointer arithmetic */
    int arr[5];
    float* fp = (float*)&arr[0];
    int ptr_math = fp / 2;
    
    /* 8. Misuse of GNU C statement expression - missing return value */
    int stmt_expr = ({ int a; a; });
    
    /* 9. Invalid builtin usage */
    int builtin_err = __builtin_ctz("hello");
    
    /* 10. Macro with token concatenation creating weird identifier */
    int CONCAT(var, 123) = 5;
    
    /* 11. Function address arithmetic */
    int func_addr = main + 1;
    
    /* 12. Invalid conditional expression types */
    int cond = (valid_int > 0) ? "true" : 3.14;
    
    /* 13. Array with wrong type initializer */
    char* str_arr[3] = { "one", 2, "three" };
    
    /* 14. Complex expression with multiple type errors */
    int complex = (valid_int * "text") + (&valid_float - 1);
    
    /* 15. Invalid switch case expression (not constant) */
    int dynamic = 5;
    switch (valid_int) {
        case dynamic:  /* Not a constant expression */
            break;
        case 1 + "a":  /* Invalid constant expression */
            break;
    }
    
    /* 16. Return statement with invalid expression */
    return &main + "invalid";
}

/* Function with invalid parameter usage */
void some_function(int param) {
    /* 17. Applying sizeof to incomplete type */
    int size = sizeof(struct undefined_struct);
    
    /* 18. Invalid compound literal */
    int* bad_lit = &(int){1, 2, 3};
    
    /* 19. Misaligned pointer cast */
    char c = 'A';
    double* bad_cast = (double*)&c;
    *bad_cast = 3.14;
}

/* Global scope error with macro expansion */
int global_bad = BAD_MACRO(5, 3.14);

/* Invalid array designator */
struct S {
    int a;
    int b;
} s = {
    .a = 1,
    .c = 2  /* Invalid member */
};
