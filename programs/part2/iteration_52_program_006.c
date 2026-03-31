/* test_expr_error.c
 * This program contains various malformed expressions designed to trigger
 * the error_mark_node return path in expr.cc during GCC compilation.
 * The program will fail to compile during semantic analysis.
 */

/* Global scope errors */
#define BAD_MACRO(x, y) (x & y)  /* Bitwise operation on potentially incompatible types */

/* Invalid global initializer - excess initializers */
int global_arr[3] = {1, 2, 3, 4, 5};

/* Macro that generates type mismatch when expanded */
#define TYPE_MISMATCH(a, b) (a + b)

/* Token concatenation to create strange identifier (still parseable) */
#define CONCAT(a, b) a##b
int CONCAT(var, 123) = 10;  /* This part is actually valid */

/* Function prototype */
void some_function(int x);

int main(void) {
    /* 1. Valid code for context */
    int valid_int = 42;
    float valid_float = 3.14f;
    
    /* 2. Type mismatch in binary operation - int + string literal */
    int bad_sum = 5 + "hello";  /* error_mark_node candidate */
    
    /* 3. Invalid operand combination - bitwise OR on floating point */
    double bad_bitwise = 3.14 | 2.71;  /* error_mark_node candidate */
    
    /* 4. Address-of operator on constant */
    int* bad_ptr = &42;  /* error_mark_node candidate */
    
    /* 5. Another valid statement for contrast */
    int another_valid = valid_int * 2;
    
    /* 6. Invalid initializer - scalar with multiple values in braces */
    int bad_init = {5, 6, 7};  /* error_mark_node candidate */
    
    /* 7. Misuse of GCC statement expression - missing return value */
    int bad_stmt_expr = ({ 
        int temp = 10;
        /* Missing expression result */
    });
    
    /* 8. Undeclared identifier usage (out of scope) */
    {
        int inner_scope = 100;
    }
    /* Try to use inner_scope outside its block */
    int use_out_of_scope = inner_scope + 1;  /* error_mark_node candidate */
    
    /* 9. Macro-generated type mismatch */
    float macro_bad = TYPE_MISMATCH(valid_int, "string");  /* error_mark_node candidate */
    
    /* 10. Invalid pointer arithmetic */
    int x = 10;
    float* bad_ptr_arith = &x + 2.5;  /* error_mark_node candidate */
    
    /* 11. Misuse of __builtin function */
    int bad_builtin = __builtin_ctz("not an integer");  /* error_mark_node candidate */
    
    /* 12. Invalid in control flow condition */
    if (5 + "test") {  /* error_mark_node candidate */
        valid_int = 1;
    }
    
    /* 13. Invalid return expression - taking address of main */
    /* This would be in a return statement if we had a non-void function */
    /* Instead, use it in an expression */
    void* bad_addr = &main + 1;  /* error_mark_node candidate */
    
    /* 14. Invalid array access with wrong type */
    char* str = "test";
    int bad_array_access = str[valid_float];  /* error_mark_node candidate */
    
    /* 15. Complex expression with multiple errors */
    int complex_bad = (5 + "a") * (&valid_int - "b");  /* error_mark_node candidate */
    
    /* Try to use the bad expressions to avoid "unused variable" warnings */
    (void)bad_sum;
    (void)bad_bitwise;
    (void)bad_ptr;
    (void)bad_init;
    (void)bad_stmt_expr;
    (void)use_out_of_scope;
    (void)macro_bad;
    (void)bad_ptr_arith;
    (void)bad_builtin;
    (void)bad_addr;
    (void)bad_array_access;
    (void)complex_bad;
    
    return 0;
}

/* Another function with different error context */
void some_function(int x) {
    /* Invalid: applying sizeof to a bit-field-like expression */
    int bad_sizeof = sizeof(x & 0xF0);  /* Might trigger error_mark_node */
    
    /* Invalid: comma operator with type mismatch */
    int bad_comma = (x++, "string", 3.14);  /* error_mark_node candidate */
    
    (void)bad_sizeof;
    (void)bad_comma;
}

/* Global variable with invalid initializer using address-of on non-lvalue */
int* global_bad_ptr = &(valid_int + 1);  /* error_mark_node candidate - note: valid_int not in scope here */
/* Actually, valid_int is not in scope, so this will also be an undefined identifier */
/* Let's fix that by using a compound literal incorrectly */
int* another_global_bad = &(int){10} + 1;  /* Taking address of compound literal then adding - might be valid */
/* Better: try to take address of a cast expression */
int* global_bad_addr = &(int)3.14;  /* error_mark_node candidate - can't take address of cast */
