/* tree_coverage_test.c - Test program to exercise GCC's internal tree node classification */

#include <stdio.h>
#include <stdlib.h>

/* GNU extensions creating complex tree nodes */
#define CHOOSE_EXPR_TEST(x) __builtin_choose_expr(__builtin_constant_p(x), (x)*2, (x)+1)

/* Statement expression in constant context */
#define STMT_EXPR_CONST ({ static int counter = 0; ++counter; })

/* Type compatibility check */
#define TYPE_COMPATIBLE(a, b) __builtin_types_compatible_p(__typeof__(a), __typeof__(b))

/* Front-end specific constructs */
static inline auto_trailing_return(int x) -> int {  /* C++ style trailing return */
    return x * 2;
}

/* Function with GNU attributes in unusual places */
int __attribute__((always_inline)) 
inline_func(int i __attribute__((unused))) {
    return i + 1;
}

/* _Generic selector (C11) */
#define type_name(x) _Generic((x), \
    int: "int", \
    double: "double", \
    default: "unknown" \
)

/* Array parameter with static qualifier */
int static_array_param(int arr[static 10]) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Switch with case ranges */
int switch_with_ranges(int x) {
    switch (x) {
        case 1 ... 5:
            return 1;
        case 6 ... 10:
            return 2;
        default:
            return 0;
    }
}

/* Function with void parameter */
void void_func(void) {
    printf("Void function\n");
}

/* Complex expression using multiple GNU extensions */
int complex_expression(int x) {
    /* __builtin_choose_expr with __builtin_constant_p */
    int a = CHOOSE_EXPR_TEST(5);
    
    /* Statement expression */
    int b = ({ 
        int y = x; 
        while (y > 0) y--; 
        y; 
    });
    
    /* __builtin_types_compatible_p */
    int c = TYPE_COMPATIBLE(a, b) ? 1 : 0;
    
    return a + b + c;
}

/* OpenMP test function */
void openmp_test(void) {
    int i;
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel region - generates OMP_PARALLEL, OMP_FOR nodes */
    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    printf("OpenMP sum: %d\n", sum);
    
    /* OpenMP target directive - may generate distinct tree nodes */
    #pragma omp target teams distribute parallel for map(tofrom:sum)
    for (i = 0; i < 100; i++) {
        sum += 1;
    }
    
    printf("After target: %d\n", sum);
}

/* OpenACC test (if supported) */
#ifdef _OPENACC
void openacc_test(void) {
    int i;
    int sum = 0;
    
    #pragma acc parallel loop reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += i;
    }
    
    printf("OpenACC sum: %d\n", sum);
}
#endif

/* User-defined literal style (C++ like, but in C with GNU extensions) */
#define DECLARE_LITERAL_SUFFIX(suffix, type) \
    static inline type operator "" _##suffix(const char* str) { \
        return (type)atof(str); \
    }

/* Try to declare a literal suffix (more effective in C++ mode) */
DECLARE_LITERAL_SUFFIX(myfloat, float)

/* Main function combining all constructs */
int main(int argc, char *argv[]) {
    int result = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* Test GNU extensions */
    result += complex_expression(10);
    printf("Complex expression result: %d\n", result);
    
    /* Test _Generic */
    int x = 5;
    double y = 3.14;
    printf("Type of x: %s\n", type_name(x));
    printf("Type of y: %s\n", type_name(y));
    
    /* Test switch with ranges */
    result += switch_with_ranges(3);
    printf("After switch: %d\n", result);
    
    /* Test static array parameter */
    int arr[10] = {1,2,3,4,5,6,7,8,9,10};
    result += static_array_param(arr);
    printf("After array param: %d\n", result);
    
    /* Test statement expression */
    int stmt_result = STMT_EXPR_CONST;
    printf("Statement expr: %d\n", stmt_result);
    
    /* Test OpenMP */
    openmp_test();
    
    /* Test OpenACC if available */
    #ifdef _OPENACC
    openacc_test();
    #endif
    
    /* Potentially problematic code guarded by macro */
    #ifdef TEST_ERROR_PATH
    /* This should generate an error but the parser might create special nodes */
    void_func(1);  /* Calling void function with argument */
    
    /* Label as expression (GNU extension) */
    int label_value = (&&label1 - &&label2);
    label1:
    label2:
    printf("Label diff: %d\n", label_value);
    #endif
    
    /* Nested functions (GNU extension) */
    auto nested_func(int a) -> int {
        return a * a;
    };
    
    result += nested_func(5);
    
    /* Try to use the literal suffix */
    float f = "3.14"_myfloat;
    printf("Literal float: %f\n", f);
    
    /* Array with designated initializers and ranges */
    int design_arr[10] = {[0 ... 4] = 1, [5 ... 9] = 2};
    
    /* Alignment specifier */
    _Alignas(16) int aligned_var = 42;
    
    /* __auto_type (GNU C) */
    __auto_type auto_var = 3.14159;
    
    printf("Final result: %d\n", result);
    printf("Done\n");
    
    return 0;
}
