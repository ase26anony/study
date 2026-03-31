/* test_tree_coverage.c - Comprehensive test to trigger all tree node classifications */
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization */
static volatile int volatile_var = 0;

/* Memory barrier to prevent reordering */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* ========== IDENTIFIER_NODE generation ========== */
/* Generate many unique identifiers using macros */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

void test_identifiers(void) {
    /* Generate multiple unique identifiers */
    int MAKE_ID(1) = 1;
    int MAKE_ID(2) = 2;
    int MAKE_ID(3) = 3;
    int MAKE_ID(4) = 4;
    int MAKE_ID(5) = 5;
    int MAKE_ID(6) = 6;
    int MAKE_ID(7) = 7;
    int MAKE_ID(8) = 8;
    int MAKE_ID(9) = 9;
    int MAKE_ID(10) = 10;
    
    /* Use them in complex expressions */
    volatile_var = MAKE_ID(1) + MAKE_ID(2) * MAKE_ID(3) - MAKE_ID(4) / MAKE_ID(5);
    MEMORY_BARRIER();
}

/* ========== TREE_VEC generation ========== */
/* Use GCC vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

void test_vectors(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Multiple vector operations to generate TREE_VEC nodes */
    v4si result1 = a + b;
    v4si result2 = b * c;
    v4si result3 = result1 - result2;
    
    /* Force usage */
    volatile_var = result3[0] + result3[1] + result3[2] + result3[3];
    MEMORY_BARRIER();
}

/* ========== SSA_NAME generation ========== */
int test_ssa(int n) {
    int x = 0;
    int y = 0;
    
    /* Complex loop with multiple branches to force SSA */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            x = i * 2;
        } else if (i % 3 == 0) {
            x = i * 3;
        } else {
            x = i;
        }
        
        if (i % 5 == 0) {
            y = x + i;
        } else {
            y = x - i;
        }
        
        volatile_var += y;
    }
    
    MEMORY_BARRIER();
    return x + y;
}

/* ========== BLOCK generation ========== */
void test_blocks(void) {
    /* Outer block with variables */
    int outer = 10;
    
    {
        /* Nested block 1 */
        int block1_var = outer * 2;
        
        {
            /* Nested block 2 */
            int block2_var = block1_var + 5;
            
            {
                /* Nested block 3 */
                int block3_var = block2_var * 3;
                volatile_var += block3_var;
                
                if (volatile_var > 0) {
                    /* Block inside if */
                    int if_block_var = block3_var / 2;
                    volatile_var -= if_block_var;
                }
            }
        }
    }
    
    /* Loop with block */
    for (int i = 0; i < 5; i++) {
        int loop_block_var = i * i;
        volatile_var += loop_block_var;
        
        {
            int inner_loop_var = loop_block_var + 1;
            volatile_var -= inner_loop_var;
        }
    }
    
    MEMORY_BARRIER();
}

/* ========== CONSTRUCTOR generation ========== */
struct ComplexStruct {
    int a;
    int b;
    int c;
    int d;
};

int global_func(void) {
    return volatile_var + 1;
}

void test_constructors(void) {
    /* Non-constant initializers for aggregates */
    struct ComplexStruct s1 = {
        .a = global_func(),
        .b = volatile_var * 2,
        .c = 3,
        .d = 4
    };
    
    struct ComplexStruct s2 = {
        global_func() + 1,
        volatile_var - 1,
        5,
        6
    };
    
    /* Array with non-constant initializers */
    int arr[4] = {
        global_func(),
        volatile_var,
        s1.a + s2.b,
        s1.c * s2.d
    };
    
    /* Nested struct initialization */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    };
    
    struct Nested nested = {
        .inner = {
            .a = global_func(),
            .b = 2,
            .c = 3,
            .d = 4
        },
        .extra = 5
    };
    
    volatile_var = s1.a + s2.b + arr[0] + nested.inner.c;
    MEMORY_BARRIER();
}

/* ========== OMP_CLAUSE generation ========== */
#ifdef _OPENMP
#include <omp.h>

void test_omp_clauses(void) {
    int i;
    int sum = 0;
    int arr[100];
    int private_var = 0;
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP with multiple clauses to generate various OMP_CLAUSE nodes */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) \
        schedule(dynamic) num_threads(4) if(volatile_var > 0)
    for (i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel private(private_var) firstprivate(sum)
    {
        private_var = omp_get_thread_num();
        #pragma omp critical
        {
            volatile_var += private_var + sum;
        }
    }
    
    /* OpenMP sections with clauses */
    #pragma omp parallel sections private(i) lastprivate(private_var)
    {
        #pragma omp section
        {
            private_var = 1;
        }
        #pragma omp section
        {
            private_var = 2;
        }
    }
    
    MEMORY_BARRIER();
}
#endif

/* ========== C++ specific for TREE_BINFO ========== */
#ifdef __cplusplus

class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int method() { return 1; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int method() override { return 2; }
    int derived_data;
};

class AnotherDerived : public BaseClass {
public:
    virtual int method() override { return 3; }
    int another_data;
};

void test_binfo(void) {
    BaseClass* base_ptr;
    DerivedClass derived_obj;
    AnotherDerived another_obj;
    
    /* Use polymorphism to trigger BINFO nodes */
    if (volatile_var % 2 == 0) {
        base_ptr = &derived_obj;
    } else {
        base_ptr = &another_obj;
    }
    
    volatile_var = base_ptr->method();
    
    /* Multiple inheritance-like access */
    BaseClass& base_ref = derived_obj;
    volatile_var += base_ref.method();
    
    MEMORY_BARRIER();
}

#else
/* C version - try to trigger BINFO through LTO structures */
struct BaseStruct {
    int type_id;
    void* vtable;  /* Simulate virtual table */
};

struct DerivedStruct {
    struct BaseStruct base;
    int derived_data;
};

void test_binfo(void) {
    /* This may generate BINFO-like nodes with -flto */
    struct DerivedStruct d;
    d.base.type_id = 1;
    d.derived_data = volatile_var;
    
    volatile_var = d.base.type_id + d.derived_data;
    MEMORY_BARRIER();
}
#endif

/* ========== Main function ========== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Test all tree node types */
    test_identifiers();
    result += volatile_var;
    
    test_vectors();
    result += volatile_var;
    
    result += test_ssa(100);
    
    test_blocks();
    result += volatile_var;
    
    test_constructors();
    result += volatile_var;
    
    #ifdef _OPENMP
    test_omp_clauses();
    result += volatile_var;
    #endif
    
    test_binfo();
    result += volatile_var;
    
    /* Final computation to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result == 0 ? 0 : 1;
}
