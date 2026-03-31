/* test_expr_error.c
 * This program contains various syntactically valid but semantically invalid
 * expressions designed to trigger the error_mark_node return path in expr.cc.
 * The program will fail to compile during semantic analysis.
 */

/* Global scope errors */
#define BAD_MACRO(x, y) (x & y)  /* Bitwise operation macro */
#define CONCAT(a, b) a##b        /* Token concatenation */

/* Invalid global initializer - excess initializers */
int global_arr[3] = {1, 2, 3, 4, 5};

/* Function prototype */
void some_function(int param);

/* Another macro that will be misused */
#define BAD_EXPR(a, b) (a + b)

/* Invalid use of address operator on constant (global scope) */
int* bad_ptr_global = &42;  /* Line 17: & on literal */

int main(void) {
    /* Valid code for context */
    int valid_var = 10;
    printf("Starting program...\n");
    
    /* Pattern 1: Type mismatch in binary operation */
    int type_mismatch = 5 + "string";  /* Line 23: int + string literal */
    
    /* Pattern 2: Bitwise operator on floating point */
    double floating_bitwise = 3.14 | 2.71;  /* Line 26: | on doubles */
    
    /* Valid statement to provide context */
    int ok = 5;
    
    /* Pattern 3: Invalid initializer - scalar with multiple values */
    int bad_init = {5, 6, 7};  /* Line 31: multiple values for scalar */
    
    /* Pattern 4: Using macro with type mismatch */
    int macro_error = BAD_EXPR(10, "text");  /* Line 34: int + string via macro */
    
    /* Pattern 5: Invalid pointer arithmetic */
    int x = 5;
    float ptr_math = &x / 2;  /* Line 37: pointer in division */
    
    /* Pattern 6: Misuse of GNU C statement expression */
    int stmt_expr = ({ 
        int a; 
        /* Missing return value - just a declaration */
        double b; 
    });  /* Line 43: statement expr without value */
    
    /* Pattern 7: Invalid builtin function usage */
    int builtin_error = __builtin_ctz("hello");  /* Line 46: string arg to ctz */
    
    /* Pattern 8: Out of scope identifier usage */
    {
        int inner_scope = 42;
    }
    int scope_error = inner_scope * 2;  /* Line 52: inner_scope out of scope */
    
    /* Pattern 9: Address of function in arithmetic */
    int func_addr_math = main + 1;  /* Line 55: function pointer arithmetic */
    
    /* Pattern 10: Using token concatenation to create weird identifier */
    int CONCAT(var, ++) = 5;  /* Line 58: creates var++ identifier */
    
    /* Pattern 11: Invalid compound literal */
    int* bad_compound = (int[]){1, 2, 3} + 4;  /* Line 61: array pointer arithmetic */
    
    /* Pattern 12: Misusing macro with bitwise operators */
    float float_bitwise = BAD_MACRO(3.14f, 2.5f);  /* Line 64: float & float via macro */
    
    /* Pattern 13: Invalid conditional expression types */
    int cond_expr = (valid_var > 0) ? "yes" : 10;  /* Line 67: mismatched types */
    
    /* Pattern 14: Taking address of register variable */
    register int reg_var = 42;
    int* reg_addr = &reg_var;  /* Line 71: address of register variable */
    
    /* Pattern 15: Invalid shift operation */
    double shift_error = 3.14 << 2;  /* Line 74: shift on double */
    
    /* Control flow with invalid expression */
    if (5 + "test") {  /* Line 77: invalid if condition */
        printf("This shouldn't compile\n");
    }
    
    /* Loop with invalid condition */
    while (&valid_var - "string") {  /* Line 82: pointer minus string */
        break;
    }
    
    /* Return statement with invalid expression */
    return main * 2;  /* Line 86: function pointer multiplication */
}

/* Function with invalid parameter usage */
void some_function(int param) {
    /* Pattern 16: Applying sizeof to incomplete type */
    int size_err = sizeof(struct undefined);  /* Line 92: incomplete type */
    
    /* Pattern 17: Invalid cast operation */
    float cast_err = (float)"text";  /* Line 95: string to float cast */
}

/* Global function pointer misuse */
int (*func_ptr)(void) = &main + 1;  /* Line 99: function pointer arithmetic */

/* Invalid static initializer */
static int static_bad = {1, 2};  /* Line 102: multiple values for static scalar */

/* Misaligned attribute usage (GCC extension misuse) */
int __attribute__((aligned(3.14))) misaligned_var = 5;  /* Line 105: non-power-of-2 alignment */
