/* Compile with: gcc -O0 -fdump-tree-original -fdump-tree-gimple -fopenmp -std=gnu11 -Wno-unused-variable -Wno-unused-function tree_test.c -o tree_test */

#include <stdio.h>
#include <stdlib.h>

/* Helper functions using GNU extensions */
static inline int use_builtin_choose(void) {
    /* __builtin_choose_expr creates specialized expression nodes */
    return __builtin_choose_expr(1, 42, 0);
}

static int use_statement_expr(int x) {
    /* Statement expression in constant context */
    int y = ({ 
        int z = x * 2; 
        z + 1; 
    });
    return y;
}

static int use_type_compatible(int x) {
    /* __builtin_types_compatible_p creates comparison nodes */
    if (__builtin_types_compatible_p(typeof(x), int)) {
        return x * 2;
    }
    return x;
}

/* Function with static array parameter - creates specialized tree nodes */
static int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function with GNU attributes in unusual places */
static void __attribute__((always_inline)) 
attributed_function(int i __attribute__((unused))) {
    /* Multiple attributes on variables */
    int j __attribute__((aligned(16))) = 0;
    volatile int k __attribute__((deprecated)) = 1;
}

/* Use _Generic selection expression (C11) */
#define type_to_string(x) _Generic((x), \
    int: "int", \
    double: "double", \
    default: "unknown" \
)

/* Use case ranges in switch (GNU extension) */
static int case_ranges(int x) {
    switch (x) {
        case 1 ... 5:
            return x * 2;
        case 6 ... 10:
            return x / 2;
        default:
            return -1;
    }
}

/* OpenMP function to generate OpenMP tree nodes */
static void openmp_test(void) {
    int sum = 0;
    int arr[100];
    
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel for - generates OMP_PARALLEL, OMP_FOR nodes */
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* OpenMP target directive - may generate distinct tree nodes */
    #pragma omp target teams distribute parallel for map(tofrom:sum)
    for (int i = 0; i < 50; i++) {
        sum -= i;
    }
    
    printf("OpenMP sum: %d\n", sum);
}

/* Function that might trigger error recovery paths */
#ifdef TEST_ERROR_PATH
static void test_error_paths(void) {
    /* Declare function with void parameter */
    void foo(void);
    
    /* This would normally be an error but might generate special nodes */
    /* Note: This is commented out to keep compilation working */
    /* foo(1); */  /* Would trigger error recovery */
    
    /* Use offsetof with non-standard types */
    struct weird {
        int a;
        int b[];
    };
    
    /* __builtin_constant_p in complex expression */
    int x = 10;
    if (__builtin_constant_p(x + __builtin_constant_p(x))) {
        x = 20;
    }
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Use GNU extensions */
    result += use_builtin_choose();
    result += use_statement_expr(10);
    result += use_type_compatible(5);
    
    /* Use static array parameter */
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    result += static_array_param(arr);
    
    /* Use attributed function */
    attributed_function(result);
    
    /* Use _Generic selector */
    const char *type_str = type_to_string(result);
    printf("Type string: %s\n", type_str);
    
    /* Use case ranges */
    result += case_ranges(3);
    
    /* OpenMP test */
    openmp_test();
    
    /* Use __builtin_constant_p in type-dependent expression */
    int y = 0;
    if (__builtin_types_compatible_p(typeof(y), int) && 
        __builtin_constant_p(sizeof(int))) {
        y = 100;
    }
    
    /* Complex statement expression with nested builtins */
    int z = ({
        int tmp = __builtin_choose_expr(1, 42, 0);
        tmp + __builtin_constant_p(tmp) ? 1 : 0;
    });
    
    /* Try to create unusual tree nodes with computed goto (GNU extension) */
    void *label_ptr = &&done;
    static int counter = 0;
    
    if (counter++ < 1) {
        goto *label_ptr;
    }
    
done:
    /* Use OpenACC-like pragma (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel loop
    for (int i = 0; i < 10; i++) {
        result += i;
    }
    #endif
    
    /* Final print to prevent dead code elimination */
    printf("Final result: %d\n", result);
    
    return 0;
}

/* Additional C++-like constructs if compiled as C++ */
#ifdef __cplusplus
namespace test {
    /* User-defined literal (C++11) */
    constexpr long double operator"" _deg(long double deg) {
        return deg * 3.14159265358979323846L / 180;
    }
    
    /* nullptr usage */
    template<typename T>
    T* create() {
        return nullptr;
    }
}
#endif
