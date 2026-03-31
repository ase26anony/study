/* test_tree_coverage.c - Comprehensive test to trigger specific tree node classifications */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization */
#define NOOPT __attribute__((noinline))
#define VOLATILE volatile

/* Generate many unique identifiers for IDENTIFIER_NODE */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

/* Vector types for TREE_VEC */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Struct for CONSTRUCTOR */
struct ComplexStruct {
    int a;
    float b;
    double c;
    char d;
};

/* Function declarations */
NOOPT void test_identifiers_and_blocks(void);
NOOPT void test_vectors_and_ssa(void);
NOOPT struct ComplexStruct test_aggregate_init(int x);
NOOPT void test_omp_clauses(int *arr, int n);
NOOPT int checksum_all(void);

/* Global variables to prevent optimization */
VOLATILE int global_counter = 0;
VOLATILE int result_holder = 0;

/* Many identifiers for IDENTIFIER_NODE coverage */
int MAKE_ID(0), MAKE_ID(1), MAKE_ID(2), MAKE_ID(3), MAKE_ID(4);
int MAKE_ID(5), MAKE_ID(6), MAKE_ID(7), MAKE_ID(8), MAKE_ID(9);
int MAKE_ID(10), MAKE_ID(11), MAKE_ID(12), MAKE_ID(13), MAKE_ID(14);
int MAKE_ID(15), MAKE_ID(16), MAKE_ID(17), MAKE_ID(18), MAKE_ID(19);

/* Function with many identifiers and nested blocks */
NOOPT void test_identifiers_and_blocks(void) {
    /* Outer block */
    {
        int outer_var = 10;
        
        /* First inner block */
        {
            int inner_var_1 = outer_var * 2;
            VOLATILE int block_local_1 = inner_var_1 + 5;
            
            /* Deeply nested block */
            {
                int deeply_nested = block_local_1 * 3;
                asm volatile("" : : : "memory"); /* Prevent optimization */
                
                /* Another level */
                {
                    VOLATILE int deepest = deeply_nested / 2;
                    global_counter += deepest;
                }
            }
        }
        
        /* Second inner block with different scope */
        {
            VOLATILE float float_local = 3.14f;
            VOLATILE char char_local = 'A';
            
            /* Block with switch */
            {
                int switch_var = 0;
                switch (outer_var) {
                    case 10:
                        switch_var = 100;
                        break;
                    default:
                        switch_var = 200;
                }
                global_counter += switch_var;
            }
        }
    }
    
    /* Another independent block */
    {
        VOLATILE long long_var = 999999LL;
        VOLATILE short short_var = 123;
        
        /* Block with do-while */
        {
            int counter = 0;
            do {
                VOLATILE int loop_local = counter * 2;
                global_counter += loop_local;
                counter++;
            } while (counter < 5);
        }
    }
}

/* Function for TREE_VEC and SSA_NAME coverage */
NOOPT void test_vectors_and_ssa(void) {
    /* Vector operations for TREE_VEC */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    
    /* Multiple vector operations */
    v4si result1 = vec1 + vec2;
    v4si result2 = vec1 * vec3;
    v4si result3 = result1 - result2;
    
    /* Force vector usage */
    VOLATILE v4si volatile_vec = result3;
    asm volatile("" : : : "memory");
    
    /* SSA_NAME generation with complex control flow */
    int ssa_var = 0;
    VOLATILE int trigger = 1;
    
    /* Loop with multiple assignments to create SSA */
    for (int i = 0; i < 100; i++) {
        /* Multiple conditional assignments */
        if (i % 3 == 0) {
            ssa_var = i * 2;
        } else if (i % 3 == 1) {
            ssa_var = i + 10;
        } else {
            ssa_var = i - 5;
        }
        
        /* Another SSA variable with phi node potential */
        int ssa_var2;
        if (trigger > 0) {
            ssa_var2 = ssa_var * 3;
        } else {
            ssa_var2 = ssa_var / 2;
        }
        
        /* Use both to prevent elimination */
        global_counter += ssa_var + ssa_var2;
        
        /* Modify trigger to affect control flow */
        if (i % 7 == 0) {
            trigger = -trigger;
        }
    }
    
    /* Nested loops for more SSA complexity */
    for (int j = 0; j < 20; j++) {
        int nested_ssa = j;
        for (int k = 0; k < 10; k++) {
            if (k % 2 == 0) {
                nested_ssa += k;
            } else {
                nested_ssa -= k;
            }
        }
        global_counter += nested_ssa;
    }
}

