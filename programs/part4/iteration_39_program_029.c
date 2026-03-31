/* test_tree_coverage.c - Comprehensive test to trigger specific tree node classifications */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimizations from removing our test code */
static volatile int volatile_counter = 0;
#define KEEP_ALIVE(x) do { volatile_counter += (int)(x); } while(0)

/* ========== IDENTIFIER_NODE ========== */
/* Generate many distinct identifiers using macros */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(var_, n)
#define MAKE_FUNC(n) CONCAT(func_, n)

/* Generate multiple identifiers */
#define GEN_IDENTS(n) \
    int MAKE_ID(n) = n; \
    KEEP_ALIVE(MAKE_ID(n));

/* Function with many identifiers */
static int test_identifiers(void) {
    /* Generate 20 distinct identifiers */
    GEN_IDENTS(0) GEN_IDENTS(1) GEN_IDENTS(2) GEN_IDENTS(3) GEN_IDENTS(4)
    GEN_IDENTS(5) GEN_IDENTS(6) GEN_IDENTS(7) GEN_IDENTS(8) GEN_IDENTS(9)
    GEN_IDENTS(10) GEN_IDENTS(11) GEN_IDENTS(12) GEN_IDENTS(13) GEN_IDENTS(14)
    GEN_IDENTS(15) GEN_IDENTS(16) GEN_IDENTS(17) GEN_IDENTS(18) GEN_IDENTS(19)
    
    /* Function identifiers */
    int (*func_ptr)(void) = test_identifiers;
    KEEP_ALIVE((long)func_ptr);
    
    return volatile_counter;
}

/* ========== TREE_VEC ========== */
/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

static v4si test_vectors(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Various vector operations to generate TREE_VEC nodes */
    v4si result1 = a + b;
    v4si result2 = a * b;
    v4si result3 = result1 - result2;
    v4si result4 = result3 & a;
    
    /* Mix with scalar operations */
    for (int i = 0; i < 4; i++) {
        result4[i] += i;
    }
    
    KEEP_ALIVE(result4[0]);
    return result4;
}

/* ========== SSA_NAME ========== */
static int test_ssa(void) {
    int x = 0, y = 0, z = 0;
    
    /* Complex loop with multiple assignments to create SSA names */
    for (int i = 0; i < 100; i++) {
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        if (i % 3 == 0) {
            x = i * 2;
        } else if (i % 3 == 1) {
            x = i * 3;
        } else {
            x = i * 4;
        }
        
        /* Another branch creating phi nodes */
        if (i % 2 == 0) {
            y = x + i;
        } else {
            y = x - i;
        }
        
        z += y;
        
        /* Volatile to prevent dead code elimination */
        KEEP_ALIVE(z);
    }
    
    return z;
}

/* ========== BLOCK ========== */
static int test_blocks(void) {
    int outer = 0;
    
    /* Level 1 block */
    {
        int level1 = 10;
        
        /* Level 2 block */
        {
            int level2 = 20;
            
            /* Level 3 block */
            {
                int level3 = 30;
                outer = level1 + level2 + level3;
                
                /* Level 4 block in loop */
                for (int i = 0; i < 5; i++) {
                    int loop_block = i * 10;
                    outer += loop_block;
                    
                    /* Level 5 block in if */
                    if (i % 2 == 0) {
                        int if_block = i * 20;
                        outer += if_block;
                    } else {
                        int else_block = i * 30;
                        outer += else_block;
                    }
                }
            }
        }
    }
    
    /* Switch with blocks */
    switch (outer % 4) {
        case 0: {
            int case0_block = outer * 2;
            outer = case0_block;
            break;
        }
        case 1: {
            int case1_block = outer * 3;
            outer = case1_block;
            break;
        }
        default: {
            int default_block = outer * 4;
            outer = default_block;
            break;
        }
    }
    
    return outer;
}

/* ========== CONSTRUCTOR ========== */
struct ComplexStruct {
    int a, b, c;
    float f;
    double d;
};

