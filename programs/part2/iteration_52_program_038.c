/* test_error_mark_node.c
 * This program contains various syntactically valid but semantically invalid
 * expressions designed to trigger the error_mark_node return path in expr.cc.
 * It mixes valid and invalid code to ensure the parser engages before hitting errors.
 */

/* Global scope errors */
#define BAD_MACRO(a, b) (a & b)  /* Bitwise operation on incompatible types */
#define CONCAT(a, b) a##b  /* Token concatenation */

/* Invalid global initializer - excess initializers */
int global_arr[3] = {1, 2, 3, 4, 5};

/* Function prototype */
void some_function(int x);

int main(void) {
    /* 1. Valid code to establish context */
    int valid_var = 42;
    float valid_float = 3.14f;
    
    /* 2. Type mismatch in binary operation */
    int type_mismatch = valid_var + "string literal";  /* int + string pointer */
    
    /* 3. Invalid operand combination - bitwise on float */
    double d = 3.14159;
    double bitwise_float = d | 2.71828;  /* Bitwise OR on doubles */
    
    /* 4. Address-of operator on constant */
    int* bad_ptr = &42;  /* Taking address of literal */
    
    /* 5. Using macro to generate type mismatch */
    int macro_error = BAD_MACRO(valid_float, "text");  /* float & string */
    
    /* 6. Invalid initializer - scalar with multiple values */
    int bad_init = {5, 6, 7};  /* Multiple values for scalar */
    
    /* 7. Undeclared identifier (out of scope) */
    {
        int inner_var = 100;
    }
    int scope_error = inner_var * 2;  /* inner_var no longer in scope */
    
    /* 8. Misuse of GCC statement expression */
    int stmt_expr = ({ 
        int a; 
        /* Missing return value - just declarations */
        float b; 
    });  /* No value returned from block */
    
    /* 9. Invalid function argument */
    some_function(5 + "string");  /* Type mismatch in argument */
    
    /* 10. Misuse of __builtin function */
    int builtin_error = __builtin_ctz("hello");  /* String instead of integer */
    
    /* 11. Token concatenation creating invalid code */
    int CONCAT(var, 123) = 5;  /* Creates var123 - valid but testing macro expansion */
    
    /* 12. Return statement with invalid expression */
    return main + 1;  /* Taking address of function in arithmetic */
}

/* Another function with errors in different context */
void some_function(int x) {
    /* 13. Control flow condition with type error */
    if (x + "text") {  /* int + string pointer in condition */
        /* 14. Loop with invalid condition */
        while (&x / 2) {  /* Pointer arithmetic in condition */
            /* 15. Invalid expression statement */
            3.14 << 2;  /* Shift on floating point */
        }
    }
    
    /* 16. Array with invalid size expression */
    int dynamic_size[sizeof("string") >> "text"];  /* Type mismatch in shift */
    
    /* 17. Compound literal with errors */
    int* ptr = &(int){ &x };  /* Taking address of compound literal with wrong type */
}

/* Global function pointer with type mismatch */
int (*func_ptr)(void) = &some_function;  /* Wrong function signature */

/* Invalid static initializer */
static int static_bad = { {1, 2}, {3, 4} };  /* Nested braces for scalar */
