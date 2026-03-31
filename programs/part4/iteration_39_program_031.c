/* test_tree_coverage.c - Comprehensive test for GCC tree node classification */

/* Prevent excessive optimization */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

#include <stdio.h>
#include <stdlib.h>

/* For OpenMP */
#ifdef _OPENMP
#include <omp.h>
#endif

/* ========== TREE_VEC: Vector extensions ========== */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE static void test_vectors(void) {
    volatile v4si a = {1, 2, 3, 4};
    volatile v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c - d;
    
    /* Use memory barrier to prevent optimization */
    asm volatile("" : : : "memory");
    
    /* Force use of results */
    volatile int* p = (int*)&e;
    (void)p[0];
}

/* ========== SSA_NAME: Complex control flow ========== */
NOINLINE static int test_ssa(int n) {
    int x = 0;
    int y = 1;
    volatile int trigger = 0;
    
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            x = y + i;
        } else if (i % 3 == 1) {
            x = y - i;
        } else {
            x = y * i;
        }
        
        /* Multiple assignments to create phi nodes */
        y = x + (i % 2);
        
        /* Memory clobber to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    return x + y;
}

/* ========== BLOCK: Nested scopes ========== */
NOINLINE static void test_blocks(int iterations) {
    int outer = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Level 1 block */
        {
            int level1 = i * 2;
            
            /* Level 2 block */
            {
                int level2 = level1 + 1;
                
                /* Level 3 block with volatile */
                {
                    volatile int level3 = level2 * 3;
                    outer += level3;
                }
            }
            
            /* Another block in same scope */
            {
                int temp = outer;
                asm volatile("" : "+r"(temp) : : "memory");
                outer = temp;
            }
        }
        
        /* Switch with blocks */
        switch (i % 4) {
            case 0: {
                int case_var = i;
                outer += case_var;
                break;
            }
            case 1: {
                int case_var = i * 2;
                outer -= case_var;
                break;
            }
            default: {
                int case_var = i / 2;
                outer ^= case_var;
                break;
            }
        }
    }
    
    volatile int result = outer;
    (void)result;
}

/* ========== CONSTRUCTOR: Aggregate initialization ========== */
struct ComplexStruct {
    int a;
    float b;
    double c;
    int* d;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

NOINLINE static struct NestedStruct test_constructors(int val) {
    /* Non-constant initializer with function calls */
    int computed = val * 2 + 1;
    float fval = (float)val / 3.0f;
    
    /* Constructor with designated initializers */
    struct ComplexStruct cs = {
        .a = computed,
        .b = fval,
        .c = (double)val * 1.5,
        .d = &computed
    };
    
    /* Nested constructor */
    struct NestedStruct ns = {
        .inner = cs,
        .extra = val % 7
    };
    
    /* Array constructor with mixed initializers */
    int arr[5] = { computed, val, val + 1, val * 2, computed - val };
    
    /* Use array to prevent optimization */
    volatile int* parr = arr;
    (void)parr[0];
    
    return ns;
}

/* ========== IDENTIFIER_NODE: Many unique identifiers ========== */
/* Generate unique identifiers using macros */
#define CONCAT(a, b) a##b
#define UNIQUE_VAR(prefix) CONCAT(prefix, __LINE__)

NOINLINE static void test_identifiers(void) {
    /* Generate many unique identifiers */
    int UNIQUE_VAR(var_) = 1;
    int UNIQUE_VAR(var_) = 2;
    int UNIQUE_VAR(var_) = 3;
    int UNIQUE_VAR(var_) = 4;
    int UNIQUE_VAR(var_) = 5;
    int UNIQUE_VAR(var_) = 6;
    int UNIQUE_VAR(var_) = 7;
    int UNIQUE_VAR(var_) = 8;
    int UNIQUE_VAR(var_) = 9;
    int UNIQUE_VAR(var_) = 10;
    
    /* Use them in complex expressions */
    volatile int sum = 
        UNIQUE_VAR(var_1) + UNIQUE_VAR(var_2) - UNIQUE_VAR(var_3) * 
        UNIQUE_VAR(var_4) / (UNIQUE_VAR(var_5) + 1) ^ 
        UNIQUE_VAR(var_6) | UNIQUE_VAR(var_7) & 
        UNIQUE_VAR(var_8) << UNIQUE_VAR(var_9) >> UNIQUE_VAR(var_10);
    
    (void)sum;
}

/* ========== OMP_CLAUSE: OpenMP constructs ========== */
NOINLINE static int test_omp(int size) {
    int sum = 0;
    int* array = malloc(size * sizeof(int));
    
    if (!array) return -1;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* OpenMP with multiple clauses */
    #pragma omp parallel for default(none) shared(array, size) private(size) \
            reduction(+:sum) schedule(dynamic, 4) if(size > 1000) \
            num_threads(4)
    for (int i = 0; i < size; i++) {
        sum += array[i];
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel sections private(sum) firstprivate(array) \
            copyin(size) nowait
    {
        #pragma omp section
        {
            volatile int local_sum = 0;
            for (int i = 0; i < size/2; i++) {
                local_sum += array[i];
            }
            asm volatile("" : : "r"(local_sum) : "memory");
        }
        
        #pragma omp section
        {
            volatile int local_sum = 0;
            for (int i = size/2; i < size; i++) {
                local_sum += array[i];
            }
            asm volatile("" : : "r"(local_sum) : "memory");
        }
    }
    
    /* OpenMP task with clauses */
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < 10; i++) {
                #pragma omp task depend(in: array) depend(out: sum) \
                        final(i > 5) mergeable priority(i)
                {
                    volatile int task_val = array[i % size];
                    sum += task_val;
                }
            }
        }
    }
    
    free(array);
    return sum;
}

/* ========== TREE_BINFO: C++ inheritance (if compiled as C++) ========== */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int method() { return 42; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int method() override { return base_data * 2; }
    int derived_data;
};

NOINLINE static void test_binfo(void) {
    DerivedClass d;
    BaseClass* b = &d;
    volatile int result = b->method();
    (void)result;
}
#else
/* For C, try to trigger BINFO through LTO or complex types */
NOINLINE static void test_binfo(void) {
    /* Complex type that might generate BINFO in LTO */
    struct TypeWithFunctionPointers {
        int (*func1)(void);
        void (*func2)(int);
        float (*func3)(double);
    };
    
    volatile struct TypeWithFunctionPointers twfp = {
        .func1 = NULL,
        .func2 = NULL,
        .func3 = NULL
    };
    
    (void)twfp;
}
#endif

/* ========== Main orchestrator ========== */
int main(int argc, char** argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    printf("Starting tree node coverage test...\n");
    
    /* Test all constructs */
    test_identifiers();
    test_vectors();
    
    int ssa_result = test_ssa(iterations);
    test_blocks(iterations);
    
    struct NestedStruct ns = test_constructors(iterations);
    volatile int ns_val = ns.extra + ns.inner.a;
    (void)ns_val;
    
    int omp_result = 0;
    #ifdef _OPENMP
    omp_result = test_omp(iterations);
    #else
    printf("OpenMP not enabled, skipping OMP_CLAUSE tests\n");
    #endif
    
    test_binfo();
    
    /* Combine results to prevent optimization */
    volatile int final_result = 
        ssa_result + omp_result + ns_val + (int)(ns.inner.b * 100);
    
    printf("Test completed. Final checksum: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