static int array[5];

static struct ComplexStruct test_constructor(void) {
    /* Non-constant initializers */
    int val1 = test_ssa();
    int val2 = test_blocks();
    
    /* Array constructor with non-constant values */
    int arr[5] = { val1, val2, val1 + val2, val1 * 2, val2 * 3 };
    
    /* Struct constructor with designated initializers */
    struct ComplexStruct s = {
        .a = val1,
        .b = val2,
        .c = arr[2],
        .f = (float)val1 / (val2 ? val2 : 1),
        .d = (double)val2 / (val1 ? val1 : 1)
    };
    
    /* Nested struct with array */
    struct Nested {
        struct ComplexStruct inner;
        int more_data[3];
    };
    
    struct Nested n = {
        .inner = s,
        .more_data = { arr[0], arr[1], arr[2] }
    };
    
    KEEP_ALIVE(n.more_data[0]);
    return s;
}

/* ========== OMP_CLAUSE ========== */
#ifdef _OPENMP
#include <omp.h>

static int test_omp_clauses(void) {
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP with multiple clauses to generate OMP_CLAUSE nodes */
    #pragma omp parallel for private(volatile_counter) shared(arr) reduction(+:sum) schedule(dynamic, 4) num_threads(4)
    for (int i = 0; i < 100; i++) {
        int local_sum = 0;
        
        /* Nested OpenMP */
        #pragma omp parallel for reduction(+:local_sum) if(omp_get_num_threads() > 1)
        for (int j = 0; j < 10; j++) {
            local_sum += arr[i] * j;
        }
        
        sum += local_sum;
    }
    
    /* More OpenMP constructs */
    #pragma omp parallel sections private(volatile_counter)
    {
        #pragma omp section
        {
            sum += 1;
        }
        #pragma omp section
        {
            sum += 2;
        }
    }
    
    /* OpenMP task with dependencies */
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: arr[0])
        {
            arr[0] = sum;
        }
        #pragma omp task depend(in: arr[0])
        {
            sum = arr[0] * 2;
        }
    }
    
    return sum;
}
#else
static int test_omp_clauses(void) {
    return 42;  /* Return dummy value if OpenMP not supported */
}
#endif

/* ========== TREE_BINFO (using LTO/flto) ========== */
/* This structure, when compiled with -flto, may generate BINFO nodes */
struct BaseInfo {
    int type_id;
    void (*vfunc)(void);
};

struct DerivedInfo {
    struct BaseInfo base;
    int extra_data;
};

static void test_binfo_like(void) {
    /* Create structures that might generate BINFO-like nodes with LTO */
    struct DerivedInfo d = {
        .base = { .type_id = 1, .vfunc = NULL },
        .extra_data = 42
    };
    
    struct BaseInfo *base_ptr = (struct BaseInfo *)&d;
    KEEP_ALIVE(base_ptr->type_id);
}

/* ========== TREE_VEC additional tests ========== */
static void test_more_vectors(void) {
    /* Different vector sizes and types */
    typedef short v8hi __attribute__((vector_size(16)));
    typedef char v16qi __attribute__((vector_size(16)));
    
    v8hi v1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi v2 = {8, 7, 6, 5, 4, 3, 2, 1};
    v8hi v3 = v1 + v2;
    v8hi v4 = v1 * v2;
    
    v16qi c1 = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    v16qi c2 = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
    v16qi c3 = c1 + c2;
    
    KEEP_ALIVE(v3[0]);
    KEEP_ALIVE(c3[0]);
}

/* ========== Main function ========== */
int main(void) {
    int total = 0;
    
    /* Test all tree node types */
    total += test_identifiers();
    total += test_vectors()[0];
    total += test_ssa();
    total += test_blocks();
    total += test_constructor().a;
    total += test_omp_clauses();
    test_binfo_like();
    test_more_vectors();
    
    /* Use results to prevent optimization */
    printf("Result: %d\n", total + volatile_counter);
    
    return 0;
}
