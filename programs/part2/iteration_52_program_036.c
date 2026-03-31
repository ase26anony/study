/* test_error_mark_node.c
 * This program is designed to trigger the error_mark_node return path in expr.cc
 * by including syntactically valid but semantically invalid expressions.
 * Compile with: gcc -fsyntax-only -O0 test_error_mark_node.c
 * or: gcc -c -O1 -fno-omit-frame-pointer test_error_mark_node.c
 */

/* Global scope errors */
int* global_ptr = &42;  /* Invalid: address-of operator on constant */

/* Macro to generate type mismatches */
#define BAD_ADD(a, b) (a + b)
#define BAD_BITWISE(a, b) (a | b)
#define CONCAT_INVALID(x, y) x##y##_undeclared

/* Function-like macro with problematic expansion */
#define PROBLEMATIC_INIT(var, val) int var = {val, val + 1}

/* Valid function prototype for context */
void valid_function(int x);

int main(void) {
    /* 1. Valid code for context */
    int valid_var = 10;
    printf("Starting...\n");  /* Valid call */
    
    /* 2. Type mismatch in binary operation (int + string) */
    int x = 5 + "string";  /* Invalid: adding integer and string pointer */
    
    /* 3. Invalid operand combination: bitwise operator on floats */
    double d = 3.14 | 2.5;  /* Invalid: bitwise OR on floating-point values */
    
    /* 4. Using macro to generate type mismatch */
    float y = BAD_ADD(&valid_var, 2);  /* Invalid: pointer + integer in macro */
    
    /* 5. Invalid initializer: scalar with multiple values */
    int z = {5, 6};  /* Invalid: scalar initialized with brace-enclosed list */
    
    /* 6. Undeclared identifier via macro concatenation */
    int CONCAT_INVALID(foo, bar) = 7;  /* Creates invalid identifier */
    
    /* 7. Statement expression misuse (GCC extension) */
    int w = ({ int a; a; });  /* Invalid: missing return value in statement expr */
    
    /* 8. Invalid in control flow condition */
    if (5 + "text") {  /* Invalid: pointer arithmetic in condition */
        printf("Never reached\n");
    }
    
    /* 9. Builtin function misuse */
    int bits = __builtin_ctz("hello");  /* Invalid: string argument to ctz */
    
    /* 10. Array initializer with excess elements */
    int arr[3] = {1, 2, 3, 4};  /* Invalid: too many initializers */
    
    /* 11. Return statement with invalid expression */
    return main + 1;  /* Invalid: taking address of function in expression */
}

/* Another function with scope issues */
void test_scope(void) {
    {
        int inner_var = 42;
    }
    /* Try to use out-of-scope variable */
    int outer_var = inner_var * 2;  /* Invalid: inner_var not in scope */
}

/* Global with problematic macro */
PROBLEMATIC_INIT(global_bad, 5);  /* Expands to invalid initializer */

/* Valid function definition for more context */
void valid_function(int x) {
    /* Valid statement */
    int local = x * 2;
    
    /* Invalid: applying address-of to literal in argument */
    printf("%p\n", &100);  /* Invalid: address of constant */
}
