/* tree_coverage.c - Complex program to exercise GCC's internal tree representation */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Helper using __builtin_choose_expr with statement expressions */
static inline int choose_expr_demo(int x) {
    /* This creates specialized tree nodes during parsing */
    return __builtin_choose_expr(
        __builtin_constant_p(x),
        ({ int y = x * 2; y + 1; }),
        ({ int y = x / 2; y - 1; })
    );
}

/* Function with GNU statement expression and __builtin_types_compatible_p */
#define TYPE_SAFE_ADD(a, b) \
    ({ \
        typeof(a) _a = (a); \
        typeof(b) _b = (b); \
        _a + _b; \
    })

static int type_safe_add(int a, double b) {
    /* __builtin_types_compatible_p creates comparison nodes */
    if (__builtin_types_compatible_p(typeof(a), int)) {
        return TYPE_SAFE_ADD(a, (int)b);
    }
    return 0;
}

/* Function with array parameter using static qualifier */
void process_array(int arr[static 10]) {
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 2;
    }
}

/* Function with C11 _Generic selector */
#define GET_TYPE_NAME(x) _Generic((x), \
    int: "int", \
    double: "double", \
    char*: "string", \
    default: "unknown" \
)

const char* get_type_name_demo(void) {
    int i = 42;
    double d = 3.14;
    return GET_TYPE_NAME(i + d);
}

/* Function with switch case ranges (GNU extension) */
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

/* Function with attributes in unusual places */
int __attribute__((unused)) unused_var __attribute__((aligned(16))) = 0;

void attribute_demo(void) {
    int i __attribute__((unused)) = 0;
    int j __attribute__((aligned(8))) = 1;
    
    /* Potentially creates annotated tree nodes */
    if (j) {
        i = j;
    }
}

/* OpenMP function with complex clauses */
void omp_complex_demo(int n) {
    int i;
    int sum = 0;
    
    #pragma omp target teams distribute parallel for simd \
            reduction(+:sum) map(tofrom:sum) num_teams(4) thread_limit(64)
    for (i = 0; i < n; i++) {
        sum += i;
    }
    
    printf("OpenMP sum: %d\n", sum);
}

/* Function that might trigger error recovery paths */
#ifdef TEST_ERROR_PATH
void error_path_demo(void) {
    /* Declare function with void parameter */
    void foo(void);
    
    /* Incorrect call - might generate error recovery nodes */
    foo(1);  /* This should cause an error but may create special tree nodes */
}
#endif

/* Main function combining all patterns */
int main(void) {
    int result;
    int arr[10];
    
    printf("Starting tree coverage test...\n");
    
    /* Test various constructs */
    result = choose_expr_demo(42);
    printf("choose_expr_demo: %d\n", result);
    
    result = type_safe_add(10, 20.5);
    printf("type_safe_add: %d\n", result);
    
    process_array(arr);
    printf("process_array done\n");
    
    const char* type_name = get_type_name_demo();
    printf("Type name: %s\n", type_name);
    
    result = switch_with_ranges(3);
    printf("switch_with_ranges: %d\n", result);
    
    attribute_demo();
    
    /* OpenMP section */
    omp_complex_demo(100);
    
    /* Additional OpenMP with nested parallelism */
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            #pragma omp parallel for
            for (int i = 0; i < 10; i++) {
                arr[i] = i;
            }
        }
        
        #pragma omp section
        {
            int x = 0;
            #pragma omp atomic
            x++;
        }
    }
    
    /* Use __builtin_constant_p in complex expression */
    int dynamic_val = rand() % 100;
    int complex_result = __builtin_choose_expr(
        __builtin_constant_p(dynamic_val),
        dynamic_val * 2,
        ({ 
            int temp = dynamic_val;
            while (temp > 0) temp--;
            temp;
        })
    );
    
    printf("complex_result: %d\n", complex_result);
    
    /* Final print to ensure execution */
    printf("Done\n");
    
    return 0;
}
