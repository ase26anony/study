/* test_tree_coverage.c - Comprehensive test to trigger specific tree node classifications */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure tree nodes are fully built */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* ====== IDENTIFIER_NODE generation ====== */
/* Generate many unique identifiers using macros */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(var_, n)
#define MAKE_FUNC(n) CONCAT(func_, n)

/* Generate multiple identifiers */
#define GEN_IDENTS(n) \
    int MAKE_ID(n) = n; \
    void MAKE_FUNC(n)(void) { volatile int x = MAKE_ID(n); (void)x; }

/* Instantiate identifiers */
GEN_IDENTS(1) GEN_IDENTS(2) GEN_IDENTS(3) GEN_IDENTS(4) GEN_IDENTS(5)
GEN_IDENTS(6) GEN_IDENTS(7) GEN_IDENTS(8) GEN_IDENTS(9) GEN_IDENTS(10)

/* ====== TREE_VEC generation ====== */
/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

NOINLINE static void test_vectors(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* Generates TREE_VEC for vector operation */
    v4si d = a * b;
    v4si e = c + d;
    
    /* Use volatile to prevent optimization */
    volatile v4si *vp = &e;
    (void)vp;
    
    /* More complex vector operations */
    v4sf f = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf g = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf h = f * g + f;
    
    volatile v4sf *vfp = &h;
    (void)vfp;
}

/* ====== SSA_NAME generation ====== */
NOINLINE static int test_ssa(int n) {
    int x = 0, y = 0, z = 0;
    
    /* Complex loop with multiple branches to generate SSA form */
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            x = i * 2;
        } else if (i % 3 == 1) {
            x = i + 5;
        } else {
            x = i - 3;
        }
        
        /* Another SSA variable with phi node potential */
        if (i % 2 == 0) {
            y = x + i;
        } else {
            y = x - i;
        }
        
        z += y;
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
    
    /* Additional SSA complexity */
    int result = 0;
    for (int i = 0; i < n; i++) {
        int temp;
        if (z > 100) {
            temp = z / (i + 1);
        } else {
            temp = z * (i + 1);
        }
        result += temp;
    }
    
    return result;
}

/* ====== BLOCK generation ====== */
NOINLINE static void test_blocks(int iterations) {
    /* Outer block with local variables */
    int outer = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Nested block 1 */
        {
            int block1_var = i * 2;
            outer += block1_var;
            
            /* Deeper nested block */
            {
                int inner_var = block1_var + 5;
                outer -= inner_var;
                
                /* Even deeper with its own scope */
                if (inner_var > 10) {
                    int conditional_var = inner_var % 7;
                    outer += conditional_var;
                }
            }
        }
        
        /* Another separate block */
        {
            float block2_float = (float)i / 3.0f;
            double block2_double = (double)outer * 1.5;
            
            /* Block inside a conditional */
            if (block2_float > 2.0f) {
                int cond_block_var = (int)block2_double;
                outer ^= cond_block_var;
            }
        }
        
        /* Switch with blocks in cases */
        switch (i % 4) {
            case 0: {
                int case0_var = i + 100;
                outer += case0_var;
                break;
            }
            case 1: {
                int case1_var = i * 200;
                outer -= case1_var;
                break;
            }
            case 2: {
                int case2_var = i / 3;
                outer |= case2_var;
                break;
            }
            default: {
                int default_var = i % 13;
                outer &= default_var;
                break;
            }
        }
    }
    
    volatile int *block_result = &outer;
    (void)block_result;
}

/* ====== CONSTRUCTOR generation ====== */
struct ComplexStruct {
    int a, b, c;
    float x, y, z;
    char *name;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra[4];
    double values[3];
};

NOINLINE static struct NestedStruct test_constructor(int seed) {
    /* Non-constant function calls for initializer */
    int get_value(void) { return seed * 3 + 7; }
    float get_float(void) { return (float)seed / 2.0f; }
    
    /* Constructor with non-constant expressions */
    struct ComplexStruct cs = {
        .a = get_value(),
        .b = seed + 42,
        .c = abs(seed - 100),
        .x = get_float(),
        .y = get_float() * 2.0f,
        .z = 3.14159f,
        .name = (seed % 2) ? "odd" : "even"
    };
    
    /* Array constructor with mixed expressions */
    int dynamic_array[5] = {
        get_value(),
        seed * 2,
        seed + cs.a,
        cs.b - 17,
        abs(seed) % 100
    };
    
    /* Nested struct constructor */
    struct NestedStruct ns = {
        .inner = cs,
        .extra = { dynamic_array[0], dynamic_array[1], 
                   dynamic_array[2], dynamic_array[3] },
        .values = { get_float(), (double)seed / 3.0, 
                   (double)cs.x * cs.y }
    };
    
    /* Additional constructor with designated initializers */
    struct {
        int first;
        struct { int a; int b; } second;
        int third[2];
    } anon_struct = {
        .first = get_value(),
        .second = { .a = seed, .b = cs.c },
        .third = { [0] = dynamic_array[0], [1] = dynamic_array[1] }
    };
    
    volatile int *check = &anon_struct.first;
    (void)check;
    
    return ns;
}

