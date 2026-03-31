/* test_tree_coverage.c - Comprehensive test to trigger specific tree node classifications in GCC */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization from removing our test constructs */
#define NO_OPTIMIZE asm volatile("" : : : "memory")

/* Generate many unique identifiers for IDENTIFIER_NODE */
#define GEN_ID(n) identifier_##n
#define DECLARE_ID(n) int GEN_ID(n) = n;

/* Vector types for TREE_VEC */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a, b, c;
    float f;
    double d;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

/* Function declarations to prevent inlining */
int __attribute__((noinline)) compute_value(int x);
float __attribute__((noinline)) compute_float(float x);
double __attribute__((noinline)) compute_double(double x);

/* Test IDENTIFIER_NODE and BLOCK nodes */
void __attribute__((noinline)) test_identifiers_and_blocks(int iterations) {
    /* Outer block */
    {
        DECLARE_ID(1)
        DECLARE_ID(2)
        DECLARE_ID(3)
        volatile int trigger = GEN_ID(1) + GEN_ID(2) + GEN_ID(3);
        NO_OPTIMIZE;
        
        /* Inner block 1 */
        {
            DECLARE_ID(4)
            DECLARE_ID(5)
            DECLARE_ID(6)
            trigger += GEN_ID(4) * GEN_ID(5) - GEN_ID(6);
            NO_OPTIMIZE;
            
            /* Deeply nested block */
            {
                DECLARE_ID(7)
                DECLARE_ID(8)
                DECLARE_ID(9)
                DECLARE_ID(10)
                trigger -= GEN_ID(7) / (GEN_ID(8) + GEN_ID(9) - GEN_ID(10));
                NO_OPTIMIZE;
            }
        }
        
        /* Inner block 2 with loop */
        for (int i = 0; i < iterations; i++) {
            DECLARE_ID(11)
            DECLARE_ID(12)
            DECLARE_ID(13)
            trigger ^= (GEN_ID(11) | GEN_ID(12)) & GEN_ID(13);
            NO_OPTIMIZE;
            
            /* Block inside loop */
            {
                DECLARE_ID(14)
                DECLARE_ID(15)
                trigger += GEN_ID(14) - GEN_ID(15);
                NO_OPTIMIZE;
            }
        }
        
        printf("Identifier test result: %d\n", trigger);
    }
}

/* Test TREE_VEC and SSA_NAME nodes */
int __attribute__((noinline)) test_vectors_and_ssa(int n) {
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    
    /* Vector operations generating TREE_VEC nodes */
    v4si result = vec1 + vec2 * vec3;
    v4si result2 = vec1 - vec2 / (vec3 + vec1);
    
    /* Mixed vector-scalar operations */
    int sum = 0;
    volatile int temp;
    
    /* Loop with SSA variables */
    for (int i = 0; i < n; i++) {
        int x, y, z;
        
        /* Create SSA_NAME nodes through conditional assignments */
        if (i % 3 == 0) {
            x = i * 2;
            y = i + 5;
            z = x - y;
        } else if (i % 3 == 1) {
            x = i / 2;
            y = i - 3;
            z = y * x;
        } else {
            x = i + 7;
            y = i * 3;
            z = x + y;
        }
        
        /* Complex expression with SSA variables */
        int w = (x > y) ? (z * 2) : (z / 2);
        sum += w;
        
        /* Use vector elements to prevent optimization */
        temp = result[i % 4] + result2[i % 4];
        NO_OPTIMIZE;
    }
    
    /* More vector operations */
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fresult = fvec1 * fvec2 + fvec1 / fvec2;
    
    temp = fresult[0] + fresult[3];
    NO_OPTIMIZE;
    
    return sum + temp;
}

/* Test CONSTRUCTOR nodes */
struct NestedStruct __attribute__((noinline)) test_aggregate_init(int base) {
    /* Non-constant initializers for CONSTRUCTOR nodes */
    int val1 = compute_value(base);
    int val2 = compute_value(base * 2);
    int val3 = compute_value(base + 5);
    float fval = compute_float(base * 1.5f);
    double dval = compute_double(base * 2.5);
    
