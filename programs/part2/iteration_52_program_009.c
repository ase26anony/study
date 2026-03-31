/* test_error_mark_node.c
 * This program contains various syntactically valid but semantically invalid
 * expressions designed to trigger the error_mark_node return path in expr.cc.
 * The program will fail to compile during semantic analysis.
 */

/* Global scope errors */
#define BAD_MACRO(x, y) (x & y)  /* Bitwise operator macro */
#define CONCAT(a, b) a##b        /* Token concatenation */

/* Invalid global initializer - excess initializers */
int global_arr[3] = {1, 2, 3, 4, 5};

/* Function prototype */
void some_function(int param);

/* Another macro that will be misused */
#define BAD_EXPR(a, b) (a + b)

int main(void) {
    /* Valid code for context */
    int valid_var = 42;
    printf("Starting program...\n");
    
    /* 1. Type mismatch in binary operation */
    int type_mismatch = 5 + "string literal";
    
    /* 2. Invalid operand combination - bitwise on float */
    double floating = 3.14159;
    double bitwise_float = floating | 2.71828;
    
    /* Valid statement to provide context */
    int ok = 10;
    
    /* 3. Address-of operator on constant */
    int* bad_ptr = &42;
    
    /* 4. Using macro with type mismatch */
    int macro_result = BAD_EXPR(5, "text");
    
    /* 5. Invalid initializer - scalar with multiple values */
    int bad_init = {7, 8, 9};
    
    /* 6. Statement expression misuse (GCC extension) */
    int stmt_expr = ({ int a; a; });
    
    /* 7. Undeclared identifier (out of scope) */
    {
        int inner = 100;
    }
    int use_after_scope = inner * 2;
    
    /* 8. Builtin function with wrong argument type */
    int builtin_misuse = __builtin_ctz("hello");
    
    /* 9. Token concatenation creating weird identifier */
    int CONCAT(var, 123) = 50;
    
    /* 10. Function address arithmetic */
    void (*func_ptr)(void) = &main + 1;
    
    /* 11. Pointer arithmetic with wrong types */
    float* float_ptr = (float*)&valid_var;
    float_ptr = float_ptr / 2;
    
    /* 12. Control flow with invalid condition */
    if (5 + "test") {
        printf("This shouldn't compile\n");
    }
    
    /* 13. Return statement with invalid expression */
    return &main + "invalid";
}
