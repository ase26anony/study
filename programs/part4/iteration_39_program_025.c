/* test_tree_coverage.c - Comprehensive test to trigger specific tree node classifications */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimizations from removing our test constructs */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* ========== IDENTIFIER_NODE generation ========== */
/* Generate many distinct identifiers using macros */
#define GEN_ID(n) identifier_##n
#define DECLARE_ID(n) int GEN_ID(n) = n;

NOINLINE USED void test_identifiers(void) {
    /* Generate multiple unique identifiers */
    DECLARE_ID(1)
    DECLARE_ID(2)
    DECLARE_ID(3)
    DECLARE_ID(4)
    DECLARE_ID(5)
    DECLARE_ID(6)
    DECLARE_ID(7)
    DECLARE_ID(8)
    DECLARE_ID(9)
    DECLARE_ID(10)
    
    volatile int sum = 0;
    sum += GEN_ID(1) + GEN_ID(2) + GEN_ID(3) + GEN_ID(4) + GEN_ID(5) +
           GEN_ID(6) + GEN_ID(7) + GEN_ID(8) + GEN_ID(9) + GEN_ID(10);
    
    /* Use asm to prevent optimization */
    asm volatile("" : : "r"(sum) : "memory");
}

/* ========== TREE_VEC generation ========== */
/* Use GCC vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE USED v4si test_vectors(v4si a, v4si b) {
    /* Vector operations that generate TREE_VEC nodes */
    v4si result = a + b;
    result = result * a;
    result = result - b;
    
    /* Complex vector expression */
    v4si temp = {1, 2, 3, 4};
    result = result + temp;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(result) : "memory");
    return result;
}

/* ========== SSA_NAME generation ========== */
NOINLINE USED int test_ssa(int n) {
    int x = 0, y = 0, z = 0;
    volatile int trigger = 1;
    
    /* Complex loop with multiple assignments to create SSA form */
    for (int i = 0; i < n; i++) {
        if (trigger & 1) {
            x = i * 2;
            y = x + i;
        } else {
            x = i / 2;
            y = x - i;
        }
        
        if (i % 3 == 0) {
            z = x + y;
        } else if (i % 3 == 1) {
            z = x - y;
        } else {
            z = x * y;
        }
        
        /* Use all variables to prevent elimination */
        trigger += z;
    }
    
    return x + y + z + trigger;
}

/* ========== BLOCK generation ========== */
NOINLINE USED int test_blocks(int n) {
    int outer = 0;
    
    /* Level 1 block */
    {
        int level1 = n * 2;
        
        /* Level 2 block inside if */
        if (level1 > 0) {
            int level2 = level1 + 10;
            
            /* Level 3 block inside loop */
            for (int i = 0; i < level2; i++) {
                int level3 = i * 3;
                
                /* Level 4 block inside switch */
                switch (i % 4) {
                    case 0: {
                        int level4 = level3 + 1;
                        outer += level4;
                        break;
                    }
                    case 1: {
                        int level4 = level3 + 2;
                        outer += level4;
                        break;
                    }
                    default: {
                        int level4 = level3 + 3;
                        outer += level4;
                        break;
                    }
                }
            }
        }
        
        /* Another block after the if */
        {
            int post_block = outer * 2;
            outer = post_block;
        }
    }
    
    /* Final block with volatile to prevent optimization */
    {
        volatile int final = outer;
        asm volatile("" : : "r"(final) : "memory");
        return final;
    }
}

/* ========== CONSTRUCTOR generation ========== */
struct ComplexStruct {
    int a;
    int b;
    int c;
    int d;
};

int global_counter = 0;

NOINLINE int get_value(int seed) {
    return seed * 2 + global_counter++;
}

