/* test_error_mark_node.c - Program to trigger error_mark_node in expr.cc */

/* Global scope errors */
#define BAD_MACRO(x, y) (x & y)
#define CONCAT(a, b) a##b##INVALID

/* Invalid global initializer (excess initializers) */
int global_arr[3] = {1, 2, 3, 4, 5};  /* Line 251 trigger candidate */

/* Function prototype */
void test_func(int param);

int main(void) {
    /* Valid code for context */
    int valid_var = 42;
    printf("Starting test...\n");
    
    /* 1. Type mismatch in binary operation */
    int type_mismatch = valid_var + "string literal";  /* int + string */
    
    /* 2. Invalid operand combination - bitwise on float */
    double fp_var = 3.14159;
    double bitwise_float = fp_var | 2.71828;  /* bitwise OR on doubles */
    
    /* Valid statement to maintain structure */
    int ok = 5;
    
    /* 3. Invalid initializer - scalar with multiple values */
    int bad_init = {10, 20, 30};  /* scalar with list */
    
    /* 4. Address-of operator on constant */
    int* bad_ptr = &42;  /* address of literal */
    
    /* 5. Macro-generated type error */
    int macro_error = BAD_MACRO(5, "text");  /* int & string */
    
    /* 6. Invalid identifier via concatenation */
    int CONCAT(var, name) = 100;  /* creates varINVALIDnameINVALID? */
    
    /* 7. Statement expression misuse */
    int stmt_expr = ({ 
        int a; 
        /* missing return value - just declaration */
    });
    
    /* 8. Builtin function with wrong type */
    int builtin_error = __builtin_ctz("hello");  /* string instead of int */
    
    /* 9. Control flow with invalid condition */
    if (main + 1) {  /* function pointer arithmetic */
        printf("This shouldn't compile\n");
    }
    
    /* 10. Return statement with invalid expression */
    return &valid_var / 2;  /* pointer arithmetic in return */
}

/* Another function with scope errors */
void test_func(int param) {
    /* Use variable before declaration (non-C23) */
    printf("%d\n", undeclared_var);
    int undeclared_var = param;
    
    /* Inner scope reference error */
    {
        int inner_var = 100;
    }
    /* Try to use inner_var outside scope */
    int outer_ref = inner_var * 2;
    
    /* Array with wrong size initializer */
    char small_arr[2] = "too long";  /* string too long */
}

/* Global function pointer misuse */
void (*func_ptr)(void) = &"not a function";  /* address of string literal */

/* Invalid compound literal */
int* bad_lit = &(int){1, 2};  /* compound literal with multiple values */

/* Misaligned attribute */
int __attribute__((aligned(3.5))) misaligned_var;  /* non-power-of-2, float */