    /* Struct with non-constant initializer */
    struct ComplexStruct cs = {
        .a = val1,
        .b = val2 + 3,
        .c = val3 - val1,
        .f = fval * 2.0f,
        .d = dval / 3.0
    };
    
    /* Array with non-constant initializers */
    int arr[5] = {
        compute_value(base),
        val1 + val2,
        val3 * 2,
        cs.a + cs.b,
        cs.c
    };
    
    /* Nested struct initialization */
    struct NestedStruct ns = {
        .inner = {
            .a = arr[0],
            .b = arr[1] + cs.a,
            .c = arr[2] - cs.b,
            .f = compute_float(arr[3]),
            .d = compute_double(arr[4])
        },
        .extra = base * 3
    };
    
    /* Another constructor with mixed expressions */
    struct ComplexStruct cs2 = {
        val1 ^ val2,
        val2 | val3,
        val1 & val3,
        fval + 1.0f,
        dval - 0.5
    };
    
    volatile int check = cs2.a + cs2.b;
    NO_OPTIMIZE;
    
    return ns;
}

/* Test OMP_CLAUSE nodes */
#ifdef _OPENMP
#include <omp.h>

int __attribute__((noinline)) test_omp_clauses(int size) {
    int sum = 0;
    int* array = (int*)malloc(size * sizeof(int));
    
    if (!array) return -1;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel shared(array) private(size) reduction(+:sum) num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Nested OpenMP construct */
        #pragma omp for schedule(dynamic, 4) nowait
        for (int i = 0; i < size; i++) {
            sum += array[i] * (tid + 1);
        }
        
        /* OpenMP sections with different clauses */
        #pragma omp sections private(tid) firstprivate(sum)
        {
            #pragma omp section
            {
                int local_sum = sum;
                for (int i = 0; i < 10; i++) {
                    local_sum += i;
                }
                #pragma omp atomic
                sum += local_sum;
            }
            
            #pragma omp section
            {
                int local_sum = sum / 2;
                #pragma omp critical
                {
                    sum = sum > local_sum ? sum : local_sum;
                }
            }
        }
        
        /* OpenMP single construct */
        #pragma omp single copyprivate(tid)
        {
            tid = 0;
        }
    }
    
    /* Another OpenMP construct with collapse clause */
    int matrix[10][10];
    #pragma omp parallel for collapse(2) private(sum)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * j;
        }
    }
    
    free(array);
    return sum;
}
#endif

/* Helper functions */
int __attribute__((noinline)) compute_value(int x) {
    volatile int result = x * 3 + 7;
    NO_OPTIMIZE;
    return result;
}

float __attribute__((noinline)) compute_float(float x) {
    volatile float result = x * 2.5f - 1.0f;
    NO_OPTIMIZE;
    return result;
}

double __attribute__((noinline)) compute_double(double x) {
    volatile double result = x / 1.7 + 3.2;
    NO_OPTIMIZE;
    return result;
}

/* Main function that ties everything together */
int main(int argc, char** argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    printf("Starting tree node coverage test...\n");
    
    /* Test 1: IDENTIFIER_NODE and BLOCK */
    test_identifiers_and_blocks(iterations / 10);
    
    /* Test 2: TREE_VEC and SSA_NAME */
    int vector_result = test_vectors_and_ssa(iterations);
    printf("Vector/SSA test result: %d\n", vector_result);
    
    /* Test 3: CONSTRUCTOR */
    struct NestedStruct ns = test_aggregate_init(iterations);
    printf("Constructor test result: %d\n", ns.inner.a + ns.inner.b + ns.extra);
    
    /* Test 4: OMP_CLAUSE */
    #ifdef _OPENMP
    int omp_result = test_omp_clauses(iterations);
    printf("OpenMP test result: %d\n", omp_result);
    #else
    printf("OpenMP not enabled, skipping OMP_CLAUSE test\n");
    #endif
    
    /* Final checksum to ensure all code runs */
    volatile int final_check = vector_result + ns.inner.a + ns.extra;
    #ifdef _OPENMP
    final_check += omp_result;
    #endif
    
    printf("Final checksum: %d\n", final_check);
    
    return 0;
}
