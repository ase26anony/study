/* test_tree_coverage.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* GNU extensions creating complex tree nodes */
#define CHOOSE_EXPR(cond, expr1, expr2) \
    __builtin_choose_expr((cond), (expr1), (expr2))

/* Statement expression in constant context */
static inline int stmt_expr_test(int x) {
    return ({ 
        int y = x * 2; 
        typeof(y) z = y + 1; 
        z; 
    });
}

/* Type-dependent expression with builtins */
static int type_dependent_expr(void) {
    return __builtin_types_compatible_p(int, long) ? 1 : 0;
}

/* Function with static array parameter (C99 feature) */
void static_array_param(int arr[static 10]) {
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 2;
    }
}

/* Function with GNU attributes in unusual places */
int __attribute__((always_inline)) 
inline_with_attrs(int x) 
    __attribute__((const));

int inline_with_attrs(int x) {
    return x * 3;
}

/* C11 _Generic selector */
#define type_name(x) _Generic((x), \
    int: "int", \
    double: "double", \
    default: "unknown" \
)

/* Switch with case ranges (GNU extension) */
int switch_with_ranges(int val) {
    switch (val) {
        case 1 ... 5:
            return 1;
        case 6 ... 10:
            return 2;
        default:
            return 0;
    }
}

/* Function that might generate error recovery nodes */
#ifdef TEST_ERROR_PATH
void void_func(void);
#endif

/* OpenMP helper function */
void omp_helper(int n, int *result) {
    int sum = 0;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i * i;
    }
    
    *result = sum;
}

/* OpenACC helper (if supported) */
#ifdef _OPENACC
void acc_helper(int n, int *result) {
    int sum = 0;
    
    #pragma acc parallel loop reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i;
    }
    
    *result = sum;
}
#endif

/* Complex expression using multiple extensions */
static int complex_expression(int a, int b) {
    return CHOOSE_EXPR(
        __builtin_constant_p(a),
        stmt_expr_test(a),
        ({ 
            int temp = a + b; 
            temp * temp - b; 
        })
    );
}

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Test statement expressions */
    result += stmt_expr_test(5);
    
    /* Test type-dependent expressions */
    result += type_dependent_expr();
    
    /* Test static array parameter */
    int arr[10];
    static_array_param(arr);
    result += arr[3];
    
    /* Test inline function with attributes */
    result += inline_with_attrs(7);
    
    /* Test _Generic selector */
    const char *name = type_name(3.14);
    if (name[0] == 'd') result += 1;
    
    /* Test switch with ranges */
    result += switch_with_ranges(3);
    
    /* Test complex expression */
    result += complex_expression(2, 3);
    
    /* Test OpenMP */
    int omp_result;
    omp_helper(100, &omp_result);
    result += omp_result % 100;
    
    /* Test OpenACC if available */
    #ifdef _OPENACC
    int acc_result;
    acc_helper(50, &acc_result);
    result += acc_result % 50;
    #endif
    
    /* Potentially problematic code for error recovery */
    #ifdef TEST_ERROR_PATH
    /* This should generate a warning/error but compile */
    void_func(42);  /* Calling void function with argument */
    #endif
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional OpenMP constructs for more tree nodes */
    #pragma omp target teams distribute parallel for simd
    for (int i = 0; i < 10; i++) {
        arr[i] += i;
    }
    
    /* Nested OpenMP pragmas */
    #pragma omp parallel
    {
        #pragma omp master
        {
            printf("OpenMP master thread\n");
        }
        
        #pragma omp barrier
        
        #pragma omp for nowait
        for (int i = 0; i < 5; i++) {
            /* empty */
        }
    }
    
    /* Variable-length array (C99) */
    int vla_size = result % 10 + 1;
    int vla[vla_size];
    for (int i = 0; i < vla_size; i++) {
        vla[i] = i * i;
    }
    
    /* Designated initializers */
    struct point {
        int x, y, z;
    } p = { .y = 2, .z = 3, .x = 1 };
    
    /* Compound literals */
    int *ptr = (int[]){1, 2, 3, 4, 5};
    
    printf("Done\n");
    return 0;
}

/* Additional test functions for more coverage */

/* Function with GNU typeof in parameter */
void typeof_param(typeof(int) x, typeof(double) y) {
    printf("Types: %d, %f\n", x, y);
}

/* Alignment attribute */
int __attribute__((aligned(32))) aligned_var = 42;

/* Transparent union (GNU extension) */
typedef union __attribute__((transparent_union)) {
    int *intp;
    void *voidp;
} transparent_union_t;

/* Cleanup attribute */
void cleanup_func(int *p) {
    printf("Cleaning up %p\n", (void*)p);
}

void test_cleanup(void) {
    int x __attribute__((cleanup(cleanup_func))) = 10;
    x = 20;
}
