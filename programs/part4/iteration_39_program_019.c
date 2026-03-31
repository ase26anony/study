/* test_tree_coverage.c - Comprehensive test to trigger specific tree node classifications in GCC */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization from removing our test constructs */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* ==================== IDENTIFIER_NODE generation ==================== */
/* Generate many unique identifiers using macros */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

/* Function with many distinct identifiers */
NOINLINE static void test_identifiers(void) {
    /* Create many unique variable names */
    int MAKE_ID(0) = 1;
    int MAKE_ID(1) = MAKE_ID(0) + 1;
    int MAKE_ID(2) = MAKE_ID(1) * 2;
    int MAKE_ID(3) = MAKE_ID(2) / 2;
    int MAKE_ID(4) = MAKE_ID(3) - 1;
    int MAKE_ID(5) = MAKE_ID(4) << 1;
    int MAKE_ID(6) = MAKE_ID(5) >> 1;
    int MAKE_ID(7) = MAKE_ID(6) | 1;
    int MAKE_ID(8) = MAKE_ID(7) & 0xFF;
    int MAKE_ID(9) = MAKE_ID(8) ^ 0x55;
    
    /* Use volatile to prevent optimization */
    volatile int result = 0;
    result += MAKE_ID(0) + MAKE_ID(9);
    asm volatile("" : : "r"(result) : "memory");
}

/* ==================== TREE_VEC generation ==================== */
/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE static v4si test_vectors(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Various vector operations to generate TREE_VEC nodes */
    v4si result1 = a + b;
    v4si result2 = a * b;
    v4si result3 = result1 - result2;
    v4si result4 = result3 & a;
    v4si result5 = result4 | b;
    
    /* Mix with scalar operations */
    int scalar = 42;
    v4si result6 = result5 + scalar;
    
    /* Use volatile to ensure operations aren't optimized away */
    volatile v4si* volatile_ptr = &result6;
    asm volatile("" : : "r"(volatile_ptr) : "memory");
    
    return result6;
}

/* ==================== SSA_NAME generation ==================== */
NOINLINE static int test_ssa_names(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Complex control flow to generate SSA form */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            x = y + z;
        } else if (i % 3 == 0) {
            x = y * z;
        } else {
            x = y - z;
        }
        
        if (i % 5 == 0) {
            y = x + i;
        } else {
            y = x - i;
        }
        
        z = (z + x + y) % 100;
    }
    
    /* Phi nodes will be created for x, y, z in SSA form */
    volatile int vol_result = x + y + z;
    asm volatile("" : : "r"(vol_result) : "memory");
    
    return x + y + z;
}

/* ==================== BLOCK generation ==================== */
NOINLINE static int test_blocks(int iterations) {
    int outer = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Level 1 block */
        {
            int level1 = i * 2;
            
            /* Level 2 block */
            {
                int level2 = level1 + 1;
                
                /* Level 3 block with its own variables */
                {
                    int level3 = level2 * 3;
                    outer += level3;
                    
                    /* Even deeper nested block */
                    {
                        volatile int deepest = level3 % 7;
                        outer += deepest;
                    }
                }
            }
            
            /* Another block at level 1 */
            {
                int another_var = outer % 11;
                outer = another_var * 2;
            }
        }
        
        /* Block in loop body */
        if (i % 2 == 0) {
            int block_in_if = outer + i;
            outer = block_in_if;
        } else {
            int block_in_else = outer - i;
            outer = block_in_else;
        }
    }
    
    return outer;
}

/* ==================== CONSTRUCTOR generation ==================== */
struct ComplexStruct {
    int a;
    int b;
    int c;
    int d;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

NOINLINE static struct ComplexStruct test_constructors(int base) {
    /* Non-constant initializer using function calls/expressions */
    int val1 = base + 1;
    int val2 = base * 2;
    int val3 = base / 3;
    int val4 = base % 7;
    
    /* Constructor with non-constant expressions */
    struct ComplexStruct s1 = {
        .a = val1,
        .b = val2 + val1,
        .c = val3 * val2,
        .d = val4 - val3
    };
    
    /* Array with non-constant initializer */
    int arr[4] = {
        val1 + val2,
        val2 * val3,
        val3 - val4,
        val4 + val1
    };
    
    /* Nested struct constructor */
    struct NestedStruct ns = {
        .inner = {
            .a = arr[0],
            .b = arr[1],
            .c = arr[2],
            .d = arr[3]
        },
        .extra = val1 + val2 + val3 + val4
    };
    
    /* Use designated initializers */
    struct ComplexStruct s2 = {
        .b = ns.extra,
        .a = ns.inner.a,
        .d = ns.inner.d,
        .c = ns.inner.c
    };
    
    /* Mix with volatile to prevent optimization */
    volatile struct ComplexStruct* vs = &s2;
    asm volatile("" : : "r"(vs) : "memory");
    
    return s2;
}

/* ==================== OMP_CLAUSE generation ==================== */
#ifdef _OPENMP
#include <omp.h>

NOINLINE static int test_omp_clauses(int size) {
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
        
        #pragma omp for schedule(dynamic, 4) nowait
        for (int i = 0; i < size; i++) {
            sum += array[i] * (tid + 1);
        }
        
        /* Nested OpenMP construct */
        #pragma omp single
        {
            int single_tid = omp_get_thread_num();
            volatile int single_marker = single_tid;
            asm volatile("" : : "r"(single_marker) : "memory");
        }
    }
    
