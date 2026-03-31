/* test_tree_coverage.c - Comprehensive test to trigger specific tree node classifications in GCC */

/* Enable OpenMP support */
#ifdef _OPENMP
#include <omp.h>
#endif

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization from removing our test constructs */
static volatile int volatile_guard = 0;

/* Memory barrier to prevent reordering */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* ====== 1. IDENTIFIER_NODE generation ====== */
/* Generate many unique identifiers using macros */
#define CONCAT(a, b) a##b
#define MAKE_IDENTIFIER(prefix, num) CONCAT(prefix, num)

/* Function with many parameters and local variables */
int test_identifiers_and_blocks(int param1, int param2, int param3, 
                                int param4, int param5, int param6) {
    /* Multiple local variables with unique names */
    int MAKE_IDENTIFIER(local_var_, 1) = param1 + 1;
    int MAKE_IDENTIFIER(local_var_, 2) = param2 * 2;
    int MAKE_IDENTIFIER(local_var_, 3) = param3 - 3;
    int MAKE_IDENTIFIER(local_var_, 4) = param4 / 4;
    int MAKE_IDENTIFIER(local_var_, 5) = param5 ^ 5;
    int MAKE_IDENTIFIER(local_var_, 6) = param6 | 6;
    
    /* Nested blocks (for BLOCK nodes) */
    {
        int nested_var_1 = MAKE_IDENTIFIER(local_var_, 1) * 2;
        {
            int deeply_nested_var = nested_var_1 + 100;
            MEMORY_BARRIER();
            volatile_guard = deeply_nested_var;
        }
    }
    
    {
        int nested_var_2 = MAKE_IDENTIFIER(local_var_, 2) / 2;
        MEMORY_BARRIER();
    }
    
    /* More identifiers in expressions */
    int result = MAKE_IDENTIFIER(local_var_, 1) + 
                 MAKE_IDENTIFIER(local_var_, 2) - 
                 MAKE_IDENTIFIER(local_var_, 3) * 
                 MAKE_IDENTIFIER(local_var_, 4) / 
                 MAKE_IDENTIFIER(local_var_, 5) ^ 
                 MAKE_IDENTIFIER(local_var_, 6);
    
    MEMORY_BARRIER();
    return result;
}

/* ====== 2. TREE_VEC generation ====== */
/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

v4si test_vector_operations(v4si a, v4si b) {
    /* Various vector operations to generate TREE_VEC nodes */
    v4si add_result = a + b;
    v4si sub_result = a - b;
    v4si mul_result = a * b;
    v4si div_result = a / (b + (v4si){1,1,1,1}); /* Avoid division by zero */
    
    /* Vector comparisons and blends */
    v4si cmp_result = a > b;
    v4si blend_result = cmp_result ? a : b;
    
    /* Vector shuffling (creates TREE_VEC for permute indices) */
    v4si shuffled = __builtin_shuffle(a, b, (v4si){3,2,1,0});
    
    MEMORY_BARRIER();
    return add_result + sub_result + mul_result + div_result + blend_result + shuffled;
}

/* ====== 3. SSA_NAME generation ====== */
int test_ssa_formation(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Complex loop with multiple assignments to create SSA form */
    for (int i = 0; i < n; i++) {
        MEMORY_BARRIER();
        
        /* Multiple conditional assignments to the same variable */
        if (i % 3 == 0) {
            x = y + z;
        } else if (i % 3 == 1) {
            x = y - z;
        } else {
            x = y * z;
        }
        
        /* Phi nodes will be created for x in SSA form */
        if (x > 100) {
            y = x / 2;
        } else {
            y = x * 2;
        }
        
        /* Another variable with multiple definitions */
        if (i % 2 == 0) {
            z = i + volatile_guard;
        } else {
            z = i - volatile_guard;
        }
        
        volatile_guard = i; /* Prevent dead code elimination */
    }
    
    /* Complex expression with multiple uses */
    int result = x + y - z * (x - y) / (z + 1);
    MEMORY_BARRIER();
    return result;
}

/* ====== 4. CONSTRUCTOR generation ====== */
struct ComplexStruct {
    int a;
    float b;
    double c;
    long d;
};

