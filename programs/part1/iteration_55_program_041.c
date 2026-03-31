/* tree_code_kind_test.c */
/* Compile with: gcc -O0 -fdump-tree-all -fopenmp -std=gnu11 -Wall tree_code_kind_test.c -o test */

#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with statement expressions */
static int choose_expr_test(int x) {
    /* This creates complex tree nodes during parsing */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        ({ int y = x * 2; y + 1; }),
        ({ int y = x / 2; y - 1; })
    );
}

/* Function with GNU statement expression and __builtin_types_compatible_p */
static inline auto_type_test(void) {
    /* Using typeof with statement expression */
    typeof(({ static int counter = 0; ++counter; })) result = 42;
    return result;
}

/* Function with array parameter using static qualifier (creates specialized tree nodes) */
static int static_array_test(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function using _Generic selector (creates distinct tree node type) */
#define get_type_name(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char *: "string", \
    default: "unknown" \
)

/* Function with switch case ranges (parsed into specific internal representation) */
static int case_range_test(int val) {
    switch (val) {
        case 1 ... 5:
            return val * 2;
        case 6 ... 10:
            return val / 2;
        default:
            return -1;
    }
}

/* Function with attributes in unusual places */
static int __attribute__((always_inline)) inline_attr_test(int x) 
    __attribute__((warn_unused_result));
    
static int inline_attr_test(int x) {
    int __attribute__((unused)) unused_var = x * 2;
    return x + 1;
}

/* OpenMP test function - targeting non-clause OpenMP tree nodes */
static void omp_test(void) {
    int i, sum = 0;
    int arr[100];
    
    for (i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel for - generates OMP_PARALLEL, OMP_FOR nodes */
    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* OpenMP target directive - may generate distinct tree nodes */
    #pragma omp target teams distribute parallel for map(tofrom:sum)
    for (i = 0; i < 50; i++) {
        sum -= i;
    }
    
    printf("OpenMP sum: %d\n", sum);
}

/* Function that would cause error recovery paths if called incorrectly */
static void void_func(void) {
    /* Empty void function */
}

/* Main function combining all constructs */
int main(void) {
    int result;
    
    /* Test GNU extensions */
    result = choose_expr_test(10);
    printf("choose_expr_test: %d\n", result);
    
    result = auto_type_test();
    printf("auto_type_test: %d\n", result);
    
    /* Test static array parameter */
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    result = static_array_test(arr);
    printf("static_array_test: %d\n", result);
    
    /* Test _Generic selector */
    const char *type_name = get_type_name(3.14);
    printf("Type name: %s\n", type_name);
    
    /* Test case ranges */
    result = case_range_test(3);
    printf("case_range_test(3): %d\n", result);
    
    result = case_range_test(8);
    printf("case_range_test(8): %d\n", result);
    
    /* Test function with attributes */
    result = inline_attr_test(42);
    printf("inline_attr_test: %d\n", result);
    
    /* Test OpenMP constructs */
    omp_test();
    
    /* Potentially problematic code that might trigger internal paths */
    #ifdef TEST_ERROR_PATH
    /* This would normally cause an error but might trigger special tree nodes */
    void_func(1);  /* Calling void function with argument */
    #endif
    
    /* Using __builtin_constant_p in complex expression */
    int x = 10;
    if (__builtin_constant_p(x) && __builtin_types_compatible_p(typeof(x), int)) {
        printf("x is constant int\n");
    }
    
    /* Compound literal with attributes */
    struct __attribute__((packed)) packed_struct {
        char a;
        int b;
    } ps = { .a = 'A', .b = 123 };
    
    printf("Packed struct: %c %d\n", ps.a, ps.b);
    
    /* Try to use OpenACC if supported (commented out as not always available) */
    /*
    #pragma acc parallel loop
    for (int i = 0; i < 10; i++) {
        arr[i] *= 2;
    }
    */
    
    printf("Done\n");
    return 0;
}