NOINLINE USED struct ComplexStruct test_constructor(int seed) {
    /* Non-constant initializer with function calls */
    struct ComplexStruct s = {
        .a = get_value(seed),
        .b = get_value(seed + 1),
        .c = get_value(seed + 2),
        .d = get_value(seed + 3)
    };
    
    /* Array with non-constant initializer */
    int arr[4] = {
        get_value(seed),
        get_value(seed + 4),
        get_value(seed + 8),
        get_value(seed + 12)
    };
    
    /* Nested struct initialization */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    };
    
    struct Nested n = {
        .inner = s,
        .extra = get_value(seed + 16)
    };
    
    /* Use results to prevent optimization */
    s.a += arr[0] + n.extra;
    asm volatile("" : : "r"(s), "r"(arr), "r"(n) : "memory");
    
    return s;
}

/* ========== OMP_CLAUSE generation ========== */
#ifdef _OPENMP
#include <omp.h>

NOINLINE USED int test_omp_clauses(int n) {
    int sum = 0;
    int private_var = 0;
    int shared_arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        shared_arr[i] = i;
    }
    
    /* OpenMP with multiple clauses to generate various OMP_CLAUSE nodes */
    #pragma omp parallel for private(private_var) shared(shared_arr) \
        reduction(+:sum) schedule(dynamic, 4) num_threads(2) \
        if(n > 1000) default(none)
    for (int i = 0; i < n && i < 100; i++) {
        private_var = i * 2;
        sum += shared_arr[i] + private_var;
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel sections private(private_var) \
        firstprivate(sum) copyin(global_counter)
    {
        #pragma omp section
        {
            private_var = sum * 2;
            #pragma omp atomic
            global_counter += private_var;
        }
        
        #pragma omp section
        {
            private_var = sum / 2;
            #pragma omp atomic
            global_counter -= private_var;
        }
    }
    
    /* Task construct with dependencies */
    #pragma omp task depend(inout: sum) final(n > 5000) priority(1)
    {
        sum = sum * 3 / 2;
    }
    
    #pragma omp taskwait
    
    return sum;
}
#else
NOINLINE USED int test_omp_clauses(int n) {
    /* Dummy implementation when OpenMP is not available */
    return n * 2;
}
#endif

/* ========== TREE_BINFO generation attempt ========== */
/* Note: TREE_BINFO is primarily for C++ class hierarchies.
   We'll attempt to trigger it with LTO and by creating
   structures that might need type inheritance information. */

/* Complex struct with nested anonymous structs */
struct TypeHierarchy {
    int base_field;
    
    struct {
        int derived_field1;
        int derived_field2;
    } derived_part;
    
    union {
        int as_int;
        float as_float;
    } variant;
};

NOINLINE USED struct TypeHierarchy test_binfo_like(int val) {
    struct TypeHierarchy th = {
        .base_field = val,
        .derived_part = {val * 2, val * 3},
        .variant = {.as_int = val * 4}
    };
    
    /* Cast through void pointer to create type conversion nodes */
    void* ptr = &th;
    struct TypeHierarchy* th2 = (struct TypeHierarchy*)ptr;
    
    /* Access through different type perspectives */
    int* as_int_ptr = (int*)ptr;
    th.base_field += as_int_ptr[0];
    
    asm volatile("" : : "r"(th), "r"(th2) : "memory");
    return th;
}

/* ========== Main orchestrator ========== */
int main(int argc, char** argv) {
    int result = 0;
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Test all tree node types */
    test_identifiers();
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_result = test_vectors(vec_a, vec_b);
    result += ((int*)&vec_result)[0];
    
    result += test_ssa(seed);
    result += test_blocks(seed);
    
    struct ComplexStruct cs = test_constructor(seed);
    result += cs.a + cs.b + cs.c + cs.d;
    
    result += test_omp_clauses(seed);
    
    struct TypeHierarchy th = test_binfo_like(seed);
    result += th.base_field + th.derived_part.derived_field1;
    
    /* Final result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Force tree dump if compiled with appropriate flags */
    #ifdef __GNUC__
    asm volatile("# Tree coverage test complete" : : : "memory");
    #endif
    
    return (result > 0) ? 0 : 1;
}
