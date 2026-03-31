/* test_tree_coverage.c */
/* Compile with: gcc -O0 -fopenmp -std=gnu11 -fdump-tree-all -o test test_tree_coverage.c */

#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with constant condition */
static int builtin_choose_test(int x) {
    /* This creates specialized expression nodes */
    return __builtin_choose_expr(1, x * 2, x / 2);
}

/* Statement expression in constant context */
#define COMPLEX_EXPR(x) ({ \
    typeof(x) _x = (x); \
    _x = __builtin_constant_p(_x) ? _x : (_x + 1); \
    _x * 2; \
})

/* Type-dependent expression using __builtin_types_compatible_p */
static int type_compatible_test(void *ptr) {
    /* May generate comparison nodes with uncommon tree codes */
    if (__builtin_types_compatible_p(typeof(ptr), void*))
        return 1;
    return 0;
}

/* Function with static array index qualifier */
static int static_array_test(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function using _Generic selection (C11) */
static const char* generic_selector_test(int x) {
    return _Generic(x,
        int: "int",
        long: "long",
        default: "unknown"
    );
}

/* Function with switch case ranges */
static int case_range_test(int x) {
    switch (x) {
        case 1 ... 5:
            return x * 2;
        case 6 ... 10:
            return x / 2;
        default:
            return x;
    }
}

/* Function with unusual attribute placement */
static int __attribute__((unused)) unused_func(int i __attribute__((unused))) {
    return 42;
}

/* OpenMP helper function */
static void omp_helper(int n) {
    int sum = 0;
    
    /* OpenMP parallel region - generates OMP_* tree nodes */
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i;
    }
    
    /* OpenMP target directive */
    #pragma omp target teams distribute parallel for map(tofrom:sum)
    for (int i = 0; i < n; i++) {
        sum += i * 2;
    }
    
    printf("OpenMP sum: %d\n", sum);
}

/* Function that might trigger error recovery paths */
#ifdef TEST_ERROR_PATH
static void error_path_test(void) {
    void foo(void);  /* Declaration with void parameter list */
    
    /* This should cause an error but might generate special tree nodes */
    foo(1);  /* Calling void function with argument */
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    /* Test various GNU extensions */
    result += builtin_choose_test(5);
    result += COMPLEX_EXPR(10);
    result += type_compatible_test(&result);
    
    /* Test static array qualifier */
    result += static_array_test(arr);
    
    /* Test _Generic selector */
    const char *type_str = generic_selector_test(result);
    printf("Type: %s\n", type_str);
    
    /* Test case ranges */
    result += case_range_test(3);
    
    /* Test unused function with attributes */
    result += unused_func(0);
    
    /* Test OpenMP constructs */
    omp_helper(100);
    
    /* Additional complex expression using statement expression */
    int complex_val = ({
        int x = result;
        int y = __builtin_choose_expr(x > 0, x, -x);
        y * y;
    });
    
    /* Use __builtin_constant_p in a complex way */
    if (__builtin_constant_p(complex_val)) {
        printf("Constant: %d\n", complex_val);
    } else {
        printf("Not constant: %d\n", complex_val);
    }
    
    /* Mix with OpenMP atomic */
    #pragma omp parallel
    {
        #pragma omp atomic
        result++;
    }
    
    /* Try to create unusual tree nodes with nested builtins */
    int nested = __builtin_choose_expr(
        __builtin_types_compatible_p(typeof(result), int),
        __builtin_constant_p(result) ? result : 0,
        -1
    );
    
    printf("Final result: %d\n", result + nested);
    printf("Done\n");
    
    return 0;
}
