/* test_error_mark_node.c
 * Contains syntactically valid but semantically invalid expressions
 * to trigger error_mark_node returns in GCC's expr.cc
 */

/* Global scope errors */
#define BAD_MACRO(x, y) (x & y)
#define CONCAT(a, b) a##b##_invalid

/* Invalid global initializer - excess initializers */
int global_arr[3] = {1, 2, 3, 4, 5};

/* Function prototype */
void some_function(int x);

/* Another macro that generates type mismatches */
#define TYPE_MISMATCH(a, b) (a + b)

int main(void) {
    /* Valid code to establish context */
    int valid_int = 42;
    float valid_float = 3.14f;
    
    /* 1. Type mismatch in binary operation - int + string literal */
    int x = 5 + "string";
    
    /* 2. Invalid operand combination - bitwise OR on floats */
    double d = 3.14 | 2.5;
    
    /* More valid code for context */
    if (valid_int > 0) {
        int inner_scope = 10;
    }
    
    /* 3. Out-of-scope identifier usage */
    int y = inner_scope * 2;
    
    /* 4. Address-of operator on constant */
    int* p = &42;
    
    /* 5. Scalar initialized with multiple values */
    int z = {5, 6, 7};
    
    /* 6. Macro-generated type mismatch */
    float f = TYPE_MISMATCH(valid_int, "text");
    
    /* 7. Invalid pointer arithmetic */
    int* ptr = &valid_int;
    int result = ptr / 2;
    
    /* 8. Misuse of GNU C statement expression */
    int stmt_expr = ({
        int a;
        /* Missing return value - should be last expression */
        a;
    });
    
    /* 9. Invalid builtin function usage */
    int bits = __builtin_ctz("hello");
    
    /* 10. Macro with token concatenation creating invalid identifier */
    int CONCAT(not, valid) = 100;
    
    /* 11. Function address in arithmetic */
    int func_addr = main + 1;
    
    /* 12. Array with wrong initializer count */
    int small_arr[2] = {1, 2, 3, 4};
    
    /* 13. Invalid conditional expression types */
    int cond = (valid_int > 0) ? "yes" : "no";
    
    /* 14. Compound literal misuse */
    int* bad_ptr = &(int){1, 2};
    
    /* 15. Another macro invocation with bad types */
    int macro_result = BAD_MACRO(valid_float, "text");
    
    /* Valid return to complete function */
    return 0;
}

/* Function with invalid expressions in parameters */
void some_function(int x) {
    /* 16. Invalid expression in function argument */
    printf("%d", 5 + "str");
    
    /* 17. Invalid return expression */
    return &x + "invalid";
}

/* Global function pointer with invalid initializer */
void (*func_ptr)(void) = &"not_a_function";

/* Invalid static initializer */
static int static_bad = {10, 20};
