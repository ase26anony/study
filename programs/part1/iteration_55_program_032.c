/* test_tree_coverage.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* GNU extensions for complex tree nodes */
#define CHOOSE_EXPR(cond, a, b) __builtin_choose_expr(cond, a, b)
#define TYPE_COMPAT(a, b) __builtin_types_compatible_p(a, b)
#define IS_CONSTANT(expr) __builtin_constant_p(expr)

/* Statement expression generating complex tree structure */
static inline int stmt_expr_test(int x) {
    return ({
        int y = x * 2;
        int z;
        if (IS_CONSTANT(x))
            z = y + 1;
        else
            z = y - 1;
        z;
    });
}

/* Function with static array parameter (special tree node) */
void static_array_param(int arr[static 10]) {
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 2;
    }
}

/* Function using __builtin_choose_expr in type-dependent context */
static int choose_expr_test(int val) {
    return CHOOSE_EXPR(
        TYPE_COMPAT(typeof(val), int),
        val * 3,
        ({ double d = val; (int)(d * 2.5); })
    );
}

/* Function with attribute in unusual position */
int __attribute__((unused)) unused_var __attribute__((aligned(16))) = 42;

/* C11 _Generic selector (creates distinct tree node) */
#define type_name(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char *: "string", \
    default: "unknown" \
)

/* Function with trailing return type (C++-like, works in GNU C) */
auto trailing_return(int x) -> int {
    return x * 2;
}

/* Switch with case ranges (special internal representation) */
int switch_with_ranges(int x) {
    switch (x) {
        case 1 ... 5:
            return x * 10;
        case 10 ... 20:
            return x / 2;
        default:
            return -x;
    }
}

/* OpenMP helper function */
void omp_helper(int *data, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        data[i] = stmt_expr_test(data[i]);
    }
}

/* Function with void parameter - for error path testing */
#ifdef TEST_ERROR_PATH
void void_func(void) {
    printf("Void function\n");
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Test statement expressions */
    result += stmt_expr_test(5);
    
    /* Test __builtin_choose_expr */
    result += choose_expr_test(10);
    
    /* Test static array parameter */
    int arr[10];
    static_array_param(arr);
    result += arr[3];
    
    /* Test switch with ranges */
    result += switch_with_ranges(3);
    result += switch_with_ranges(15);
    
    /* Test _Generic selector */
    int x = 42;
    double y = 3.14;
    const char *type_x = type_name(x);
    const char *type_y = type_name(y);
    printf("Type of x: %s, Type of y: %s\n", type_x, type_y);
    
    /* Test trailing return type */
    result += trailing_return(7);
    
    /* OpenMP parallel region - generates OMP_* tree nodes */
    int omp_data[100];
    #pragma omp target teams distribute parallel for simd map(tofrom: omp_data)
    for (int i = 0; i < 100; i++) {
        omp_data[i] = i * i;
    }
    
    /* Process with OpenMP */
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < 100; i++) {
            result += omp_data[i] % 10;
        }
        
        /* Nested OpenMP construct */
        #pragma omp sections
        {
            #pragma omp section
            {
                result += 1;
            }
            #pragma omp section
            {
                result += 2;
            }
        }
    }
    
    /* Additional OpenMP construct */
    #pragma omp taskloop
    for (int i = 0; i < 50; i++) {
        omp_data[i] = choose_expr_test(omp_data[i]);
    }
    
    /* Test error path if enabled */
    #ifdef TEST_ERROR_PATH
    /* This should generate error recovery nodes */
    void_func(42);  /* Wrong number of arguments */
    #endif
    
    /* Complex expression mixing multiple extensions */
    int complex_result = ({
        int temp = 0;
        #pragma omp parallel for reduction(+:temp)
        for (int i = 0; i < 20; i++) {
            temp += CHOOSE_EXPR(
                i % 2 == 0,
                switch_with_ranges(i),
                stmt_expr_test(i)
            );
        }
        temp;
    });
    
    result += complex_result;
    
    /* Use attribute on statement (GNU extension) */
    if (result > 100) {
        printf("Result is large\n");
    } __attribute__((unused));
    
    printf("Final result: %d\n", result);
    printf("Done\n");
    
    return 0;
}

/* Additional constructs outside main */

/* OpenACC pragma (if supported) */
#ifdef _OPENACC
void acc_test(void) {
    int acc_arr[1000];
    #pragma acc parallel loop copyout(acc_arr)
    for (int i = 0; i < 1000; i++) {
        acc_arr[i] = i * 3;
    }
}
#endif

/* Function with array of pointers to functions */
typedef int (*func_ptr_t)(int);
static func_ptr_t func_array[] = {
    stmt_expr_test,
    choose_expr_test,
    switch_with_ranges,
    NULL
};

/* Designated initializers with ranges (GCC extension) */
struct sparse_array {
    int values[100];
};

static struct sparse_array sparse = {
    .values = {
        [0 ... 9] = 1,
        [10 ... 19] = 2,
        [50 ... 59] = 3,
        [90 ... 99] = 4
    }
};
