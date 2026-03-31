/* test_error_mark_node.c
 * Contains syntactically valid but semantically invalid expressions
 * to trigger error_mark_node returns in GCC's expr.cc
 */

/* Global scope errors */
int global_bad = 5 + "string";  /* Type mismatch in binary operation */

/* Invalid macro definitions */
#define BAD_ADD(a, b) (a + b)
#define BAD_CONCAT(x, y) x##y##z  /* Will create invalid identifier */
#define BAD_INIT {1, 2, 3, 4, 5}

/* Function prototype */
void some_function(int x);

int main(void) {
    /* Valid code to establish context */
    int valid_var = 42;
    float valid_float = 3.14;
    
    /* Pattern 1: Type mismatches in binary operations */
    int type_mismatch = valid_var + "hello";  /* int + string literal */
    float ptr_math = (float)(&valid_var / 2); /* pointer arithmetic with float cast */
    
    /* Pattern 2: Invalid operand combinations */
    double bitwise_float = 3.14 | 2.71;  /* bitwise OR on doubles */
    int* addr_const = &42;               /* address of constant */
    
    /* Pattern 3: Using macro to generate type mismatch */
    int macro_bad = BAD_ADD(10, "world");
    
    /* Pattern 4: Invalid initializers */
    int excess_init[3] = {1, 2, 3, 4};  /* too many initializers */
    int scalar_list = {5, 6};           /* scalar with multiple values */
    
    /* Pattern 5: Statement expression misuse (GCC extension) */
    int stmt_expr = ({ 
        int a; 
        /* Missing return value - just a declaration */
    });
    
    /* Pattern 6: Builtin function misuse */
    int builtin_bad = __builtin_ctz("string");  /* string instead of integer */
    
    /* Pattern 7: Control flow with invalid expressions */
    if (valid_var + "test") {  /* invalid condition */
        int inner = 5;
    }
    
    /* Pattern 8: Return statement with invalid expression */
    return main + 1;  /* taking address of main function */
    
    /* Unreachable valid code */
    valid_var = valid_var * 2;
    return 0;
}

/* Additional function with errors */
void some_function(int x) {
    /* Undeclared identifier usage (C89/C99 style) */
    undeclared_var = 10;  /* no declaration in scope */
    
    /* Scope violation attempt */
    {
        int inner_var = 99;
    }
    inner_var = 100;  /* out of scope */
    
    /* Invalid array access */
    char* str = "test";
    int bad_index = str[3.14];  /* float as array index */
}

/* Global with macro-generated invalid initializer */
int global_array[2] = BAD_INIT;  /* too many initializers via macro */

/* Attempt to use concatenated invalid identifier */
int BAD_CONCAT(invalid, _id) = 5;  /* becomes invalid_idz which doesn't exist */
