/* Compile with: gcc -O0 -fopenmp -std=gnu11 -fdump-tree-all -o test_tree test_tree.c */
/* Also try: gcc -O2 -fopenmp -std=gnu++17 -fdump-tree-optimized -xc++ test_tree.c */

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
    __builtin_constant_p(_x) ? _x * 3 : _x + 5; \
})

/* Type-dependent expression using __builtin_types_compatible_p */
static int type_compatible_test(void) {
    int (*fp)(void) = NULL;
    /* Creates comparison nodes for type compatibility checks */
    if (__builtin_types_compatible_p(typeof(fp), void*))
        return 1;
    return 0;
}

/* Function with static array index parameter (C99 feature) */
static int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function using _Generic selector (C11) */
static const char* generic_selector_test(int x) {
    return _Generic(x,
        int: "integer",
        float: "float",
        double: "double",
        default: "unknown"
    );
}

/* Function with GNU attributes in unusual places */
static int __attribute__((always_inline)) 
inline_func(int x __attribute__((unused))) 
    __attribute__((warn_unused_result));
    
static int inline_func(int x) {
    return x + 1;
}

/* OpenMP helper function */
static void omp_test(void) {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        sum += i;
    }
    printf("OpenMP sum: %d\n", sum);
}

/* Function with switch case ranges (GNU extension) */
static int switch_case_range(int x) {
    switch (x) {
        case 1 ... 5:
            return x * 2;
        case 6 ... 10:
            return x / 2;
        default:
            return x;
    }
}

/* C++ style nullptr if compiled as C++ */
#ifdef __cplusplus
static void nullptr_test(void* ptr = nullptr) {
    if (ptr == nullptr) {
        printf("nullptr detected\n");
    }
}
#else
static void nullptr_test(void* ptr) {
    if (ptr == NULL) {
        printf("NULL detected\n");
    }
}
#endif

/* OpenACC pragma (if supported) */
static void acc_test(void) {
    int n = 100;
    float* data = (float*)malloc(n * sizeof(float));
    
    #ifdef _OPENACC
    #pragma acc parallel loop copyout(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i * 0.5f;
    }
    #endif
    
    free(data);
}

/* Complex constructor with designated initializers */
struct complex_struct {
    int a;
    int b[3];
    struct {
        float x;
        float y;
    } point;
};

static struct complex_struct make_complex(void) {
    /* Designated initializers can create complex tree nodes */
    return (struct complex_struct){
        .a = 1,
        .b = {2, 3, 4},
        .point = {.x = 1.0f, .y = 2.0f}
    };
}

/* Function that might trigger error recovery paths */
#ifdef TEST_ERROR_PATH
static void error_path_test(void) {
    /* Declare function with void parameter list */
    void foo(void);
    
    /* This should cause an error but might generate special tree nodes */
    /* foo(1); */  /* Uncomment to test error recovery */
    
    /* Array with static storage and const qualifier in parameter */
    static const int arr[5] = {1, 2, 3, 4, 5};
    int result = static_array_param((int*)arr);  /* Cast to avoid type error */
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Test builtin choose expression */
    result += builtin_choose_test(10);
    
    /* Test statement expression macro */
    result += COMPLEX_EXPR(5);
    
    /* Test type compatibility */
    result += type_compatible_test();
    
    /* Test static array parameter */
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    result += static_array_param(arr);
    
    /* Test _Generic selector */
    const char* type_str = generic_selector_test(42);
    printf("Type: %s\n", type_str);
    
    /* Test inline function with attributes */
    result += inline_func(5);
    
    /* Test OpenMP */
    omp_test();
    
    /* Test switch case ranges */
    result += switch_case_range(3);
    result += switch_case_range(8);
    
    /* Test nullptr/NULL */
    nullptr_test(NULL);
    
    /* Test OpenACC if available */
    acc_test();
    
    /* Test complex constructor */
    struct complex_struct cs = make_complex();
    result += cs.a + cs.b[0];
    
    /* Test error path if enabled */
    #ifdef TEST_ERROR_PATH
    error_path_test();
    #endif
    
    /* Additional GNU extensions */
    
    /* Binary constants (GNU extension) */
    int binary_val = 0b101010;
    result += binary_val;
    
    /* Nested functions (GNU extension) - may create unique tree nodes */
    auto int nested_func(int y) { return y * 2; }
    result += nested_func(5);
    
    /* Aligned attribute */
    int __attribute__((aligned(32))) aligned_var = 42;
    result += aligned_var;
    
    /* Cleanup attribute */
    char* buffer __attribute__((cleanup(free))) = malloc(100);
    if (buffer) {
        buffer[0] = 'A';
    }
    
    /* Vector extensions (GNU extension) */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    result += vec3[0];
    
    /* Transparent union attribute */
    union __attribute__((transparent_union)) transparent_union {
        int i;
        float f;
    } tu = {.i = 42};
    
    printf("Final result: %d\n", result);
    printf("Done\n");
    
    return 0;
}
