/* test_tree_codes.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* GNU extensions creating complex tree nodes */
#define CHOOSE_EXPR(cond, a, b) __builtin_choose_expr(cond, a, b)
#define TYPE_COMPAT(a, b) __builtin_types_compatible_p(a, b)
#define IS_CONSTANT(e) __builtin_constant_p(e)

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

/* Function with static array parameter (creates specialized tree nodes) */
int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function using __builtin_choose_expr in type-dependent context */
static int builtin_choose_test(int x) {
    return CHOOSE_EXPR(
        TYPE_COMPAT(typeof(x), int),
        x * 3,
        ({ 
            float f = x * 1.5f;
            (int)f;
        })
    );
}

/* Function with attribute in unusual place */
int attributed_var __attribute__((unused)) = 
    ({ 
        int x = 42;
        x; 
    });

/* C11 _Generic selector (creates distinct tree node) */
#define type_to_string(x) _Generic((x), \
    int: "int", \
    float: "float", \
    double: "double", \
    default: "unknown" \
)

/* Switch with case ranges (parsed into specific internal representation) */
int switch_with_ranges(int x) {
    switch (x) {
        case 1 ... 5:
            return x * 2;
        case 6 ... 10:
            return x / 2;
        case 11 ... 20:
            return x + 100;
        default:
            return -1;
    }
}

/* Function with void parameter - for potential error path */
#ifdef TEST_ERROR_PATH
void void_func(void) {
    printf("Void function\n");
}
#endif

/* OpenMP test function - generates OMP_* tree nodes */
void omp_test(int n) {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i * i;
    }
    printf("OMP sum: %d\n", sum);
    
    /* OpenMP target directive - may create different tree nodes */
    #pragma omp target teams distribute parallel for map(tofrom:sum)
    for (int i = 0; i < n; i++) {
        sum += i;
    }
}

/* Complex expression mixing multiple extensions */
int complex_expression(int a, int b) {
    return ({
        /* Use _Generic in statement expression */
        const char* type_name = type_to_string(a);
        
        /* Nested __builtin_choose_expr */
        int result = CHOOSE_EXPR(
            a > 0,
            CHOOSE_EXPR(
                TYPE_COMPAT(typeof(b), int),
                a + b,
                a - b
            ),
            ({ 
                /* Statement expr with switch ranges */
                int x = switch_with_ranges(a);
                x * b;
            })
        );
        
        /* Attribute on local variable */
        int __attribute__((unused)) temp = result;
        
        result;
    });
}

/* Main function combining all patterns */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Test statement expressions */
    result += stmt_expr_test(10);
    printf("stmt_expr_test: %d\n", result);
    
    /* Test builtin choose */
    result += builtin_choose_test(20);
    printf("builtin_choose_test: %d\n", result);
    
    /* Test static array parameter */
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    result += static_array_param(arr);
    printf("static_array_param: %d\n", result);
    
    /* Test switch with ranges */
    result += switch_with_ranges(7);
    printf("switch_with_ranges: %d\n", result);
    
    /* Test _Generic selector */
    const char* int_type = type_to_string(result);
    printf("Type of result: %s\n", int_type);
    
    /* Test complex expression */
    result += complex_expression(5, 3);
    printf("complex_expression: %d\n", result);
    
    /* Test OpenMP constructs */
    omp_test(100);
    
    /* Potential error path (commented by default) */
    #ifdef TEST_ERROR_PATH
    /* This would generate an error but might create interesting tree nodes */
    void_func(42);  /* Calling void function with argument */
    #endif
    
    /* Use attribute in main */
    int __attribute__((unused)) unused_var = result;
    
    printf("Final result: %d\n", result);
    printf("Done\n");
    
    return 0;
}

/* Additional C++ specific constructs (if compiled as C++) */
#ifdef __cplusplus
/* User-defined literal */
constexpr long double operator"" _deg(long double deg) {
    return deg * 3.14159265358979323846L / 180;
}

/* Template with trailing return type */
template<typename T, typename U>
auto template_add(T t, U u) -> decltype(t + u) {
    return t + u;
}

/* nullptr usage */
void* get_null() {
    return nullptr;
}
#endif
