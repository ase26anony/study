/* tree_coverage_test.c - Test program to exercise GCC's internal tree representation */
/* Compile with: gcc -O1 -fopenmp -std=gnu11 -fdump-tree-all -o test tree_coverage_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with statement expressions */
static int builtin_choose_test(int x) {
    /* This creates specialized expression nodes */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        ({ int y = x * 2; y + 1; }),
        ({ int y = x / 2; y - 1; })
    );
}

/* Function with GNU statement expression in return */
static inline int statement_expr_func(int a) {
    return ({
        typeof(a) tmp = a;
        int result = 0;
        for (int i = 0; i < 3; i++) {
            result += tmp;
            tmp *= 2;
        }
        result;
    });
}

/* Function using __builtin_types_compatible_p in complex expression */
static int type_compatible_test(void *ptr) {
    return __builtin_types_compatible_p(typeof(ptr), void*) 
           ? *((int*)ptr) 
           : __builtin_types_compatible_p(typeof(ptr), int*)
             ? *((int*)ptr) * 2
             : 0;
}

/* Function with array parameter using static qualifier */
static int static_array_test(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function using _Generic selector (C11) */
#define get_type_name(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char*: "string", \
    default: "unknown" \
)

/* Function with switch case ranges */
static int case_range_test(int val) {
    switch (val) {
        case 1 ... 5:
            return val * 10;
        case 10 ... 20:
            return val + 100;
        default:
            return -val;
    }
}

/* Function with attributes in unusual places */
static int attribute_test(void) {
    int i __attribute__((unused)) = 42;
    int j __attribute__((aligned(16))) = 24;
    return i + j;
}

/* OpenMP test function */
static void omp_test(void) {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        sum += i;
    }
    printf("OpenMP sum: %d\n", sum);
    
    /* OpenMP target directive - may create distinct tree nodes */
    #pragma omp target teams distribute parallel for map(tofrom:sum)
    for (int i = 0; i < 50; i++) {
        sum -= i;
    }
}

/* Function that might trigger error recovery paths if called wrong */
static void void_func(void) {
    printf("Void function called\n");
}

/* Main function combining all constructs */
int main(void) {
    int result = 0;
    
    /* Test builtin choose with statement expressions */
    result += builtin_choose_test(10);
    printf("builtin_choose_test: %d\n", result);
    
    /* Test statement expression function */
    result += statement_expr_func(5);
    printf("statement_expr_func: %d\n", result);
    
    /* Test type compatible expressions */
    int x = 42;
    result += type_compatible_test(&x);
    printf("type_compatible_test: %d\n", result);
    
    /* Test static array parameter */
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    result += static_array_test(arr);
    printf("static_array_test: %d\n", result);
    
    /* Test _Generic selector */
    const char *type_name = get_type_name(3.14);
    printf("Type name: %s\n", type_name);
    
    /* Test case ranges */
    result += case_range_test(3);
    result += case_range_test(15);
    printf("case_range_test: %d\n", result);
    
    /* Test attribute usage */
    result += attribute_test();
    printf("attribute_test: %d\n", result);
    
    /* Test OpenMP constructs */
    omp_test();
    
    /* Test error path (commented by default) */
    #ifdef TEST_ERROR_PATH
    /* This would normally be a compile error but might trigger 
       internal error recovery nodes during parsing */
    void_func(123);  /* Wrong number of arguments for void function */
    #endif
    
    /* Use __builtin_constant_p in a complex constant expression */
    int dynamic_val = result;
    if (__builtin_constant_p(0)) {
        /* This branch uses constant folding nodes */
        dynamic_val += 1000;
    }
    
    /* Final print to prevent dead code elimination */
    printf("Final result: %d\n", dynamic_val);
    
    /* Additional GNU extensions */
    /* Using __auto_type for type inference */
    __auto_type auto_var = &result;
    *auto_var += 1;
    
    /* Using compound literals with attributes */
    struct point {
        int x;
        int y;
    } *p = &((struct point){ .x = 1, .y = 2 });
    
    /* Using __builtin_va_arg_pack() for variable arguments */
    #define debug_print(...) \
        printf("Debug: " __VA_ARGS__)
    
    debug_print("Result is %d\n", result);
    
    return 0;
}