    /* Another OpenMP construct with different clauses */
    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val) collapse(2) if(size > 100)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int val = i * 100 + j;
            if (val > max_val) {
                max_val = val;
            }
        }
    }
    
    /* OpenMP sections with private/firstprivate */
    #pragma omp parallel sections private(sum)
    {
        #pragma omp section
        {
            int section_sum = 0;
            for (int i = 0; i < 10; i++) section_sum += i;
            volatile int v = section_sum;
            asm volatile("" : : "r"(v) : "memory");
        }
        
        #pragma omp section
        {
            int section_prod = 1;
            for (int i = 1; i <= 5; i++) section_prod *= i;
            volatile int v = section_prod;
            asm volatile("" : : "r"(v) : "memory");
        }
    }
    
    free(array);
    return sum + max_val;
}
#endif

/* ==================== TREE_BINFO generation attempt ==================== */
/* Try to trigger BINFO nodes through various means */
NOINLINE static void attempt_binfo_generation(void) {
    /* Use complex type hierarchies through pointer casts */
    struct Base { int base_data; };
    struct Derived { 
        struct Base base;
        int derived_data;
    };
    
    struct Derived d = { .base = { .base_data = 42 }, .derived_data = 100 };
    struct Base* bp = (struct Base*)&d;
    
    /* Access through base pointer */
    volatile int result = bp->base_data;
    asm volatile("" : : "r"(result) : "memory");
    
    /* Try with arrays of pointers */
    void* ptr_array[4];
    ptr_array[0] = &d;
    ptr_array[1] = bp;
    
    /* Complex memory access pattern */
    for (int i = 0; i < 2; i++) {
        if (i == 0) {
            struct Derived* dp = (struct Derived*)ptr_array[i];
            volatile int v = dp->derived_data;
            asm volatile("" : : "r"(v) : "memory");
        } else {
            struct Base* bpp = (struct Base*)ptr_array[i];
            volatile int v = bpp->base_data;
            asm volatile("" : : "r"(v) : "memory");
        }
    }
}

/* ==================== Main orchestrator ==================== */
int main(int argc, char** argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    printf("Starting tree node coverage test...\n");
    
    /* Test all tree node types */
    test_identifiers();
    
    v4si vec_result = test_vectors();
    volatile int* vptr = (int*)&vec_result;
    int vec_sum = vptr[0] + vptr[1] + vptr[2] + vptr[3];
    
    int ssa_result = test_ssa_names(iterations);
    
    int block_result = test_blocks(iterations / 10);
    
    struct ComplexStruct constr_result = test_constructors(iterations);
    int constr_sum = constr_result.a + constr_result.b + 
                     constr_result.c + constr_result.d;
    
    attempt_binfo_generation();
    
    int omp_result = 0;
#ifdef _OPENMP
    omp_result = test_omp_clauses(iterations);
#endif
    
    /* Combine all results to prevent dead code elimination */
    int final_result = vec_sum + ssa_result + block_result + 
                       constr_sum + omp_result;
    
    printf("Test completed. Checksum: %d\n", final_result);
    
    /* Use result to affect return value */
    return (final_result > 0) ? 0 : 1;
}

/* Additional functions to increase identifier count */
NOINLINE static void extra_identifiers_1(void) {
    int alpha, beta, gamma, delta, epsilon, zeta, eta, theta;
    volatile int mix = alpha + beta - gamma * delta / epsilon % zeta & eta | theta;
    asm volatile("" : : "r"(mix) : "memory");
}

NOINLINE static void extra_identifiers_2(void) {
    int iota, kappa, lambda, mu, nu, xi, omicron, pi;
    volatile int mix = iota ^ kappa + lambda - mu * nu / xi % omicron & pi;
    asm volatile("" : : "r"(mix) : "memory");
}
