/* test_tree_coverage.c - Comprehensive test to trigger all tree node classifications */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization from removing our test constructs */
#define NO_OPTIMIZE asm volatile("" : : : "memory")

/* Generate many unique identifiers for IDENTIFIER_NODE */
#define GEN_ID(n) identifier_##n
#define USE_ID(n) int GEN_ID(n) = n;

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
struct ComplexStruct __attribute__((noinline)) make_struct(int a, int b);

/* Test IDENTIFIER_NODE and BLOCK nodes */
void __attribute__((noinline)) test_identifiers_and_blocks(int iterations) {
    /* Many unique identifiers */
    USE_ID(0); USE_ID(1); USE_ID(2); USE_ID(3); USE_ID(4);
    USE_ID(5); USE_ID(6); USE_ID(7); USE_ID(8); USE_ID(9);
    
    volatile int trigger = iterations;
    
    /* Nested blocks creating BLOCK nodes */
    {
        int block_local_1 = compute_value(trigger);
        {
            int block_local_2 = block_local_1 * 2;
            {
                int block_local_3 = block_local_2 + compute_value(block_local_2);
                NO_OPTIMIZE;
            }
        }
    }
    
    if (trigger > 0) {
        /* Another block in if statement */
        int if_block_var = trigger * 3;
        for (int i = 0; i < 3; i++) {
            /* Block inside loop */
            int loop_block_var = if_block_var + i;
            NO_OPTIMIZE;
        }
    }
    
    /* Switch with blocks */
    switch (trigger % 3) {
        case 0: {
            int switch_block_0 = compute_value(trigger);
            NO_OPTIMIZE;
            break;
        }
        case 1: {
            int switch_block_1 = compute_value(trigger + 1);
            NO_OPTIMIZE;
            break;
        }
        default: {
            int switch_block_default = compute_value(trigger * 2);
            NO_OPTIMIZE;
            break;
        }
    }
}

/* Test TREE_VEC and SSA_NAME nodes */
float __attribute__((noinline)) test_vectors_and_ssa(int n) {
    /* Vector operations creating TREE_VEC nodes */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = vec_a + vec_b;
    v4si vec_d = vec_a * vec_b;
    
    v4sf fvec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec_b = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fvec_c = fvec_a * fvec_b;
    
    NO_OPTIMIZE;
    
    /* Complex loop with SSA variables */
    float sum = 0.0f;
    float prod = 1.0f;
    
    for (int i = 0; i < n; i++) {
        /* This creates SSA_NAME nodes due to phi nodes */
        float x;
        if (i % 2 == 0) {
            x = compute_float(i * 0.1f);
        } else {
            x = compute_float(i * 0.2f);
        }
        
        /* Another SSA variable with multiple assignments */
        float y;
        if (i % 3 == 0) {
            y = x * 2.0f;
        } else if (i % 3 == 1) {
            y = x / 2.0f;
        } else {
            y = x + 1.0f;
        }
        
        sum += y;
        prod *= (y + 1.0f);
        
        NO_OPTIMIZE;
    }
    
    /* Use vector results to prevent optimization */
    float vec_sum = fvec_c[0] + fvec_c[1] + fvec_c[2] + fvec_c[3];
    return sum + prod + vec_sum;
}

/* Test CONSTRUCTOR nodes */
struct NestedStruct __attribute__((noinline)) test_aggregate_init(int base) {
    /* Non-constant initializers create CONSTRUCTOR nodes */
    int val1 = compute_value(base);
    int val2 = compute_value(base + 1);
    int val3 = compute_value(base + 2);
    float fval = compute_float(base * 0.5f);
    
    /* Constructor with non-constant expressions */
    struct ComplexStruct cs = {
        .a = val1,
        .b = val2,
        .c = val3 * 2,
        .f = fval,
        .d = fval * 2.0
    };
    
    /* Array with non-constant initializer */
    int arr[4] = {
        compute_value(base),
        compute_value(base + 10),
        compute_value(base + 20),
        compute_value(base + 30)
    };
    
    /* Nested struct constructor */
    struct NestedStruct ns = {
        .inner = {
            .a = arr[0],
            .b = arr[1],
            .c = arr[2],
            .f = compute_float(arr[3]),
            .d = arr[3] * 0.5
        },
        .extra = val1 + val2
    };
    
