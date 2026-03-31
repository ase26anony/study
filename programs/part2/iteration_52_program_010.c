/* test_error_mark_node.c
 * This program contains various syntactically valid but semantically invalid
 * expressions designed to trigger the error_mark_node return path in expr.cc.
 * It mixes valid and invalid code to ensure the parser engages before hitting errors.
 */

/* Global scope error: invalid initializer */
int global_arr[3] = {1, 2, 3, 4};  /* Excess initializers */

/* Macro that will generate type mismatches */
#define BAD_ADD(a, b) (a + b)
#define BAD_BITWISE(a, b) (a | b)
#define INVALID_INIT(val) {val, val*2}

/* Function-like macro with token concatenation creating invalid expression */
#define CONCAT_EXPR(x, y) x##y##_undeclared = 5

/* Misuse of GNU statement expression */
#define BAD_STMT_EXPR(x) ({ int temp; })

/* Invalid global expression (will be caught during parsing) */
/* int* bad_global_ptr = &42; */  /* Commented because it causes early failure */

int main(void) {
    /* 1. Type mismatch in binary operation (int + string literal) */
    int x = BAD_ADD(5, "string");
    
    /* 2. Valid statement to provide context */
    int ok = 5;
    
    /* 3. Applying bitwise operator to floating-point types */
    double d = 3.14;
    double result = BAD_BITWISE(d, 2.5);
    
    /* 4. Address-of operator on constant */
    int* p = &42;
    
    /* 5. Scalar initialized with multiple values */
    int z = INVALID_INIT(5);
    
    /* 6. Using undeclared identifier (out of scope reference) */
    {
        int inner = 10;
    }
    int outer = inner;  /* 'inner' is out of scope */
    
    /* 7. Invalid pointer arithmetic */
    int value = 10;
    float f = &value / 2;
    
    /* 8. Misuse of __builtin function with wrong argument type */
    int bits = __builtin_ctz("hello");
    
    /* 9. Statement expression missing return value */
    int n = BAD_STMT_EXPR(10);
    
    /* 10. Macro concatenation creating invalid identifier */
    CONCAT_EXPR(var, 1);
    
    /* 11. Invalid operand in conditional expression */
    int cond = (d > "string") ? 1 : 0;
    
    /* 12. Taking address of a function in arithmetic */
    int addr_diff = &main + 1;
    
    /* 13. Another valid statement */
    int another_ok = ok * 2;
    
    /* 14. Invalid array access with wrong type */
    char* str = "test";
    int invalid_access = str[3.14];
    
    /* 15. Return statement with invalid expression */
    return main + 1;
}
