/* test_error_mark_node.c
 * This program contains various syntactically valid but semantically invalid
 * expressions designed to trigger the error_mark_node return path in expr.cc.
 * The program will fail to compile during semantic analysis.
 */

/* 1. Invalid initializer - scalar with multiple values in braces */
int global_bad = {1, 2, 3};  /* Should trigger error in global context */

/* 2. Macro generating type mismatches */
#define BAD_ADD(a, b) (a + b)
#define BAD_BITWISE(a, b) (a | b)

/* 3. Function-like macro with token concatenation creating invalid expression */
#define CONCAT_EXPR(x, y) x##y##_invalid
#define MAKE_BAD(x, y) x y

int main(void) {
    /* Some valid code for context */
    int valid_var = 42;
    float valid_float = 3.14;
    
    /* 4. Type mismatch in binary operation (int + string literal) */
    int bad1 = 5 + "string";  /* Invalid operand types */
    
    /* 5. Using macro to generate type mismatch */
    int bad2 = BAD_ADD(10, "text");
    
    /* 6. Bitwise operator on floating-point types */
    double bad3 = 3.14 | 2.71;  /* Invalid for floating point */
    
    /* 7. Address-of operator on constant literal */
    int* bad4 = &42;  /* Cannot take address of literal */
    
    /* 8. Invalid initializer - excess elements */
    int arr[3] = {1, 2, 3, 4, 5};  /* Too many initializers */
    
    /* 9. Undeclared identifier (out of scope) */
    {
        int inner = 10;
    }
    int bad5 = inner;  /* 'inner' out of scope here */
    
    /* 10. Misuse of GNU C statement expression */
    int bad6 = ({ 
        int a; 
        /* Missing return value - statement expression with no value */
    });
    
    /* 11. Invalid function argument */
    printf("%d", 5 + "str");  /* Type mismatch in argument */
    
    /* 12. Taking address of function in arithmetic */
    void* bad7 = (void*)(main + 1);  /* Function pointer arithmetic */
    
    /* 13. Misuse of __builtin with wrong argument type */
    int bad8 = __builtin_ctz("hello");  /* String instead of integer */
    
    /* 14. Invalid token concatenation attempt */
    /* This creates an invalid identifier when expanded */
    int CONCAT_EXPR(bad, expr) = 5;
    
    /* 15. Pointer arithmetic with incorrect types */
    float f;
    int* bad9 = &f + 1;  /* Type mismatch: &f is float*, assigned to int* */
    
    /* 16. Another invalid initializer */
    int bad10 = {7, 8};  /* Scalar with multiple values */
    
    /* 17. Control flow with invalid condition */
    if (5 | 3.14) {  /* Bitwise OR with float */
        valid_var = 1;
    }
    
    /* 18. For loop with invalid expression */
    for (int i = "start"; i < 10; i++) {  /* String as initializer */
        valid_var++;
    }
    
    /* 19. Return statement with invalid expression */
    return &valid_var + "string";  /* Pointer + string */
}
