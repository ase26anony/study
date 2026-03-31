/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

#include <stdio.h>
#include <stdlib.h>

/* ==================== COMPLEX FUNCTION DECLARATORS ==================== */

/* 1. Function pointer with nested parentheses - targets case '(' */
int (*func_ptr_simple)(int);
int (*(*complex_func_ptr)(int (*)(double)))(char);

/* 2. Typedef chain building complex function types */
typedef int (*fn1)(void);
typedef fn1 (*fn2)(int);
typedef fn2 (*fn3)(char, double);
fn3 deep_func_ptr;

/* 3. Function returning pointer to array of function pointers */
int (*(*func_ret_array(void))[5])(int, int);

/* ==================== COMPLEX ARRAY DECLARATIONS ==================== */

/* 4. Multi-dimensional arrays - targets case '[' */
int multi_dim_array[2+3][(4*2)][7];

/* 5. Array of pointers to functions returning pointers to arrays */
int (*(*array_of_func_ptrs[5])(int))[10];

/* 6. Array with parenthesized size expression */
char (*strings[(2+3)])[20];

/* 7. Pointer to array of pointers to arrays */
int (*(*ptr_to_array)[3])[4];

/* ==================== NESTED AGGREGATE TYPES ==================== */

/* 8. Struct with nested arrays and function pointers - targets case '{' */
struct Nested {
    int a[2][3];
    struct {
        char *p;
        double (*compute)(int, float);
    } inner;
    union {
        long u1;
        int (*func_array[2])(void);
    } data;
};

/* 9. Deeply nested struct with arrays */
struct Level1 {
    struct Level2 {
        struct Level3 {
            int matrix[2][2];
            char *(*get_name)(void);
        } l3;
        float values[4];
    } l2;
    void (*operation)(struct Level2*);
};

/* ==================== GLOBAL VARIABLES WITH INITIALIZERS ==================== */

/* 10. Global struct with initializer (nested braces) */
struct Nested global_nested = { 
    {{1,2,3},{4,5,6}}, 
    { NULL, NULL },
    { .func_array = { NULL, NULL } }
};

/* 11. Array with designated initializers and nested braces */
int arr_2d[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 12. Complex global with all delimiters mixed */
int (*global_complex)[3] = (int(*)[3])&arr_2d[0];

/* ==================== COMBINED CONSTRUCTS ==================== */

/* 13. Variable with initializer using compound literal (mixes all delimiters) */
int (*ptr_to_2d_array)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

/* 14. Function prototype with complex parameter */
void process_table(int (*table[])[5], struct { int x; double y; } param);

/* 15. Typedef combining all delimiter types */
typedef int (*(*complex_type)[3])(int[2], struct { char tag; });

/* ==================== HELPER FUNCTIONS ==================== */

/* Simple function compatible with function pointers */
int simple_func(int x) {
    return x * 2;
}

/* Function returning pointer to function */
int (*get_func_ptr(int x))(int) {
    return simple_func;
}

/* Function for complex function pointer */
int (*process_char(char c))(int) {
    static int result = 0;
    return simple_func;
}

/* ==================== MAIN FUNCTION ==================== */

int main(void) {
    int result = 0;
    
    /* 1. Use complex function pointer typedef */
    fn1 local_fn1 = simple_func;
    if (local_fn1) {
        result += local_fn1();
    }
    
    /* 2. Access nested array from global struct */
    result += global_nested.a[0][0];
    result += global_nested.a[1][2];
    
    /* 3. Use 2D array with designated initializers */
    result += arr_2d[0][1];
    result += arr_2d[1][2];
    
    /* 4. Use pointer to 2D array from compound literal */
    result += ptr_to_2d_array[0][0];
    result += ptr_to_2d_array[2][1];
    
    /* 5. Assign to complex function pointer */
    int (*(*local_complex)(int (*)(double)))(char) = NULL;
    
    /* 6. Create and use local struct with initializer */
    struct Nested local_nested = {
        {{7,8,9},{10,11,12}},
        { "test", NULL },
        { .u1 = 42 }
    };
    result += local_nested.a[0][2];
    
    /* 7. Array of pointers with initialization */
    char *str_array[] = { "hello", "world", "test" };
    if (str_array[0][0]) result += 1;
    
    /* 8. Complex local declaration with initializer */
    int (*(*local_array[2])(int))[3] = { NULL, NULL };
    
    /* 9. Nested initializer with all brace types */
    struct {
        int (*func)(int);
        int arr[2][2];
    } local_mixed = { 
        simple_func, 
        {{1,2},{3,4}} 
    };
    result += local_mixed.arr[1][1];
    
    /* 10. Call through function pointer */
    int (*local_func_ptr)(int) = simple_func;
    result += local_func_ptr(5);
    
    /* Print final result to prevent optimization */
    printf("Result: %d\n", result);
    
    return result == 0 ? 0 : 1;
}

/* ==================== ADDITIONAL COMPLEX DECLARATIONS ==================== */

/* 16. Even more complex global declarations */
void (*signal_handlers[3])(int, struct {
    void (*old_handler)(int);
    int flags[2];
});

/* 17. Union with nested struct and array */
union UltraComplex {
    struct {
        int (*(*func_table[2][2])(void))[3];
        char data[4][5];
    } s;
    long (*array_of_ptrs[(1+2)*3])[7];
};

/* 18. Function with complex return type and parameters */
struct Nested (*(*get_processor(void))(int, ...))(double) {
    return NULL;
}

/* 19. Static variable with complex type and initializer */
static int (*(*static_complex)(int (*)[2]))[3] = NULL;

/* 20. External declaration to force type analysis */
extern int (*(*extern_var)(int (*(*)(double))[3]))(char);
