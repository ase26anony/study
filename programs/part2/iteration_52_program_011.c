/* test_error_mark_node.c - Program to trigger error_mark_node in expr.cc */

/* Global scope errors */
#define BAD_MACRO(x, y) (x & y)
#define CONCAT(a, b) a##b##_invalid

/* Invalid global initializer (excess initializers) */
int global_arr[3] = {1, 2, 3, 4, 5};  /* Line 251 trigger: excess initializers */

/* Function prototype */
void some_function(int x);

int main(void) {
    /* Valid code structure */
    int valid_var = 42;
    printf("Starting...\n");
    
    /* Pattern 1: Type mismatches in binary operations */
    int x = 5 + "string";  /* int + string literal */
    float y = &valid_var / 2;  /* pointer arithmetic type mismatch */
    
    /* Pattern 2: Invalid operand combinations */
    double d = 3.14 | 2.5;  /* bitwise OR on floating point */
    int* p = &42;  /* address-of on literal */
    
    /* Pattern 3: Using macro to generate type mismatch */
    int bad_macro_result = BAD_MACRO(3.14, "text");
    
    /* Valid statement for context */
    int ok = 5 * 2;
    
    /* Pattern 4: Invalid initializers */
    int z = {5, 6};  /* scalar with multiple values */
    int arr[2] = {1, 2, 3};  /* excess initializers */
    
    /* Pattern 5: Undeclared/out-of-scope identifiers */
    {
        int inner = 10;
    }
    /* Try to use 'inner' outside its scope */
    int outer = inner * 2;
    
    /* Pattern 6: Invalid function argument */
    printf("%d", 5 + "str");
    
    /* Pattern 7: Misusing GCC extensions */
    /* Statement expression missing return value */
    int stmt_expr = ({ int a = 5; });
    
    /* Misuse __builtin function */
    int builtin_bad = __builtin_ctz("hello");
    
    /* Pattern 8: Invalid return expression */
    return main + 1;  /* address of function in arithmetic */
}

/* Additional function with errors */
void some_function(int x) {
    /* Pattern 9: Invalid operations in control flow */
    if (3.14 & 2.71) {  /* bitwise on floats in condition */
        x = x + 1;
    }
    
    /* Pattern 10: Token concatenation creating invalid identifier */
    int CONCAT(123, 456) = 789;  /* creates: 123456_invalid = 789 */
    
    /* Pattern 11: Array with wrong initializer type */
    char* str_arr[] = { "hello", 123, "world" };  /* int in string array */
    
    /* Pattern 12: Complex invalid expression */
    for (double* ptr = &3.14; ptr < &2.71; ptr++) {  /* address of literals */
        *ptr = *ptr + 1;
    }
}

/* Pattern 13: Invalid global with GNU extension misuse */
static int global_bad = ({ 
    struct { int a; char b; } s;
    s;  /* incomplete expression in statement expr */
});
