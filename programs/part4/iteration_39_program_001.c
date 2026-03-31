/* test_tree_coverage.c - Comprehensive test for GCC tree node coverage */

/* Enable OpenMP support */
#ifdef _OPENMP
#include <omp.h>
#endif

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization from removing our test constructs */
static volatile int volatile_var = 0;

/* Memory barrier to prevent reordering */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* ========== TARGET 1: IDENTIFIER_NODE ========== */
/* Generate many distinct identifiers using macros */
#define CONCAT(a, b) a##b
#define MAKE_IDENTIFIER(n) CONCAT(identifier_, n)

/* Generate multiple identifiers in a function */
static void test_identifiers(void) {
    /* Create many distinct identifiers */
    int MAKE_IDENTIFIER(0) = 1;
    int MAKE_IDENTIFIER(1) = 2;
    int MAKE_IDENTIFIER(2) = 3;
    int MAKE_IDENTIFIER(3) = 4;
    int MAKE_IDENTIFIER(4) = 5;
    int MAKE_IDENTIFIER(5) = 6;
    int MAKE_IDENTIFIER(6) = 7;
    int MAKE_IDENTIFIER(7) = 8;
    int MAKE_IDENTIFIER(8) = 9;
    int MAKE_IDENTIFIER(9) = 10;
    
    /* Use them in a complex expression */
    volatile_var = MAKE_IDENTIFIER(0) + MAKE_IDENTIFIER(1) + 
                   MAKE_IDENTIFIER(2) + MAKE_IDENTIFIER(3) +
                   MAKE_IDENTIFIER(4) + MAKE_IDENTIFIER(5) +
                   MAKE_IDENTIFIER(6) + MAKE_IDENTIFIER(7) +
                   MAKE_IDENTIFIER(8) + MAKE_IDENTIFIER(9);
    MEMORY_BARRIER();
}

/* ========== TARGET 2: TREE_VEC ========== */
/* Use GCC vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

static void test_vectors(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Complex vector operations to generate TREE_VEC nodes */
    v4si result1 = a + b;
    v4si result2 = a * b;
    v4si result3 = result1 - result2;
    v4si result4 = result3 + c;
    
    /* Force the compiler to keep the vector computations */
    volatile v4si *volatile_ptr = &result4;
    MEMORY_BARRIER();
    
    /* Use float vectors too */
    v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fb = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fc = fa * fb + fa - fb;
    
    volatile_var += result4[0] + (int)fc[0];
}

/* ========== TARGET 3: SSA_NAME ========== */
/* Create complex control flow for SSA generation */
static int test_ssa_names(int n) {
    int x = 0, y = 0, z = 0;
    
    /* Loop with multiple assignments to create phi nodes */
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            x = i * 2;
        } else if (i % 3 == 1) {
            x = i + 5;
        } else {
            x = i - 3;
        }
        
        /* Nested conditionals */
        if (x > 10) {
            y = x * 3;
        } else {
            y = x / 2;
        }
        
        /* Another variable with complex data flow */
        z = z + y;
        
        /* Prevent loop unrolling */
        if (i % 7 == 0) {
            MEMORY_BARRIER();
        }
    }
    
    /* Complex expression with multiple uses */
    return x + y * 2 - z / 3;
}

/* ========== TARGET 4: BLOCK ========== */
static void test_blocks(void) {
    int outer = 0;
    
    /* Level 1 block */
    {
        int level1_a = 1;
        int level1_b = 2;
        
        /* Level 2 block */
        {
            int level2_a = level1_a * 3;
            int level2_b = level1_b + 4;
            
            /* Level 3 block inside if statement */
            if (level2_a > 0) {
                int level3_a = level2_a * level2_b;
                outer += level3_a;
                
                /* Level 4 block inside loop */
                for (int i = 0; i < 3; i++) {
                    int level4_a = level3_a + i;
                    outer += level4_a;
                    
                    /* Level 5 block */
                    {
                        int level5_a = level4_a * 2;
                        outer -= level5_a;
                    }
                }
            }
        }
        
        /* Another block after the nested ones */
        {
            int post_block_var = outer * 2;
            volatile_var += post_block_var;
        }
    }
    
    /* Block attached to switch */
    switch (outer % 4) {
        case 0: {
            int case0_var = outer + 1;
            volatile_var += case0_var;
            break;
        }
        case 1: {
            int case1_var = outer * 2;
            volatile_var += case1_var;
            break;
        }
        default: {
            int default_var = outer / 2;
            volatile_var += default_var;
            break;
        }
    }
}

/* ========== TARGET 5: CONSTRUCTOR ========== */
struct ComplexStruct {
    int a;
    int b;
    int c;
    int d;
};

