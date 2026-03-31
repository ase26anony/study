/* test_error_mark_node.c
 * This program contains various syntactically valid but semantically invalid
 * expressions designed to trigger the error_mark_node return path in expr.cc
 * during GCC compilation. The program will fail to compile during semantic
 * analysis.
 */

/* Global scope errors */
#define BAD_MACRO(x, y) (x & y)  /* Bitwise operation on incompatible types */

/* Invalid global initializer - excess initializers */
int global_arr[3] = {1, 2, 3, 4, 5};

/* Function prototype */
void some_function(int param);

/* Another problematic macro with token concatenation */
#define CONCAT_INVALID(a, b) a##b + b##a

int main(void) {
    /* 1. Valid code for context */
    int valid_int = 42;
    float valid_float = 3.14f;
    
    /* 2. Type mismatch in binary operation - int + string literal */
    int type_mismatch = 5 + "string";
    
    /* 3. Invalid operand combination - bitwise OR on floating point */
    double invalid_bitwise = 3.14159 | 2.71828;
    
    /* 4. Address-of operator on constant literal */
    int* bad_pointer = &42;
    
    /* 5. More valid code */
    int counter = 0;
    
    /* 6. Invalid initializer - scalar with multiple values in braces */
    int bad_init = {10, 20, 30};
    
    /* 7. Undeclared identifier usage (out of scope) */
    {
        int inner_var = 100;
    }
    /* inner_var is now out of scope */
    int use_out_of_scope = inner_var * 2;
    
    /* 8. Misuse of GNU C statement expression - missing return value */
    int stmt_expr = ({ int a = 5; int b = 10; });
    
    /* 9. Invalid function argument expression */
    printf("%d", main + 1);  /* Taking address of main function in arithmetic */
    
    /* 10. Macro-generated error */
    float macro_error = BAD_MACRO(3.14, "text");
    
    /* 11. Invalid builtin usage */
    int builtin_error = __builtin_ctz("hello");
    
    /* 12. Control flow with invalid condition */
    if (5 + &valid_int) {  /* Pointer arithmetic in condition */
        counter++;
    }
    
    /* 13. Loop with invalid expression */
    for (int i = "start"; i < 10; i++) {  /* String as loop counter */
        /* Empty */
    }
    
    /* 14. Return statement with invalid expression */
    return &valid_int - "string";  /* Pointer minus string literal */
}

/* Additional function with errors in parameter handling */
void some_function(int param) {
    /* 15. Invalid compound assignment */
    param += "text";
    
    /* 16. Array with wrong initializer count */
    char small_arr[2] = "Hello, World!";
    
    /* 17. Misuse of token concatenation macro */
    int concat_result = CONCAT_INVALID(123, 456);  /* Creates invalid identifiers */
}

/* Global function pointer with invalid assignment */
int (*func_ptr)(void) = &"not a function";

/* Invalid static initializer */
static int static_bad = { {1, 2}, {3, 4} };
