/* test_tree_coverage.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* GNU extensions for complex tree nodes */
#define CHOOSE_EXPR(cond, expr1, expr2) \
    __builtin_choose_expr((cond), (expr1), (expr2))

/* Statement expression with type-dependent computation */
#define TYPE_DEPENDENT_EXPR(type, val) ({ \
    typeof(type) _result = (val); \
    __builtin_types_compatible_p(typeof(_result), int) ? _result * 2 : _result; \
    _result; \
})

/* Complex attribute usage */
static int __attribute__((unused, noinline)) 
attribute_func(int x __attribute__((unused))) {
    return x + 1;
}

/* Function with static array parameter (C99 feature) */
int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Function using __builtin_constant_p in type-dependent way */
inline int constant_p_test(int x) {
    if (__builtin_constant_p(x)) {
        return CHOOSE_EXPR(x > 0, x * 2, x / 2);
    }
    return TYPE_DEPENDENT_EXPR(x, x + 1);
}

/* C11 _Generic selector */
#define type_name(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char *: "string", \
    default: "unknown" \
)

/* Switch with case ranges (GNU extension) */
int switch_with_ranges(int val) {
    switch (val) {
        case 1 ... 5:
            return val * 10;
        case 6 ... 10:
            return val * 20;
        default:
            return val;
    }
}

/* Function with void parameter - for potential error paths */
#ifdef TEST_ERROR_PATH
void void_func(void) {
    printf("Void function\n");
}
#endif

/* OpenMP helper with complex computation */
void omp_computation(int n, int *result) {
    int sum = 0;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        /* Nested statement expression inside OpenMP region */
        int val = ({ 
            int tmp = i;
            __builtin_constant_p(i) ? tmp : tmp * 2; 
        });
        sum += val;
    }
    
    *result = sum;
}

/* Main function combining all patterns */
int main(void) {
    int result = 0;
    
    /* Test GNU extensions */
    int x = 42;
    int y = CHOOSE_EXPR(1, x, 0);
    printf("Choose expr result: %d\n", y);
    
    /* Test type-dependent expression */
    int z = TYPE_DEPENDENT_EXPR(x, 10);
    printf("Type dependent: %d\n", z);
    
    /* Test __builtin_constant_p */
    int a = constant_p_test(5);
    printf("Constant_p test: %d\n", a);
    
    /* Test _Generic selector */
    printf("Type of 42: %s\n", type_name(42));
    printf("Type of 3.14: %s\n", type_name(3.14));
    
    /* Test switch with ranges */
    int b = switch_with_ranges(3);
    printf("Switch range: %d\n", b);
    
    /* Test static array parameter */
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    int c = static_array_param(arr);
    printf("Static array sum: %d\n", c);
    
    /* Test OpenMP computation */
    omp_computation(100, &result);
    printf("OpenMP result: %d\n", result);
    
    /* Test attribute function */
    int d = attribute_func(20);
    printf("Attribute func: %d\n", d);
    
    /* Potential error path (commented by default) */
    #ifdef TEST_ERROR_PATH
    /* This would generate an error but might create interesting tree nodes */
    void_func(1);  /* Calling void function with argument */
    #endif
    
    /* Additional complex expression mixing multiple extensions */
    int final = ({
        int tmp = 0;
        #pragma omp parallel for reduction(+:tmp)
        for (int i = 0; i < 10; i++) {
            tmp += CHOOSE_EXPR(i % 2 == 0, i, -i);
        }
        tmp;
    });
    printf("Final mixed result: %d\n", final);
    
    /* Use OpenMP target for additional tree nodes */
    #pragma omp target teams distribute parallel for map(tofrom: result)
    for (int i = 0; i < 50; i++) {
        result += i;
    }
    printf("Target result: %d\n", result);
    
    /* Complex nested _Generic with statement expressions */
    auto complex_type = ({
        double dval = 3.14159;
        _Generic(dval, 
            double: "double type",
            float: "float type",
            default: "other"
        );
    });
    printf("Complex type: %s\n", complex_type);
    
    printf("Done\n");
    return 0;
}
