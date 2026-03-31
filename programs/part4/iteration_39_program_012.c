/* test_tree_coverage.c - Comprehensive test to trigger specific tree node classifications */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure tree nodes are fully built */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* ========== IDENTIFIER_NODE generation ========== */
/* Generate many unique identifiers using macros */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)
#define MAKE_FUNC(n) CONCAT(test_func_, n)

/* Declare many variables with unique names */
volatile int MAKE_ID(0), MAKE_ID(1), MAKE_ID(2), MAKE_ID(3), MAKE_ID(4);
volatile int MAKE_ID(5), MAKE_ID(6), MAKE_ID(7), MAKE_ID(8), MAKE_ID(9);

NOINLINE void test_identifiers(void) {
    /* Use all identifiers in complex expressions */
    int result = 
        MAKE_ID(0) + MAKE_ID(1) * MAKE_ID(2) - MAKE_ID(3) / (MAKE_ID(4) + 1) +
        MAKE_ID(5) % (MAKE_ID(6) + 1) | MAKE_ID(7) & MAKE_ID(8) ^ MAKE_ID(9);
    
    /* Memory barrier to prevent optimization */
    asm volatile("" : : : "memory");
    
    /* Use result to prevent dead code elimination */
    MAKE_ID(0) = result;
}

/* ========== TREE_VEC generation ========== */
/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE v4si test_vectors(v4si a, v4si b) {
    /* Various vector operations */
    v4si add = a + b;
    v4si mul = a * b;
    v4si sub = a - b;
    v4si shl = a << b;
    v4si shr = a >> b;
    
    /* Mixed vector operations */
    v4sf fvec = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fadd = fvec + fvec;
    
    /* Vector comparisons */
    v4si cmp = a > b;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    return add + mul + sub + shl + shr + cmp;
}