    /* Designated initializer with mixed order */
    struct ComplexStruct cs2 = {
        .c = val3,
        .f = fval * 0.5f,
        .a = val1 / 2,
        .d = fval * 3.0,
        .b = val2 * 2
    };
    
    NO_OPTIMIZE;
    
    /* Use all constructs */
    ns.inner.f += cs.f + cs2.f;
    return ns;
}

/* Test OMP_CLAUSE nodes */
#ifdef _OPENMP
#include <omp.h>

float __attribute__((noinline)) test_omp_clauses(int size) {
    float total = 0.0f;
    volatile int chunk_size = 4;
    
    /* OpenMP with multiple clauses */
    #pragma omp parallel shared(total) private(chunk_size) \
        reduction(+:total) num_threads(2) if(size > 100)
    {
        int thread_id = omp_get_thread_num();
        chunk_size = (thread_id + 1) * 2;
        
        #pragma omp for schedule(dynamic, chunk_size) nowait
        for (int i = 0; i < size; i++) {
            float val = compute_float(i * 0.01f);
            total += val;
        }
        
        /* Nested OpenMP construct */
        #pragma omp single
        {
            #pragma omp task firstprivate(thread_id)
            {
                float task_val = compute_float(thread_id * 10.0f);
                NO_OPTIMIZE;
            }
        }
    }
    
    /* Another OpenMP construct with different clauses */
    float max_val = 0.0f;
    float min_val = 1000000.0f;
    
    #pragma omp parallel sections reduction(max:max_val) reduction(min:min_val) \
        default(none) shared(size)
    {
        #pragma omp section
        {
            for (int i = 0; i < size/2; i++) {
                float val = compute_float(i * 0.02f);
                if (val > max_val) max_val = val;
            }
        }
        
        #pragma omp section
        {
            for (int i = size/2; i < size; i++) {
                float val = compute_float(i * 0.02f);
                if (val < min_val) min_val = val;
            }
        }
    }
    
    return total + max_val - min_val;
}
#endif

/* Helper functions */
int __attribute__((noinline)) compute_value(int x) {
    volatile int y = x;
    return y * 2 + 1;
}

float __attribute__((noinline)) compute_float(float x) {
    volatile float y = x;
    return y * 1.5f - 0.3f;
}

struct ComplexStruct __attribute__((noinline)) make_struct(int a, int b) {
    struct ComplexStruct cs = {
        .a = compute_value(a),
        .b = compute_value(b),
        .c = compute_value(a + b),
        .f = compute_float((a + b) * 0.1f),
        .d = (a + b) * 0.5
    };
    return cs;
}

/* Main function that exercises all test cases */
int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
    }
    
    printf("Testing tree node coverage...\n");
    
    /* Test 1: Identifiers and Blocks */
    test_identifiers_and_blocks(iterations);
    
    /* Test 2: Vectors and SSA */
    float vec_result = test_vectors_and_ssa(iterations);
    printf("Vector/SSA test result: %f\n", vec_result);
    
    /* Test 3: Aggregate initialization */
    struct NestedStruct ns = test_aggregate_init(iterations);
    printf("Struct test: a=%d, b=%d, extra=%d\n", 
           ns.inner.a, ns.inner.b, ns.extra);
    
    /* Test 4: OpenMP clauses */
    #ifdef _OPENMP
    float omp_result = test_omp_clauses(iterations);
    printf("OpenMP test result: %f\n", omp_result);
    #else
    printf("OpenMP not enabled, skipping OMP_CLAUSE test\n");
    #endif
    
    /* Additional complex expression mixing everything */
    {
        /* One more block with everything */
        v4si final_vec = {1, 2, 3, 4};
        for (int i = 0; i < 4; i++) {
            final_vec[i] = compute_value(final_vec[i] + i);
        }
        
        struct ComplexStruct final_cs = make_struct(
            ns.inner.a, 
            ns.inner.b
        );
        
        float final_sum = vec_result + final_cs.f + final_vec[0];
        printf("Final combined result: %f\n", final_sum);
    }
    
    return 0;
}
