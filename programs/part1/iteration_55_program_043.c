/* test_tree_codes.c */
#include <stdio.h>
#include <stdlib.h>

/* Helper 1: Uses __builtin_choose_expr with statement expressions */
static int choose_expr_test(int x) {
    /* This creates complex internal tree nodes during parsing */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        ({ int y = x * 2; y + 1; }),
        ({ int y = x / 2; y - 1; })
    );
}

/* Helper 2: Uses __builtin_types_compatible_p in type-dependent context */
#define TYPE_SAFE_ADD(a, b) \
    __builtin_choose_expr( \
        __builtin_types_compatible_p(typeof(a), typeof(b)), \
        (a) + (b), \
        (__typeof__(a))0 \
    )

/* Helper 3: Function with static array parameter (creates specialized tree) */
int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Helper 4: Uses _Generic selector (creates distinct tree node) */
#define GET_TYPE_NAME(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char*: "string", \
    default: "unknown" \
)

/* Helper 5: Function with GNU attributes in unusual places */
int __attribute__((always_inline)) 
inline_func(int x __attribute__((unused))) 
    __attribute__((warn_unused_result));
    
int inline_func(int x) {
    return x * 3;
}

/* Helper 6: Switch with case ranges (parsed into specific internal representation) */
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

/* Helper 7: Uses compound literals with attributes */
struct point {
    int x;
    int y;
};

static struct point make_point(int x, int y) {
    return (struct point){ .x = x, .y = y };
}

/* Main function with diverse constructs */
int main(void) {
    int result = 0;
    
    /* Test 1: Use choose_expr_test with constant and non-constant args */
    result += choose_expr_test(42);
    result += choose_expr_test(result);
    
    /* Test 2: Use TYPE_SAFE_ADD macro */
    int a = 10;
    double b = 20.5;
    result += TYPE_SAFE_ADD(a, a);  /* Should work */
    /* TYPE_SAFE_ADD(a, b); */      /* Would return 0 due to type mismatch */
    
    /* Test 3: Test static array parameter */
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    result += static_array_param(arr);
    
    /* Test 4: Use _Generic selector */
    const char* type_name = GET_TYPE_NAME(a);
    (void)type_name;  /* Prevent unused warning */
    
    /* Test 5: Use inline function with attributes */
    result += inline_func(result);
    
    /* Test 6: Test switch with ranges */
    result += switch_with_ranges(result % 10 + 1);
    
    /* Test 7: Use compound literal */
    struct point p = make_point(1, 2);
    result += p.x + p.y;
    
    /* Test 8: OpenMP parallel region (generates OMP_* tree nodes) */
    #pragma omp parallel for reduction(+:result)
    for (int i = 0; i < 100; i++) {
        result += i % 7;
    }
    
    /* Test 9: More complex OpenMP with teams (if supported) */
    #ifdef _OPENMP
    #pragma omp target teams distribute parallel for map(tofrom:result)
    for (int i = 0; i < 50; i++) {
        result -= i % 3;
    }
    #endif
    
    /* Test 10: Statement expression in function argument */
    printf("Result: %d\n", 
           ({ int temp = result * 2; temp + 1; }));
    
    /* Test 11: __builtin_constant_p in control flow */
    if (__builtin_constant_p(0)) {
        result += 1000;
    }
    
    /* Optional: Test error recovery path */
    #ifdef TEST_ERROR_PATH
    void foo(void);
    /* This will cause an error but compiler may generate recovery nodes */
    /* foo(1); */  /* Uncomment to test error paths */
    #endif
    
    /* Test 12: Use attribute on variable in middle of declaration */
    int x __attribute__((unused)) = 0;
    int y = x + 1;
    
    /* Test 13: Designated initializers with ranges (GCC extension) */
    int array[10] = { [0 ... 4] = 1, [5 ... 9] = 2 };
    result += array[3];
    
    /* Test 14: Binary constants (GCC extension) */
    int binary_val = 0b101010;
    result += binary_val;
    
    /* Test 15: Use __auto_type (GCC extension) */
    __auto_type auto_var = &result;
    result += *auto_var % 100;
    
    printf("Final result: %d\n", result);
    
    /* Test 16: Nested statement expressions */
    int nested = ({
        int inner = ({
            int i = 5;
            i * i;
        });
        inner + 10;
    });
    result += nested;
    
    /* Test 17: Use __builtin_choose_expr with _Generic */
    #define SELECT_OP(type) \
        _Generic((type), \
            int: *, \
            double: /, \
            default: + \
        )
    
    int final_val = result SELECT_OP(int) 2;
    printf("Done: %d\n", final_val);
    
    return 0;
}
