/* test_error_mark_node.c
 * This program contains various syntactically valid but semantically invalid
 * expressions designed to trigger the error_mark_node return path in expr.cc
 * during GCC compilation.
 */

/* Global scope errors */
#define BAD_MACRO(a, b) (a & b)  /* Bitwise operation on potentially incompatible types */
#define CONCAT(a, b) a##b  /* Token concatenation */

/* Invalid global initializer - excess initializers */
int global_arr[3] = {1, 2, 3, 4, 5};

/* Function prototype */
void some_function(int x);

/* Another macro that will be misused */
#define BAD_BUILTIN(x) __builtin_popcount(x)

int main(void) {
    /* 1. Type mismatch in binary operation - int + string literal */
    int x = 5 + "string";  /* Invalid: adding integer and pointer */
    
    /* 2. Valid statement for context */
    int ok = 5;
    
    /* 3. Invalid operand combination - bitwise OR on floating point */
    double d = 3.14;
    double result = d | 2.5;  /* Invalid: bitwise operation on double */
    
    /* 4. Address-of operator on constant */
    int* p = &42;  /* Invalid: cannot take address of literal */
    
    /* 5. Misuse of macro with type mismatch */
    int bad_macro_result = BAD_MACRO(3.14, ok);  /* float & int */
    
    /* 6. Invalid initializer - scalar with multiple values */
    int z = {5, 6, 7};  /* Invalid: too many initializers for scalar */
    
    /* 7. Undeclared identifier (out of scope) */
    {
        int inner = 10;
    }
    /* inner = 20; */  /* Would be invalid, but commented to allow other errors */
    
    /* 8. Invalid function argument */
    some_function(5 + "text");  /* Type mismatch in argument */
    
    /* 9. Misuse of GNU C statement expression */
    int stmt_expr = ({ 
        int a; 
        /* Missing return value - just a declaration */
    });
    
    /* 10. Invalid builtin usage */
    int builtin_bad = BAD_BUILTIN("hello");  /* String instead of integer */
    
    /* 11. Pointer arithmetic with wrong types */
    float f = 3.0;
    float* fp = &f;
    float bad_ptr_arith = fp / 2;  /* Invalid: pointer division */
    
    /* 12. More complex invalid expression in control flow */
    if (main + 1) {  /* Invalid: function pointer arithmetic */
        ok = 10;
    }
    
    /* 13. Token concatenation creating potentially invalid identifier */
    int CONCAT(var, 123) = 5;  /* Creates var123 - valid but testing macro expansion */
    
    /* 14. Return statement with invalid expression */
    return &ok + "string";  /* Multiple type issues in return */
}

/* Helper function definition */
void some_function(int x) {
    /* Invalid expression inside another function */
    char* bad = 5.67;  /* Type mismatch in assignment */
    
    /* Attempt to use address-of on register variable (if supported) */
    register int reg = 10;
    /* int* regptr = &reg; */ /* Potentially invalid depending on context */
}