/* ========== SSA_NAME generation ========== */
NOINLINE int test_ssa(int n) {
    int x = 0, y = 0, z = 0;
    
    /* Complex loop with multiple branches to create SSA form */
    for (int i = 0; i < n; i++) {
        /* Nested conditionals create phi nodes */
        if (i % 3 == 0) {
            x = i * 2;
        } else if (i % 3 == 1) {
            x = i * 3;
        } else {
            x = i * 4;
        }
        
        /* Another variable with complex SSA */
        if (i % 2 == 0) {
            y = x + i;
        } else {
            y = x - i;
        }
        
        /* Third variable with loop-carried dependency */
        z = z + y;
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    return x + y + z;
}

/* ========== BLOCK generation ========== */
NOINLINE int test_blocks(int n) {
    int outer = 0;
    
    /* Outer block */
    {
        int block_var1 = n * 2;
        
        /* Nested block 1 */
        {
            int block_var2 = block_var1 + 5;
            
            /* Deeply nested block */
            {
                int block_var3 = block_var2 * 3;
                outer += block_var3;
                
                /* Block in loop */
                for (int i = 0; i < 3; i++) {
                    int loop_block_var = i * block_var3;
                    outer += loop_block_var;
                    
                    /* Another nested block */
                    {
                        int inner_block = loop_block_var / 2;
                        outer -= inner_block;
                    }
                }
            }
        }
        
        /* Another sibling block */
        {
            int sibling_var = outer * 7;
            outer = sibling_var % 11;
        }
    }
    
    /* Block with switch */
    {
        int switch_var = outer;
        switch (n % 4) {
            case 0: {
                int case_var = 1;
                outer += case_var;
                break;
            }
            case 1: {
                int case_var = 2;
                outer += case_var;
                break;
            }
            default: {
                int case_var = 3;
                outer += case_var;
                break;
            }
        }
    }
    
    return outer;
}

/* ========== CONSTRUCTOR generation ========== */
struct ComplexStruct {
    int a, b, c;
    float f;
    double d;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

NOINLINE struct ComplexStruct test_constructor(int x, float y) {
    /* Non-constant initializers */
    int dynamic_val = x * 2;
    float dynamic_float = y * 3.14f;
    
    /* Constructor with mixed initializers */
    struct ComplexStruct s1 = {
        .a = dynamic_val,
        .b = x + 5,
        .c = dynamic_val % 7,
        .f = dynamic_float,
        .d = (double)dynamic_val / 2.0
    };
    
    /* Array with non-constant initializers */
    int arr[4] = {
        dynamic_val,
        x * 3,
        dynamic_val + x,
        test_ssa(2)  /* Function call in initializer */
    };
    
    /* Nested struct constructor */
    struct NestedStruct ns = {
        .inner = {
            .a = arr[0],
            .b = arr[1],
            .c = arr[2],
            .f = dynamic_float + 1.0f,
            .d = s1.d * 2.0
        },
        .extra = dynamic_val * 2
    };
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Use all constructs */
    s1.a += ns.extra + arr[3];
    return s1;
}

/* ========== OMP_CLAUSE generation ========== */
#ifdef _OPENMP
#include <omp.h>

NOINLINE int test_omp_clauses(int n) {
    int sum = 0;
    int private_var = 0;
    static int shared_var = 0;
    
    /* OpenMP parallel with multiple clauses */
    #pragma omp parallel for private(private_var) shared(shared_var) \
            reduction(+:sum) schedule(dynamic) num_threads(4) \
            firstprivate(n) if(n > 100)
    for (int i = 0; i < n; i++) {
        private_var = i * 2;
        sum += private_var;
        shared_var++;
        
        /* Nested OpenMP construct */
        #pragma omp atomic
        sum += 1;
    }
    
    /* OpenMP sections with different clauses */
    #pragma omp parallel sections private(private_var) \
            reduction(*:sum) nowait
    {
        #pragma omp section
        {
            private_var = 1;
            sum *= private_var;
        }
        
        #pragma omp section
        {
            private_var = 2;
            sum *= private_var;
        }
    }
    
    /* OpenMP task with dependencies */
    #pragma omp parallel
    #pragma omp single
    {
        int task_var = 0;
        #pragma omp task shared(task_var) depend(out: task_var)
        {
            task_var = sum;
        }
        
        #pragma omp task shared(task_var) depend(in: task_var)
        {
            sum += task_var;
        }
    }
    
    return sum + shared_var;
}
#endif

/* ========== TREE_BINFO generation (C++ only) ========== */
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

class MultipleBase1 {
public:
    virtual ~MultipleBase1() {}
    int data1;
};

class MultipleBase2 {
public:
    virtual ~MultipleBase2() {}
    int data2;
};

class MultipleDerived : public MultipleBase1, public MultipleBase2 {
public:
    int derived_data;
};

NOINLINE int test_binfo(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    BaseClass& base_ref = derived;
    
    MultipleDerived multiple;
    MultipleBase1* mb1 = &multiple;
    MultipleBase2* mb2 = &multiple;
    
    /* Virtual calls to use vtable */
    int result = base_ptr->method() + base_ref.method();
    result += derived.method();
    
    /* Access through different base pointers */
    result += mb1->data1 + mb2->data2;
    
    /* Type comparisons */
    if (dynamic_cast<DerivedClass*>(base_ptr)) {
        result += 10;
    }
    
    return result;
}
#endif

/* ========== Main orchestration ========== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Test all tree node types */
    test_identifiers();
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_result = test_vectors(vec_a, vec_b);
    result += vec_result[0];
    
    result += test_ssa(100);
    result += test_blocks(50);
    
    struct ComplexStruct cs = test_constructor(10, 3.14f);
    result += cs.a + cs.b + cs.c;
    
    #ifdef _OPENMP
    result += test_omp_clauses(200);
    #endif
    
    #ifdef __cplusplus
    result += test_binfo();
    #endif
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Additional volatile store */
    volatile int* volatile_ptr = (volatile int*)malloc(sizeof(int));
    if (volatile_ptr) {
        *volatile_ptr = result;
        free((void*)volatile_ptr);
    }
    
    return result != 0 ? 0 : 1;
}
