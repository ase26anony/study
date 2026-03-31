/* Compile with: gcc -O0 -fdump-tree-all -fopenmp -std=gnu11 -c test_tree_codes.c */
/* Also try: gcc -O2 -fdump-tree-optimized -fopenmp -std=gnu++17 -c test_tree_codes.cc */

#include <stdio.h>
#include <stdlib.h>

/* GNU extension: statement expression with complex tree structure */
#define COMPLEX_EXPR(x) ({ \
    typeof(x) __tmp = (x); \
    __builtin_choose_expr(__builtin_constant_p(__tmp), \
        __builtin_types_compatible_p(typeof(__tmp), int) ? __tmp * 2 : __tmp, \
        __tmp + 1); \
})

/* Attribute in unusual place - may create annotated nodes */
static int unused_var __attribute__((unused)) = 
    __builtin_choose_expr(1, 42, 0);

/* Function with static array parameter - creates specialized tree node */
void process_array(int arr[static 10]) {
    for (int i = 0; i < 10; i++) {
        arr[i] = COMPLEX_EXPR(i);
    }
}

/* C11 _Generic selector - creates distinct tree node */
#define TYPE_NAME(x) _Generic((x), \
    int: "int", \
    double: "double", \
    default: "unknown" \
)

/* Switch with case ranges - parsed into specific internal representation */
int range_switch(int x) {
    switch (x) {
        case 1 ... 5:
            return x * 2;
        case 6 ... 10:
            return x / 2;
        default:
            return __builtin_constant_p(x) ? x : -1;
    }
}

/* OpenMP helper function */
#pragma omp declare target
int target_function(int x) {
    return x * x;
}
#pragma omp end declare target

/* Function that might trigger error recovery paths */
#ifdef TEST_ERROR_PATH
void void_func(void) {
    /* This declaration might create special tree nodes */
}

void test_error_path(void) {
    /* This would be an error but might create recovery nodes */
    /* void_func(42); */  /* Uncomment to test error paths */
}
#endif

/* Main function with diverse constructs */
int main(void) {
    int result = 0;
    int arr[10] = {0};
    
    /* Use statement expression in constant context */
    int complex_val = COMPLEX_EXPR(10);
    
    /* Process array with static parameter */
    process_array(arr);
    
    /* Use _Generic selector */
    const char* type_str = TYPE_NAME(complex_val);
    printf("Type: %s\n", type_str);
    
    /* Test range switch */
    result = range_switch(complex_val);
    
    /* OpenMP parallel region - generates OMP_* nodes */
    #pragma omp parallel for reduction(+:result)
    for (int i = 0; i < 100; i++) {
        result += i & 0xF;
    }
    
    /* OpenMP target region - may generate distinct tree nodes */
    #pragma omp target teams distribute parallel for map(tofrom:result)
    for (int i = 0; i < 50; i++) {
        result += target_function(i % 10);
    }
    
    /* More GNU extensions */
    int chosen = __builtin_choose_expr(
        __builtin_types_compatible_p(typeof(result), int),
        result,
        0
    );
    
    /* Use __builtin_constant_p in control flow */
    if (__builtin_constant_p(chosen)) {
        printf("Constant: %d\n", chosen);
    } else {
        printf("Dynamic: %d\n", chosen);
    }
    
    /* Try to create unusual tree nodes with nested builtins */
    int nested = __builtin_choose_expr(
        1,
        __builtin_types_compatible_p(typeof(arr[0]), int) ? 
            __builtin_constant_p(arr[0]) ? arr[0] : 0 : 1,
        0
    );
    
    printf("Result: %d\n", result + nested);
    printf("Done\n");
    
    return 0;
}

/* Additional C++ specific code if compiled as C++ */
#ifdef __cplusplus
// C++ specific constructs that may generate unique tree nodes
template<typename T>
auto trailing_return(T x) -> decltype(x * 2) {
    return x * 2;
}

void cpp_specific(void) {
    // nullptr - C++ specific tree node
    int* ptr = nullptr;
    
    // User-defined literal (C++11)
    // auto x = 123_suffix;  // Would need literal operator
    
    // Use trailing return type
    auto val = trailing_return(42);
    
    // Lambda expression - may create complex tree structure
    auto lambda = [](int x) { return x + 1; };
    lambda(val);
}
#endif
