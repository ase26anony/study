/* Compile with: gcc -O0 -fdump-tree-original -fdump-tree-gimple -fopenmp -std=gnu11 -c test_tree_codes.c */

#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with statement expressions */
static int builtin_choose_expr_test(int x) {
    /* This creates complex internal tree nodes */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        ({ int y = x * 2; y + 1; }),
        ({ int z = x / 2; z - 1; })
    );
}

/* Function with GNU statement expression in return */
static inline int statement_expr_test(void) {
    return ({
        int a = 5;
        int b = 10;
        __builtin_types_compatible_p(int, typeof(a)) ? a : b;
    });
}

/* Function with static array parameter (creates specialized tree nodes) */
void static_array_param(int arr[static 10]) {
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 2;
    }
}

/* Function with attribute in unusual place */
int __attribute__((unused)) unused_var_test = ({
    int x = 42;
    x;
});

/* Use _Generic selection (creates distinct tree node) */
#define type_selector(x) _Generic((x), \
    int: "int", \
    double: "double", \
    default: "unknown" \
)

/* C++ style nullptr simulation for C */
#define nullptr ((void*)0)

/* Function with void parameter */
void void_func(void) {
    /* Intentionally empty */
}

/* OpenMP helper with complex reduction */
#pragma omp declare reduction(myadd:int:omp_out += omp_in) \
    initializer(omp_priv = 0)

int main(void) {
    int result = 0;
    
    /* Test builtin choose expr */
    result += builtin_choose_expr_test(10);
    
    /* Test statement expression */
    result += statement_expr_test();
    
    /* Test array with static parameter */
    int arr[10];
    static_array_param(arr);
    result += arr[3];
    
    /* Test _Generic selector */
    const char* type_str = type_selector(result);
    printf("Type string: %s\n", type_str);
    
    /* Use switch with case range (creates specific internal representation) */
    int val = 3;
    switch (val) {
        case 1 ... 5:
            result += 100;
            break;
        case 6 ... 10:
            result += 200;
            break;
        default:
            result += 300;
    }
    
    /* OpenMP parallel region - generates OMP_* tree nodes */
    #pragma omp parallel for reduction(myadd:result)
    for (int i = 0; i < 100; i++) {
        result += i % 7;
    }
    
    /* More complex OpenMP construct */
    #pragma omp target teams distribute parallel for simd
    for (int i = 0; i < 50; i++) {
        result -= i % 3;
    }
    
    /* Try to trigger error recovery path (commented to avoid compilation error) */
    /*
    #ifdef TEST_ERROR_PATH
    void_func(1);  // Wrong number of arguments for void function
    #endif
    */
    
    /* Use __builtin_constant_p in type-dependent expression */
    int dynamic_val = rand() % 10;
    int choice = __builtin_choose_expr(
        __builtin_constant_p(dynamic_val),
        1,
        __builtin_types_compatible_p(typeof(dynamic_val), int) ? 2 : 3
    );
    result += choice;
    
    /* Compound literal with attribute */
    struct with_attr {
        int x;
    } __attribute__((packed)) obj = { .x = result };
    
    /* OpenACC-like pragma (might create distinct nodes) */
    #pragma acc parallel loop
    for (int i = 0; i < 20; i++) {
        obj.x += i;
    }
    
    printf("Final result: %d\n", obj.x);
    printf("Done\n");
    
    return 0;
}

/* Additional function with trailing return type syntax simulation */
#define auto_return(expr) typeof(expr)
auto_return(5 + 3) trailing_return_test(int x) {
    return x * 2;
}

/* User-defined literal simulation for C */
#define UDL(x) x##_custom
int value = 123_custom;  // Will expand to 123_custom (syntax error but interesting for parser)
