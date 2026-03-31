/* tree_coverage_test.c - Test program to exercise GCC's internal tree representation */
#include <stdio.h>
#include <stdlib.h>

/* GNU extensions to create complex tree nodes */
#define CHOOSE_EXPR(cond, a, b) __builtin_choose_expr((cond), (a), (b))
#define TYPE_COMPAT(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#define IS_CONSTANT(expr) __builtin_constant_p(expr)

/* Statement expression creating complex tree structure */
#define COMPLEX_EXPR(x) ({ \
    typeof(x) _x = (x); \
    int _y = IS_CONSTANT(_x) ? 42 : 17; \
    _x + _y; \
})

/* Function with static array parameter (creates specialized tree nodes) */
static int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function using __builtin_choose_expr in type-dependent context */
static inline int choose_based_on_type(int x, double y) {
    return CHOOSE_EXPR(
        TYPE_COMPAT(x, int),
        x * 2,
        (int)(y * 3.0)
    );
}

/* Function with attribute in unusual place */
static void __attribute__((always_inline)) 
inline_func_with_attrs(int i __attribute__((unused))) {
    /* Using case ranges - creates specific internal representation */
    switch (i) {
        case 1 ... 5:
            printf("Range 1-5\n");
            break;
        case 6 ... 10:
            printf("Range 6-10\n");
            break;
        default:
            printf("Other\n");
    }
}

/* _Generic selector expression (C11) - creates distinct tree node */
#define TYPE_NAME(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char *: "string", \
    default: "unknown" \
)

/* Complex expression mixing multiple GNU extensions */
static int complex_gnu_expr(int a, int b) {
    return COMPLEX_EXPR(
        CHOOSE_EXPR(
            IS_CONSTANT(a + b),
            a * b,
            ({ 
                int temp = a > b ? a : b;
                temp * temp;
            })
        )
    );
}

/* OpenMP test function - generates OMP_* tree nodes */
static void omp_test(int n) {
    int sum = 0;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i * i;
    }
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < n; i++) {
        sum += i;
    }
    
    printf("OpenMP sum: %d\n", sum);
}

/* Function that might trigger error recovery paths */
#ifdef TEST_ERROR_PATH
static void potential_error_path(void) {
    void foo(void);  /* Declaration with void parameter list */
    
    /* This should cause an error but might generate special tree nodes */
    foo(1);  /* Calling void function with argument */
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result;
    
    /* Test static array parameter */
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    result = static_array_param(arr);
    printf("Static array result: %d\n", result);
    
    /* Test GNU extensions */
    result = choose_based_on_type(5, 2.5);
    printf("Choose expr result: %d\n", result);
    
    /* Test inline function with attributes */
    inline_func_with_attrs(3);
    inline_func_with_attrs(7);
    
    /* Test _Generic selector */
    printf("Type of 42: %s\n", TYPE_NAME(42));
    printf("Type of 3.14: %s\n", TYPE_NAME(3.14));
    
    /* Test complex GNU expression */
    result = complex_gnu_expr(3, 4);
    printf("Complex expr result: %d\n", result);
    
    /* Test OpenMP */
    omp_test(100);
    
    /* Test statement expression */
    int x = 10;
    int y = COMPLEX_EXPR(x);
    printf("Statement expr result: %d\n", y);
    
    /* Use __builtin_constant_p in various contexts */
    if (IS_CONSTANT(2 + 2)) {
        printf("Constant expression detected\n");
    }
    
    /* Array with static qualifier in parameter (another variant) */
    void process_array(int m, int n, int matrix[static m][n]) {
        printf("Processing %dx%d matrix\n", m, n);
    }
    
    int matrix[5][5];
    process_array(5, 5, matrix);
    
    /* Final output to prevent dead code elimination */
    printf("Test completed successfully\n");
    
    return 0;
}

/* Additional C++ specific constructs if compiled as C++ */
#ifdef __cplusplus
// User-defined literal
constexpr long double operator"" _deg(long double deg) {
    return deg * 3.14159265358979323846L / 180.0L;
}

// Template with trailing return type
template<typename T, typename U>
auto add(T t, U u) -> decltype(t + u) {
    return t + u;
}

// nullptr usage
void* get_null() {
    return nullptr;
}
#endif
