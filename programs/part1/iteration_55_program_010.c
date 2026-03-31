/* tree_coverage_test.c - Test program to exercise GCC's tree_code_kind default case */
/* Compile with: gcc -O0 -fopenmp -std=gnu11 -fdump-tree-all -o test tree_coverage_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with statement expressions */
static inline int builtin_choose_expr_test(int x) {
    /* This creates complex expression trees */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        ({ int y = x * 2; y + 1; }),
        ({ int y = x / 2; y - 1; })
    );
}

/* Function with static array parameter (creates specialized tree nodes) */
void static_array_param(int arr[static 10]) {
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 2;
    }
}

/* Function using __builtin_types_compatible_p in type-dependent expression */
#define TYPE_SAFE_ADD(a, b) \
    __builtin_choose_expr( \
        __builtin_types_compatible_p(typeof(a), typeof(b)), \
        (a) + (b), \
        (__builtin_choose_expr( \
            __builtin_types_compatible_p(typeof(a), int), \
            (int)(a) + (int)(b), \
            (double)(a) + (double)(b) \
        )) \
    )

/* Function with GNU statement expression and attributes */
int __attribute__((noinline)) statement_expr_test(void) {
    return ({
        int __attribute__((unused)) unused_var = 42;
        int result = 0;
        for (int i = 0; i < 10; i++) {
            result += builtin_choose_expr_test(i);
        }
        result;
    });
}

/* Use _Generic selection (creates distinct tree node) */
#define get_type_name(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char *: "string", \
    default: "unknown" \
)

/* Function with case ranges (parsed into specific internal representation) */
int case_range_test(int x) {
    switch (x) {
        case 1 ... 5:
            return x * 2;
        case 6 ... 10:
            return x / 2;
        case 100 ... 200:
            return x + 100;
        default:
            return -1;
    }
}

/* OpenMP test function - generates OMP_* tree nodes */
void omp_test(int n) {
    int sum = 0;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i;
    }
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < n; i++) {
        /* Empty - just for tree generation */
    }
    
    printf("OpenMP sum: %d\n", sum);
}

/* Function with void parameter - for potential error recovery paths */
#ifdef TEST_ERROR_PATH
void void_func(void) {
    printf("Void function\n");
}
#endif

/* Main function combining all constructs */
int main(void) {
    int result = 0;
    
    /* Test builtin_choose_expr with statement expressions */
    result = builtin_choose_expr_test(10);
    printf("builtin_choose_expr_test: %d\n", result);
    
    /* Test static array parameter */
    int arr[10];
    static_array_param(arr);
    
    /* Test TYPE_SAFE_ADD macro */
    int a = 5;
    double b = 3.14;
    printf("TYPE_SAFE_ADD int+double: %f\n", (double)TYPE_SAFE_ADD(a, (int)b));
    
    /* Test statement expression with attributes */
    result = statement_expr_test();
    printf("statement_expr_test: %d\n", result);
    
    /* Test _Generic selection */
    printf("Type of 42: %s\n", get_type_name(42));
    printf("Type of 3.14: %s\n", get_type_name(3.14));
    
    /* Test case ranges */
    result = case_range_test(3);
    printf("case_range_test(3): %d\n", result);
    result = case_range_test(150);
    printf("case_range_test(150): %d\n", result);
    
    /* Test OpenMP constructs */
    omp_test(100);
    
    /* Potentially problematic code for error recovery */
    #ifdef TEST_ERROR_PATH
    /* This would normally cause an error but might trigger special tree nodes */
    void_func(42);  /* Calling void function with argument */
    #endif
    
    /* Use __builtin_constant_p in complex context */
    int dynamic_val = rand() % 100;
    int choice = __builtin_choose_expr(
        __builtin_constant_p(dynamic_val),
        1,
        __builtin_choose_expr(
            __builtin_types_compatible_p(typeof(dynamic_val), int),
            2,
            3
        )
    );
    printf("Dynamic choice: %d\n", choice);
    
    /* Array with static index in parameter (GNU extension) */
    void process_array(int size, int array[static size]) {
        for (int i = 0; i < size; i++) {
            array[i] *= 2;
        }
    }
    
    int local_arr[5] = {1, 2, 3, 4, 5};
    process_array(5, local_arr);
    
    printf("Final array[0]: %d\n", local_arr[0]);
    
    /* Additional OpenMP construct with clauses */
    int x = 0;
    #pragma omp parallel private(x)
    {
        x = omp_get_thread_num();
        #pragma omp critical
        printf("Thread %d\n", x);
    }
    
    printf("Done\n");
    return 0;
}
