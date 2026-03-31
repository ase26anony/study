/* tree_coverage_test.c - Test program to exercise GCC's internal tree node classification */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper using __builtin_choose_expr with statement expressions */
static inline int builtin_choose_test(int x) {
    /* This creates complex expression trees */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        ({ int y = x * 2; y + 1; }),
        ({ int y = x / 2; y - 1; })
    );
}

/* Function with GNU statement expression in return */
static int statement_expr_func(int a) {
    return ({
        int temp = a;
        for (int i = 0; i < 3; i++) temp += i;
        temp;
    });
}

/* Function using __builtin_types_compatible_p in type-dependent context */
#define TYPE_SAFE_ADD(a, b) \
    __builtin_choose_expr( \
        __builtin_types_compatible_p(typeof(a), typeof(b)), \
        (a) + (b), \
        (__builtin_types_compatible_p(typeof(a), int) ? (int)(b) : (float)(b)) \
    )

/* C11 _Generic selector - creates SELECTOR tree node */
#define type_name(x) _Generic((x), \
    int: "int", \
    float: "float", \
    double: "double", \
    default: "unknown" \
)

/* Function with static array index parameter (C99 feature) */
void process_array(int arr[static 10]) {
    for (int i = 0; i < 10; i++) arr[i] *= 2;
}

/* Function with attribute in unusual position */
int __attribute__((unused)) unused_func(int x __attribute__((unused))) {
    return 0;
}

/* OpenMP test function */
void omp_test(int n) {
    int sum = 0;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i * i;
    }
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < n; i++) {
        /* Empty - just for tree generation */
    }
}

/* Function with case ranges (GNU extension) */
void switch_with_ranges(int x) {
    switch (x) {
        case 1 ... 5:
            printf("Range 1-5\n");
            break;
        case 10 ... 20:
            printf("Range 10-20\n");
            break;
        default:
            printf("Other\n");
    }
}

/* Function using user-defined literal style (C++-like but in C) */
#define DECIMAL_LITERAL(n) (n##UL)

/* Complex expression mixing multiple extensions */
int complex_expression(int a, float b) {
    /* Mix of builtins, statement exprs, and type checking */
    int result = __builtin_choose_expr(
        __builtin_types_compatible_p(typeof(a), int),
        statement_expr_func(a),
        (int)b
    );
    
    result += builtin_choose_test(result);
    
    /* Use _Generic */
    const char *tname = type_name(result);
    (void)tname; /* Suppress unused warning */
    
    return result;
}

/* Function that might trigger error recovery paths */
#ifdef TEST_ERROR_PATH
void error_path_test(void) {
    void foo(void);  /* Declaration with void parameter list */
    
    /* This should cause an error but might create interesting tree nodes */
    /* foo(1); */  /* Uncomment to test error recovery */
}
#endif

/* Main function combining all patterns */
int main(void) {
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Test various constructs */
    process_array(arr);
    
    int x = complex_expression(5, 3.14f);
    printf("Complex expression result: %d\n", x);
    
    /* Test statement expression */
    int y = statement_expr_func(10);
    printf("Statement expr result: %d\n", y);
    
    /* Test type-safe add macro */
    int a = 5, b = 3;
    float c = 2.5f;
    printf("Type-safe add (int+int): %d\n", TYPE_SAFE_ADD(a, b));
    printf("Type-safe add (int+float): %d\n", TYPE_SAFE_ADD(a, c));
    
    /* Test switch with ranges */
    switch_with_ranges(3);
    switch_with_ranges(15);
    switch_with_ranges(25);
    
    /* Test OpenMP */
    omp_test(100);
    
    /* Test _Generic */
    printf("Type of 42: %s\n", type_name(42));
    printf("Type of 3.14f: %s\n", type_name(3.14f));
    
    /* Use decimal literal macro */
    unsigned long big = DECIMAL_LITERAL(1000000);
    printf("Big number: %lu\n", big);
    
    /* Call unused function to ensure it's in AST */
    unused_func(0);
    
    printf("Done\n");
    return 0;
}