int test_aggregate_initialization(int base) {
    /* Non-constant initializers for aggregates */
    int non_const = base * 2 + volatile_guard;
    
    /* Array with non-constant initializer */
    int dynamic_array[4] = {
        non_const,
        non_const + 1,
        non_const * 2,
        test_ssa_formation(5)  /* Function call in initializer */
    };
    
    /* Struct with designated initializers and non-constant values */
    struct ComplexStruct s = {
        .a = dynamic_array[0],
        .b = (float)dynamic_array[1] / 2.0f,
        .c = (double)dynamic_array[2] * 1.5,
        .d = (long)dynamic_array[3] << 2
    };
    
    /* Nested struct initialization */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    } nested = {
        .inner = s,
        .extra = test_identifiers_and_blocks(1, 2, 3, 4, 5, 6)
    };
    
    /* Union with non-constant initializer */
    union DataUnion {
        int i;
        float f;
        void* p;
    } data = { .i = non_const };
    
    MEMORY_BARRIER();
    return s.a + nested.extra + data.i + dynamic_array[0];
}

/* ====== 5. OMP_CLAUSE generation ====== */
#ifdef _OPENMP
int test_omp_clauses(int size) {
    int sum = 0;
    int* array = (int*)malloc(size * sizeof(int));
    
    if (!array) return -1;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel shared(array) firstprivate(size) reduction(+:sum) \
            private(volatile_guard) if(size > 1000)
    {
        int local_volatile = volatile_guard;
        
        #pragma omp for schedule(dynamic, 4) nowait
        for (int i = 0; i < size; i++) {
            sum += array[i] * (i + 1);
            local_volatile = i; /* Use private variable */
        }
        
        /* Nested OpenMP construct */
        #pragma omp master
        {
            volatile_guard = local_volatile;
        }
        
        /* Barrier with clause */
        #pragma omp barrier
        
        /* Single directive with copyprivate */
        #pragma omp single copyprivate(local_volatile)
        {
            local_volatile = 42;
        }
    }
    
    /* OpenMP sections with different clauses */
    int section_result = 0;
    #pragma omp parallel sections private(volatile_guard) \
            reduction(+:section_result)
    {
        #pragma omp section
        {
            section_result += 1;
        }
        
        #pragma omp section
        {
            section_result += 2;
        }
        
        #pragma omp section
        {
            section_result += 3;
        }
    }
    
    free(array);
    MEMORY_BARRIER();
    return sum + section_result;
}
#endif

/* ====== 6. C++ specific for TREE_BINFO (if compiled as C++) ====== */
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

class MultipleDerived : public BaseClass {
public:
    virtual int method() override { return 3; }
    int multiple_data;
};

int test_cpp_inheritance() {
    BaseClass* base_ptr;
    DerivedClass derived_obj;
    MultipleDerived multiple_obj;
    
    /* Use polymorphism to trigger BINFO nodes */
    base_ptr = &derived_obj;
    int result1 = base_ptr->method();
    
    base_ptr = &multiple_obj;
    int result2 = base_ptr->method();
    
    /* Access through references */
    BaseClass& base_ref = derived_obj;
    int result3 = base_ref.method();
    
    /* Type casting operations */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        result3 += derived_ptr->derived_data;
    }
    
    MEMORY_BARRIER();
    return result1 + result2 + result3;
}
#endif

/* ====== 7. Additional BLOCK node generation ====== */
int test_more_blocks(int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Block inside loop */
        {
            int loop_local = i * 2;
            
            /* Nested block */
            {
                int inner_local = loop_local + 1;
                
                /* Deeply nested block */
                {
                    int deepest = inner_local * 3;
                    total += deepest;
                    MEMORY_BARRIER();
                }
            }
        }
        
        /* Another block with switch statement */
        switch (i % 4) {
            case 0: {
                int case_local = total + 1;
                total = case_local;
                break;
            }
            case 1: {
                int case_local = total * 2;
                total = case_local;
                break;
            }
            default: {
                int case_local = total / 2;
                total = case_local;
                break;
            }
        }
    }
    
    return total;
}

/* ====== Main function to orchestrate everything ====== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* 1. Test identifiers and blocks */
    result += test_identifiers_and_blocks(10, 20, 30, 40, 50, 60);
    
    /* 2. Test vector operations */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_result = test_vector_operations(vec_a, vec_b);
    result += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    
    /* 3. Test SSA formation */
    result += test_ssa_formation(100);
    
    /* 4. Test aggregate initialization */
    result += test_aggregate_initialization(50);
    
    /* 5. Test OpenMP clauses if available */
    #ifdef _OPENMP
    result += test_omp_clauses(500);
    #endif
    
    /* 6. Test C++ inheritance if compiled as C++ */
    #ifdef __cplusplus
    result += test_cpp_inheritance();
    #endif
    
    /* 7. Test more block generation */
    result += test_more_blocks(50);
    
    /* Final memory barrier and output */
    MEMORY_BARRIER();
    printf("Final result: %d\n", result);
    
    /* Use result to prevent optimization */
    volatile_guard = result;
    
    return result != 0 ? 0 : 1;
}