/* Function for CONSTRUCTOR coverage */
NOOPT struct ComplexStruct test_aggregate_init(int x) {
    /* Array constructor with non-constant initializers */
    int dynamic_array[4] = {
        x,
        x * 2,
        x + 10,
        x - 5
    };
    
    /* Struct constructor with designated initializers */
    struct ComplexStruct cs = {
        .a = x * 3,
        .b = (float)x / 2.0f,
        .c = (double)x * 1.5,
        .d = (char)(x % 256)
    };
    
    /* Nested struct initialization */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    };
    
    struct Nested ns = {
        .inner = { .a = x + 1, .b = 2.5f, .c = 3.14, .d = 'X' },
        .extra = x * 10
    };
    
    /* Use the aggregates to prevent optimization */
    asm volatile("" : : : "memory");
    global_counter += dynamic_array[0] + cs.a + ns.extra;
    
    return cs;
}

/* Function for OMP_CLAUSE coverage */
#ifdef _OPENMP
#include <omp.h>
#endif

NOOPT void test_omp_clauses(int *arr, int n) {
#ifdef _OPENMP
    int i;
    int local_sum = 0;
    int private_var = 0;
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(i) shared(arr, n, global_counter) \
        reduction(+:local_sum) schedule(dynamic) num_threads(4) \
        firstprivate(private_var) if(n > 1000)
    for (i = 0; i < n; i++) {
        /* Each thread gets its own private_var copy */
        private_var = omp_get_thread_num();
        
        /* Complex operation to generate various tree nodes */
        int val = arr[i];
        if (val % 2 == 0) {
            val *= 2;
        } else {
            val /= 2;
        }
        
        /* Nested block inside parallel region */
        {
            VOLATILE int block_in_omp = val + private_var;
            local_sum += block_in_omp;
        }
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel sections private(i) shared(global_counter) \
        nowait
    {
        #pragma omp section
        {
            /* Section 1 */
            for (i = 0; i < 10; i++) {
                global_counter += i;
            }
        }
        
        #pragma omp section
        {
            /* Section 2 */
            VOLATILE int section_var = 100;
            for (i = 0; i < 5; i++) {
                section_var += i * 2;
            }
            global_counter += section_var;
        }
    }
    
    /* OpenMP task with task-specific clauses */
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (i = 0; i < 3; i++) {
                #pragma omp task depend(out: arr[i]) priority(i)
                {
                    arr[i] = i * 100;
                }
            }
        }
    }
    
    global_counter += local_sum;
#else
    /* Fallback without OpenMP */
    for (int i = 0; i < n; i++) {
        global_counter += arr[i];
    }
#endif
}

/* Main function orchestrating all tests */
int main(void) {
    /* Initialize array for OpenMP test */
    int n = 100;
    int *arr = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        arr[i] = i;
    }
    
    /* Execute all test functions */
    test_identifiers_and_blocks();
    
    test_vectors_and_ssa();
    
    struct ComplexStruct cs = test_aggregate_init(42);
    global_counter += cs.a;
    
    test_omp_clauses(arr, n);
    
    /* Use all generated identifiers */
    MAKE_ID(0) = 1; MAKE_ID(1) = 2; MAKE_ID(2) = 3;
    MAKE_ID(3) = 4; MAKE_ID(4) = 5; MAKE_ID(5) = 6;
    MAKE_ID(6) = 7; MAKE_ID(7) = 8; MAKE_ID(8) = 9;
    MAKE_ID(9) = 10; MAKE_ID(10) = 11; MAKE_ID(11) = 12;
    MAKE_ID(12) = 13; MAKE_ID(13) = 14; MAKE_ID(14) = 15;
    MAKE_ID(15) = 16; MAKE_ID(16) = 17; MAKE_ID(17) = 18;
    MAKE_ID(18) = 19; MAKE_ID(19) = 20;
    
    for (int i = 0; i < 20; i++) {
        global_counter += MAKE_ID(i);
    }
    
    /* Final result to prevent dead code elimination */
    printf("Final counter value: %d\n", global_counter);
    
    free(arr);
    return global_counter > 0 ? 0 : 1;
}