/* ====== OMP_CLAUSE generation ====== */
#ifdef _OPENMP
#include <omp.h>

NOINLINE static int test_omp_clauses(int size) {
    int sum = 0;
    int *array = malloc(size * sizeof(int));
    
    if (!array) return -1;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel shared(array) reduction(+:sum) \
                         num_threads(4) if(size > 1000)
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for schedule(dynamic, 16) nowait \
                     private(thread_id) firstprivate(size)
        for (int i = 0; i < size; i++) {
            int local_var = array[i];
            /* Complex computation to ensure SSA generation inside OMP */
            for (int j = 0; j < 10; j++) {
                if (j % 2 == 0) {
                    local_var += j;
                } else {
                    local_var -= j;
                }
            }
            sum += local_var;
        }
        
        /* OMP sections with different clauses */
        #pragma omp sections private(thread_id) \
                             lastprivate(sum)
        {
            #pragma omp section
            {
                thread_id = omp_get_thread_num();
                sum += thread_id * 100;
            }
            
            #pragma omp section
            {
                thread_id = omp_get_thread_num();
                sum += thread_id * 200;
            }
        }
        
        /* OMP single with copyprivate */
        int single_var = 0;
        #pragma omp single copyprivate(single_var)
        {
            single_var = omp_get_num_threads() * 1000;
        }
        
        sum += single_var;
    }
    
    /* Another OMP construct - parallel for with collapse */
    int matrix_sum = 0;
    #pragma omp parallel for collapse(2) reduction(+:matrix_sum) \
                         schedule(static) ordered
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            #pragma omp ordered
            matrix_sum += i * j;
        }
    }
    
    sum += matrix_sum;
    
    free(array);
    return sum;
}
#else
NOINLINE static int test_omp_clauses(int size) {
    /* Fallback without OpenMP */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += i;
    }
    return sum;
}
#endif

/* ====== TREE_BINFO generation (using LTO) ====== */
/* Complex struct with nested arrays to potentially generate BINFO in LTO */
struct LargeAggregate {
    int data[256];
    struct {
        float values[64];
        int counters[32];
    } nested;
    union {
        long long as_ll[16];
        double as_double[8];
    } variant;
};

NOINLINE static void process_aggregate(struct LargeAggregate *agg) {
    volatile int total = 0;
    for (int i = 0; i < 256; i++) {
        agg->data[i] = i * 3;
        total += agg->data[i];
    }
    
    for (int i = 0; i < 64; i++) {
        agg->nested.values[i] = (float)i / 2.0f;
    }
    
    for (int i = 0; i < 16; i++) {
        agg->variant.as_ll[i] = (long long)total * i;
    }
    
    (void)total;
}

/* ====== Main function combining all tests ====== */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Call identifier functions to ensure they're used */
    func_1(); func_2(); func_3(); func_4(); func_5();
    
    /* Test vectors */
    test_vectors();
    
    /* Test SSA with complex control flow */
    int ssa_result = test_ssa(100);
    result += ssa_result;
    
    /* Test blocks with nested scopes */
    test_blocks(50);
    
    /* Test constructors */
    struct NestedStruct ns = test_constructor(argc);
    result += ns.inner.a + ns.inner.b + (int)ns.inner.x;
    
    /* Test OpenMP clauses */
    int omp_result = test_omp_clauses(1000);
    result += omp_result;
    
    /* Test large aggregate for potential BINFO generation */
    struct LargeAggregate agg;
    process_aggregate(&agg);
    result += agg.data[0] + agg.data[255];
    
    /* Use all generated identifiers */
    result += var_1 + var_2 + var_3 + var_4 + var_5;
    
    printf("Final result: %d\n", result);
    
    /* Force tree dump by calling abort in a way that's not optimizable */
    if (result == 0xdeadbeef) {
        abort();  /* Never reached, but prevents dead code elimination */
    }
    
    return result % 256;
}
