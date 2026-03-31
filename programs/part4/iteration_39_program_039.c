/* test_tree_classification.c - Comprehensive test for GCC tree node classification */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization */
#define NO_OPTIMIZE asm volatile("" : : : "memory")

/* Generate many unique identifiers for IDENTIFIER_NODE */
#define GEN_ID(n) identifier_##n
#define USE_ID(n) volatile int GEN_ID(n) = n;

/* Vector types for TREE_VEC */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Struct for CONSTRUCTOR */
struct ComplexStruct {
    int a, b, c;
    float f;
    double d;
};

/* Function declarations to prevent inlining */
int __attribute__((noinline)) compute_value(int x);
void __attribute__((noinline)) use_pointer(void* p);
float __attribute__((noinline)) get_float(void);

/* Test IDENTIFIER_NODE and BLOCK nodes */
void test_identifiers_and_blocks(int iterations) {
    /* Many unique identifiers */
    USE_ID(0); USE_ID(1); USE_ID(2); USE_ID(3); USE_ID(4);
    USE_ID(5); USE_ID(6); USE_ID(7); USE_ID(8); USE_ID(9);
    
    volatile int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Nested block for BLOCK node */
        {
            volatile int block_local_1 = i * 2;
            {
                volatile int block_local_2 = block_local_1 + 1;
                {
                    volatile int block_local_3 = block_local_2 * 3;
                    result += block_local_3;
                }
            }
        }
        
        /* Another nested scope */
        if (i % 2 == 0) {
            volatile int even_var = i;
            result += even_var;
        } else {
            volatile int odd_var = i + 100;
            result -= odd_var;
        }
    }
    
    NO_OPTIMIZE;
    printf("Identifiers/blocks result: %d\n", result);
}

/* Test TREE_VEC and SSA_NAME nodes */
void test_vectors_and_ssa(int size) {
    /* Vector operations for TREE_VEC */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    
    /* Complex loop with SSA variables */
    volatile int ssa_var = 0;
    int temp1, temp2, temp3;
    
    for (int i = 0; i < size; i++) {
        /* Create SSA_NAME nodes through conditional assignments */
        if (i % 3 == 0) {
            temp1 = i * 2;
            ssa_var = temp1 + 1;
        } else if (i % 3 == 1) {
            temp2 = i * 3;
            ssa_var = temp2 - 1;
        } else {
            temp3 = i * 4;
            ssa_var = temp3 / 2;
        }
        
        /* More SSA complexity */
        int ssa_temp = ssa_var;
        for (int j = 0; j < 2; j++) {
            ssa_temp += j;
            if (ssa_temp > 10) {
                ssa_temp -= 5;
            }
        }
        
        NO_OPTIMIZE;
        ssa_var = ssa_temp;
    }
    
    /* Use vectors to prevent optimization */
    volatile int vec_sum = vec3[0] + vec3[1] + vec3[2] + vec3[3];
    printf("Vector/SSA result: %d (vec_sum: %d)\n", ssa_var, vec_sum);
}

/* Test CONSTRUCTOR nodes */
void test_aggregate_init(void) {
    /* Non-constant initializers for CONSTRUCTOR */
    int x = compute_value(42);
    float y = get_float();
    
    /* Struct with non-constant initializer */
    struct ComplexStruct s1 = {
        .a = x,
        .b = compute_value(x),
        .c = 99,
        .f = y * 2.0f,
        .d = (double)x / 3.0
    };
    
    /* Array with non-constant initializers */
    int arr[4] = {
        compute_value(1),
        compute_value(2),
        compute_value(3),
        compute_value(4)
    };
    
    /* Nested struct initialization */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    };
    
    struct Nested n1 = {
        .inner = {
            .a = x + 1,
            .b = x - 1,
            .c = 0,
            .f = y,
            .d = 3.14159
        },
        .extra = arr[0] + arr[1]
    };
    
    NO_OPTIMIZE;
    printf("Aggregate init results: %d %f %d\n", 
           s1.a + s1.b, s1.f, n1.extra);
}

/* Test OMP_CLAUSE nodes */
#ifdef _OPENMP
#include <omp.h>

void test_omp_clauses(int n) {
    volatile int shared_sum = 0;
    int private_var = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP with multiple clauses for OMP_CLAUSE nodes */
    #pragma omp parallel for private(private_var) shared(arr, shared_sum) \
            reduction(+:shared_sum) schedule(dynamic, 4) \
            num_threads(4) if(n > 1000)
    for (int i = 0; i < n; i++) {
        private_var = omp_get_thread_num();
        int index = i % 100;
        shared_sum += arr[index] + private_var;
        
        /* Nested block inside parallel region */
        {
            volatile int local = private_var * 2;
            shared_sum += local % 10;
        }
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel sections private(private_var) \
            shared(shared_sum) nowait
    {
        #pragma omp section
        {
            private_var = 1;
            #pragma omp atomic
            shared_sum += private_var;
        }
        
        #pragma omp section
        {
            private_var = 2;
            #pragma omp atomic
            shared_sum += private_var;
        }
    }
    
    printf("OpenMP result: %d\n", shared_sum);
}
#endif

/* Helper functions */
int __attribute__((noinline)) compute_value(int x) {
    volatile int y = x * 2;
    NO_OPTIMIZE;
    return y + 1;
}

float __attribute__((noinline)) get_float(void) {
    static volatile float f = 3.14f;
    NO_OPTIMIZE;
    return f;
}

void __attribute__((noinline)) use_pointer(void* p) {
    NO_OPTIMIZE;
    *(volatile int*)p = 42;
}

/* Main function orchestrating all tests */
int main(int argc, char** argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    printf("Testing tree node classification coverage...\n");
    
    /* Test all constructs */
    test_identifiers_and_blocks(iterations);
    test_vectors_and_ssa(iterations);
    test_aggregate_init();
    
    #ifdef _OPENMP
    test_omp_clauses(iterations * 10);
    #else
    printf("OpenMP not enabled, skipping OMP_CLAUSE test\n");
    #endif
    
    /* Create many more identifiers in main */
    volatile int final_result = 0;
    for (int i = 0; i < 10; i++) {
        char var_name[20];
        sprintf(var_name, "dynamic_var_%d", i);
        NO_OPTIMIZE;
    }
    
    printf("All tests completed.\n");
    return 0;
}
