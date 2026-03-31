/* test_tree_coverage.c - Comprehensive test for GCC tree node coverage */

/* Enable OpenMP support */
#ifdef _OPENMP
#include <omp.h>
#endif

#include <stdio.h>
#include <stdlib.h>

/* ==================== IDENTIFIER_NODE and BLOCK coverage ==================== */
/* Generate many unique identifiers using macros */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(var_, n)
#define MAKE_FUNC(n) CONCAT(func_, n)

/* Function with many identifiers and nested blocks */
int test_identifiers_and_blocks(int iterations) {
    int result = 0;
    
    /* Level 1 block with local variables */
    {
        int MAKE_ID(1) = 1;
        int MAKE_ID(2) = 2;
        volatile int MAKE_ID(3) = 3; /* volatile to prevent optimization */
        
        /* Level 2 nested block */
        {
            int MAKE_ID(4) = MAKE_ID(1) + MAKE_ID(2);
            int MAKE_ID(5) = MAKE_ID(3) * 2;
            
            /* Level 3 nested block inside loop */
            for (int MAKE_ID(6) = 0; MAKE_ID(6) < iterations; MAKE_ID(6)++) {
                int MAKE_ID(7) = MAKE_ID(6) * MAKE_ID(5);
                
                /* Level 4 nested block with conditional */
                if (MAKE_ID(7) > 100) {
                    int MAKE_ID(8) = MAKE_ID(7) / 3;
                    result += MAKE_ID(8);
                } else {
                    int MAKE_ID(9) = MAKE_ID(7) * 3;
                    result += MAKE_ID(9);
                }
                
                /* Memory barrier to prevent reordering */
                asm volatile("" : : : "memory");
            }
        }
    }
    
    /* Another block with switch statement */
    {
        int MAKE_ID(10) = result % 10;
        switch (MAKE_ID(10)) {
            case 0: { int MAKE_ID(11) = 1; result += MAKE_ID(11); break; }
            case 1: { int MAKE_ID(12) = 2; result += MAKE_ID(12); break; }
            case 2: { int MAKE_ID(13) = 3; result += MAKE_ID(13); break; }
            default: { int MAKE_ID(14) = 4; result += MAKE_ID(14); break; }
        }
    }
    
    return result;
}

/* Helper functions with unique names */
int MAKE_FUNC(1)(int x) { return x * 2; }
int MAKE_FUNC(2)(int x) { return x + 5; }
int MAKE_FUNC(3)(int x) { return x / 2; }

/* ==================== TREE_VEC coverage ==================== */
/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

int test_vectors_and_ssa(int n) {
    /* Vector declarations and operations */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c;
    
    /* Various vector operations to generate TREE_VEC nodes */
    vec_c = vec_a + vec_b;
    vec_c = vec_c * vec_a;
    vec_c = vec_c - vec_b;
    
    /* Mixed-type vector operations */
    v4sf fvec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec_b = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fvec_c = fvec_a * fvec_b;
    
    /* Force vector operations to not be optimized away */
    volatile v4si* volatile_vec = &vec_c;
    asm volatile("" : : "r"(volatile_vec) : "memory");
    
    /* ==================== SSA_NAME coverage ==================== */
    /* Complex loop with SSA form variables */
    int ssa_var1 = 0;
    int ssa_var2 = 1;
    int ssa_result = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple assignments to create phi nodes */
        if (i % 3 == 0) {
            ssa_var1 = ssa_var2 * 2;
            ssa_var2 = ssa_var1 + i;
        } else if (i % 3 == 1) {
            ssa_var1 = ssa_var2 / 2;
            ssa_var2 = ssa_var1 - i;
        } else {
            ssa_var1 = ssa_var2 + 100;
            ssa_var2 = ssa_var1 * 2;
        }
        
        /* Use both SSA variables in computation */
        ssa_result += ssa_var1 + ssa_var2;
        
        /* Additional branching for more SSA complexity */
        int temp = (i % 2 == 0) ? ssa_var1 : ssa_var2;
        ssa_result += temp * 3;
        
        /* Prevent loop optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Extract scalar from vector for return value */
    int vec_sum = vec_c[0] + vec_c[1] + vec_c[2] + vec_c[3];
    return ssa_result + vec_sum + (int)fvec_c[0];
}