static int global_counter = 0;

static int get_next_value(void) {
    return global_counter++ + 1;
}

static void test_constructors(void) {
    /* Non-constant struct initializer */
    struct ComplexStruct s1 = {
        .a = get_next_value(),
        .b = get_next_value() * 2,
        .c = get_next_value() + 5,
        .d = get_next_value() - 3
    };
    
    /* Array with non-constant initializers */
    int arr[5] = {
        get_next_value(),
        get_next_value() * 2,
        s1.a + s1.b,
        s1.c - s1.d,
        get_next_value() % 10
    };
    
    /* Nested struct with constructor */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    };
    
    struct Nested n1 = {
        .inner = {
            .a = get_next_value(),
            .b = get_next_value(),
            .c = get_next_value(),
            .d = get_next_value()
        },
        .extra = get_next_value()
    };
    
    /* Use the constructed values */
    volatile_var += s1.a + arr[2] + n1.inner.b + n1.extra;
    MEMORY_BARRIER();
}

/* ========== TARGET 6: OMP_CLAUSE ========== */
#ifdef _OPENMP
static void test_omp_clauses(void) {
    int i;
    const int N = 100;
    int arr[N];
    int sum = 0;
    int private_var = 0;
    
    /* Initialize array */
    for (i = 0; i < N; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) \
        schedule(dynamic, 4) num_threads(4) if(N > 50)
    for (i = 0; i < N; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel private(private_var) firstprivate(sum)
    {
        private_var = omp_get_thread_num();
        
        #pragma omp for nowait
        for (i = 0; i < 10; i++) {
            private_var += i;
        }
        
        #pragma omp critical
        {
            volatile_var += private_var;
        }
    }
    
    /* OpenMP sections with clause */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < 5; i++) {
                volatile_var += i;
            }
        }
        
        #pragma omp section
        {
            for (i = 5; i < 10; i++) {
                volatile_var -= i;
            }
        }
    }
    
    MEMORY_BARRIER();
}
#else
static void test_omp_clauses(void) {
    /* Dummy implementation when OpenMP is not available */
    volatile_var += 42;
}
#endif

/* ========== TARGET 7: TREE_BINFO (C++ only) ========== */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int method1() { return 1; }
    virtual int method2() { return 2; }
    int data1;
    int data2;
};

class DerivedClass : public BaseClass {
public:
    virtual int method1() override { return 3; }
    virtual int method3() { return 4; }
    int data3;
};

class SecondDerived : public DerivedClass {
public:
    virtual int method2() override { return 5; }
    int data4;
};

static void test_binfo(void) {
    BaseClass* base_ptr;
    DerivedClass derived_obj;
    SecondDerived second_derived_obj;
    
    /* Use polymorphism to trigger BINFO nodes */
    base_ptr = &derived_obj;
    volatile_var += base_ptr->method1();
    
    base_ptr = &second_derived_obj;
    volatile_var += base_ptr->method2();
    
    /* Multiple inheritance-like access pattern */
    DerivedClass* derived_ptr = static_cast<DerivedClass*>(base_ptr);
    volatile_var += derived_ptr->method3();
    
    /* Access through references */
    BaseClass& base_ref = derived_obj;
    volatile_var += base_ref.method1();
    
    MEMORY_BARRIER();
}
#else
/* C version - try to trigger BINFO through LTO */
struct BaseStruct {
    int (*method1)(struct BaseStruct*);
    int data1;
};

struct DerivedStruct {
    struct BaseStruct base;
    int data2;
};

static int base_method1(struct BaseStruct* self) { return 1; }
static int derived_method1(struct BaseStruct* self) { return 2; }

static void test_binfo(void) {
    struct DerivedStruct derived;
    derived.base.method1 = derived_method1;
    
    /* Use function pointer through base */
    struct BaseStruct* base_ptr = (struct BaseStruct*)&derived;
    volatile_var += base_ptr->method1(base_ptr);
    
    MEMORY_BARRIER();
}
#endif

/* ========== MAIN FUNCTION ========== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Run all tests multiple times to increase coverage probability */
    for (int iteration = 0; iteration < 3; iteration++) {
        test_identifiers();
        test_vectors();
        result += test_ssa_names(50 + iteration);
        test_blocks();
        test_constructors();
        test_omp_clauses();
        test_binfo();
        
        /* Add some variation based on iteration */
        volatile_var += iteration * 7;
        MEMORY_BARRIER();
    }
    
    /* Final computation to use all results */
    result += volatile_var;
    
    printf("Result: %d\n", result);
    
    /* Return non-zero if result is "interesting" */
    return (result % 256) == 0 ? 0 : 1;
}
