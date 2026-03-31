/* test_tree_coverage.c - Program to exercise GCC's internal tree node generation */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper using __builtin_choose_expr with statement expressions */
static inline int choose_expr_test(int x) {
    /* This creates complex expression trees during parsing */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        ({ int y = x * 2; y + 1; }),
        ({ int y = x / 2; y - 1; })
    );
}

/* Function with GNU statement expression and type compatibility check */
static int type_complexity_test(void) {
    int a = 5;
    /* Creates specialized comparison nodes */
    if (__builtin_types_compatible_p(typeof(a), int)) {
        return ({ 
            int b = a * 2; 
            __builtin_choose_expr(b > 0, b, -b); 
        });
    }
    return 0;
}

/* Function with array parameter using static qualifier */
int static_array_test(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function with switch case ranges */
static int switch_range_test(int val) {
    switch (val) {
        case 1 ... 5:
            return val * 2;
        case 6 ... 10:
            return val / 2;
        default:
            return -1;
    }
}

/* C11 _Generic selector */
#define TYPE_SELECTOR(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char *: "string", \
    default: "unknown" \
)

/* Function with attributes in unusual places */
int __attribute__((noinline)) 
attribute_test(int x __attribute__((unused))) {
    return x + 1;
}

/* OpenMP test function */
void omp_complex_test(int n) {
    int i;
    #pragma omp target teams distribute parallel for simd \
            map(tofrom: n) reduction(+:i)
    for (i = 0; i < n; i++) {
        /* Complex OpenMP region with multiple clauses */
        #pragma omp atomic
        n += i;
    }
    
    /* Nested OpenMP */
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int j = 0; j < 10; j++) {
            #pragma omp task
            {
                int k = j * 2;
                (void)k;
            }
        }
    }
}

/* Function that might trigger error recovery paths */
#ifdef TEST_ERROR_PATH
void error_path_test(void) {
    void foo(void);  /* Declaration with void parameter */
    /* This should cause an error but may generate special tree nodes */
    foo(1);  /* Invalid call - may create error recovery nodes */
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Test GNU extensions */
    result += choose_expr_test(10);
    result += type_complexity_test();
    
    /* Test array with static qualifier */
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    result += static_array_test(arr);
    
    /* Test switch with ranges */
    result += switch_range_test(3);
    result += switch_range_test(8);
    
    /* Test _Generic */
    int x = 5;
    double y = 3.14;
    const char *type_x = TYPE_SELECTOR(x);
    const char *type_y = TYPE_SELECTOR(y);
    printf("Types: %s, %s\n", type_x, type_y);
    
    /* Test attributes */
    result += attribute_test(result);
    
    /* Test OpenMP complex constructs */
    omp_complex_test(100);
    
    /* Additional complex expression using all features */
    int complex_result = ({
        int tmp = result;
        #pragma omp parallel for reduction(+:tmp)
        for (int i = 0; i < 100; i++) {
            tmp += __builtin_choose_expr(
                __builtin_constant_p(i),
                i * 2,
                switch_range_test(i % 12)
            );
        }
        tmp;
    });
    
    printf("Final result: %d\n", complex_result);
    
    /* Try to create unusual tree nodes with constant expressions */
    struct {
        int a : 3;
        int b : 5;
    } bitfield = {0};
    
    /* Access through pointer with offset */
    int *ptr = (int*)&bitfield;
    *ptr = __builtin_choose_expr(1, 42, 0);
    
    /* Use offsetof with non-constant expression */
    size_t offset = __builtin_offsetof(typeof(bitfield), b);
    printf("Offset: %zu\n", offset);
    
    return 0;
}

/* Additional C++ specific code if compiled as C++ */
#ifdef __cplusplus
#include <type_traits>

template<typename T>
auto trailing_return_test(T x) -> decltype(x * 2) {
    return x * 2;
}

void cpp_specific_tests() {
    /* nullptr creates specific tree nodes */
    int* ptr = nullptr;
    
    /* User-defined literal (C++11) */
    auto val = 123_myliteral;  // Would need literal operator
    
    /* Template with dependent types */
    auto result = trailing_return_test(3.14);
    (void)result;
    (void)ptr;
}
#endif