/* ==================== CONSTRUCTOR coverage ==================== */
/* Struct and array initializations with non-constant expressions */
struct ComplexStruct {
    int a;
    int b;
    int c;
    int d[3];
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

int test_aggregate_init(int x, int y) {
    /* Non-constant struct initializer */
    struct ComplexStruct s1 = {
        .a = x + y,
        .b = x - y,
        .c = x * y,
        .d = { x % 10, y % 10, (x + y) % 10 }
    };
    
    /* Array with non-constant initializer */
    int arr[5] = {
        x,
        y,
        x + y,
        x - y,
        test_identifiers_and_blocks(2)  /* Function call in initializer */
    };
    
    /* Nested struct with constructor */
    struct NestedStruct ns = {
        .inner = {
            .a = arr[0],
            .b = arr[1],
            .c = arr[2],
            .d = { arr[3], arr[4], 0 }
        },
        .extra = x * y * 2
    };
    
    /* Union with constructor */
    union MixedUnion {
        int as_int;
        float as_float;
        void* as_ptr;
    } mu = { .as_int = x ^ y };
    
    /* Prevent optimization */
    volatile struct ComplexStruct* vs = &s1;
    asm volatile("" : : "r"(vs) : "memory");
    
    return s1.a + s1.b + s1.c + arr[2] + ns.extra + mu.as_int;
}

/* ==================== OMP_CLAUSE coverage ==================== */
#ifdef _OPENMP
int test_omp_clauses(int size) {
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(size) shared(arr) reduction(+:sum) \
            schedule(dynamic, 4) num_threads(4) if(size > 1000)
    for (int i = 0; i < 100; i++) {
        int local_var = arr[i] * 2;  /* private variable */
        sum += local_var;
        
        /* Nested block inside parallel region */
        {
            int block_var = local_var % 7;
            sum += block_var;
        }
    }
    
    /* OpenMP sections with different clauses */
    int section_result = 0;
    #pragma omp parallel sections private(arr) firstprivate(size) \
            reduction(*:section_result) 
    {
        #pragma omp section
        {
            section_result = size * 2;
        }
        
        #pragma omp section
        {
            section_result += size * 3;
        }
    }
    
    /* OpenMP task with clauses */
    int task_result = 0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task default(none) shared(task_result, size) \
                    depend(out: task_result)
            {
                task_result = size * size;
            }
        }
    }
    
    return sum + section_result + task_result;
}
#else
int test_omp_clauses(int size) {
    /* Fallback without OpenMP */
    return size * 2;
}
#endif

/* ==================== C++ TREE_BINFO coverage (if compiled as C++) ==================== */
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

class SecondDerived : public DerivedClass {
public:
    virtual int method() override { return derived_data + base_data; }
};

int test_binfo_nodes() {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    SecondDerived second;
    
    /* Use virtual calls to ensure vtable and binfo usage */
    int result = base_ptr->method();
    result += second.method();
    
    /* Casts that require binfo lookup */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        result += derived_ptr->method();
    }
    
    /* Multiple inheritance-like access */
    BaseClass& base_ref = second;
    result += base_ref.method();
    
    return result;
}
#else
/* C version - will not generate BINFO nodes but keeps code compilable */
int test_binfo_nodes() {
    return 0;
}
#endif

/* ==================== Main function ==================== */
int main(int argc, char** argv) {
    int result = 0;
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Test all tree node types */
    result += test_identifiers_and_blocks(iterations);
    result += test_vectors_and_ssa(iterations);
    result += test_aggregate_init(iterations, iterations / 2);
    result += test_omp_clauses(iterations);
    result += test_binfo_nodes();
    
    /* Use all helper functions with unique identifiers */
    result += MAKE_FUNC(1)(result);
    result += MAKE_FUNC(2)(result);
    result += MAKE_FUNC(3)(result);
    
    /* Final computation to prevent dead code elimination */
    volatile int final_result = result;
    asm volatile("" : : "r"(&final_result) : "memory");
    
    printf("Result: %d\n", final_result);
    return final_result % 256;
}
