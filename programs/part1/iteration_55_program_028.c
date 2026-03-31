/* tree_coverage_test.c - Test program to exercise GCC's internal tree representation */
/* Compile with: gcc -O0 -fopenmp -std=gnu11 -fdump-tree-all -o test tree_coverage_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Helper using __builtin_choose_expr with statement expressions */
static int choose_expr_test(int x) {
    /* This creates complex expression trees */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        ({ int y = x * 2; y + 1; }),
        ({ int y = x / 2; y - 1; })
    );
}

/* Function with GNU statement expression in return */
static inline int stmt_expr_func(int a, int b) {
    return ({
        int temp = a + b;
        temp * temp - a * b;
    });
}

/* Function using __builtin_types_compatible_p in type-dependent expression */
#define TYPE_SAFE_ADD(a, b) \
    __builtin_choose_expr( \
        __builtin_types_compatible_p(typeof(a), typeof(b)), \
        (a) + (b), \
        (__typeof__(a + b))0 \
    )

/* Function with attribute in unusual place */
static int attr_test(int x __attribute__((unused))) {
    return x * 2;
}

/* Function with static array index in parameter (C99 feature) */
void static_array_param(int arr[static 10]) {
    for (int i = 0; i < 10; i++) {
        arr[i] = i * i;
    }
}

/* Function with switch case ranges (GNU extension) */
static int case_range_test(int val) {
    switch (val) {
        case 1 ... 5:
            return val * 10;
        case 6 ... 10:
            return val * 20;
        default:
            return val;
    }
}

/* Function using _Generic (C11) - creates SELECTOR tree nodes */
#define get_type_name(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char *: "string", \
    default: "unknown" \
)

/* C++-like nullptr simulation for C */
#define nullptr ((void*)0)

/* OpenMP test function */
void omp_test(int n) {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i * i;
    }
    printf("OpenMP sum: %d\n", sum);
    
    /* OpenMP target with teams - may create additional OpenMP tree nodes */
    #pragma omp target teams distribute parallel for map(tofrom:sum)
    for (int i = 0; i < 10; i++) {
        sum += i;
    }
}

/* Function with trailing return type style (using typeof) */
#define auto_add(a, b) ({ \
    __typeof__(a + b) result = (a) + (b); \
    result; \
})

/* User-defined literal simulation using compound literals */
#define SECONDS(x) ((struct timeval){x, 0})

/* Error recovery path test */
#ifdef TEST_ERROR_PATH
void void_func(void) {
    printf("Void function\n");
}
#endif

/* Main function combining all constructs */
int main(void) {
    int result = 0;
    
    /* Test GNU extensions */
    result += choose_expr_test(10);
    result += stmt_expr_func(5, 3);
    
    /* Test type-safe add */
    int a = 5;
    double b = 3.14;
    result += TYPE_SAFE_ADD(a, a);  /* This should work */
    /* TYPE_SAFE_ADD(a, b) would return 0 due to type mismatch */
    
    /* Test attribute */
    result += attr_test(7);
    
    /* Test static array parameter */
    int arr[10];
    static_array_param(arr);
    result += arr[5];
    
    /* Test case ranges */
    result += case_range_test(3);
    result += case_range_test(8);
    
    /* Test _Generic selector */
    const char *type_name = get_type_name(result);
    printf("Result type: %s\n", type_name);
    
    /* Test nullptr-like construct */
    void *ptr = nullptr;
    if (ptr == NULL) {
        result += 100;
    }
    
    /* Test compound literal (user-defined literal simulation) */
    struct timeval tv = SECONDS(5);
    result += tv.tv_sec;
    
    /* Test OpenMP constructs */
    omp_test(100);
    
    /* Test auto-style return */
    result += auto_add(20, 30);
    
    /* Test error recovery path if enabled */
    #ifdef TEST_ERROR_PATH
    /* This should cause an error but the compiler might generate
       special tree nodes during error recovery */
    void_func(42);  /* Calling void function with argument */
    #endif
    
    /* Additional complex expression using all builtins */
    int complex_expr = __builtin_choose_expr(
        __builtin_types_compatible_p(typeof(result), int),
        __builtin_constant_p(result) ? result : ({
            int temp = result;
            while (temp > 10) temp /= 2;
            temp;
        }),
        0
    );
    
    result += complex_expr;
    
    /* Try to create some unusual tree nodes with labels and computed goto */
    void *label_ptr = &&done;
    goto *label_ptr;
    
    /* Unreachable code that might create interesting tree nodes */
    {
        int unreachable __attribute__((unused)) = 0;
        /* Empty switch with only default case */
        switch (unreachable) {
            default:
                break;
        }
    }
    
done:
    printf("Final result: %d\n", result);
    
    /* Additional OpenACC pragma (if supported) */
    #ifdef _OPENACC
    #pragma acc parallel loop
    for (int i = 0; i < 10; i++) {
        arr[i] *= 2;
    }
    #endif
    
    return 0;
}

/* Additional functions to create more tree node variety */

/* Function with array parameter qualifiers */
void multi_dim_array(int m, int n, int arr[restrict static m][n]) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            arr[i][j] = i * j;
        }
    }
}

/* Function using offsetof with statement expr */
#include <stddef.h>
struct mystruct {
    int a;
    double b;
    char c[10];
};

size_t get_offset(void) {
    return ({
        size_t off = offsetof(struct mystruct, b);
        off + sizeof(double);
    });
}

/* Aligned attribute test */
struct aligned_struct {
    int data[4];
} __attribute__((aligned(64)));

/* Packed attribute test */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    char c;
};

/* Cleanup attribute - might create special tree nodes */
void cleanup_func(int *p) {
    free(p);
}

void cleanup_test(void) {
    int *ptr __attribute__((cleanup(cleanup_func))) = malloc(sizeof(int));
    *ptr = 42;
}
