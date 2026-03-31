/* test_tree_coverage.c - Comprehensive test for GCC tree node coverage */

/* Prevent optimizations from removing our test constructs */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

#include <stdio.h>
#include <stdlib.h>

/* ========== TREE_VEC and SSA_NAME coverage ========== */
/* Vector types for TREE_VEC */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE void test_vectors_and_ssa(int n) {
    volatile int trigger = 0;
    asm volatile("" : : : "memory");
    
    /* TREE_VEC: Vector operations */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = vec_a + vec_b;
    v4si vec_d = vec_a * vec_b;
    
    /* SSA_NAME: Complex loop with conditional assignments */
    int ssa_var1 = 0;
    int ssa_var2 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Nested BLOCK inside loop */
        {
            int temp = i * 2;
            asm volatile("" : "+r"(temp) : : "memory");
            
            if (i % 3 == 0) {
                ssa_var1 = temp + vec_a[0];
            } else if (i % 3 == 1) {
                ssa_var1 = temp + vec_b[1];
            } else {
                ssa_var1 = temp + vec_c[2];
            }
            
            /* Another conditional for SSA */
            ssa_var2 = (i % 2 == 0) ? ssa_var1 * 2 : ssa_var1 / 2;
        }
        
        /* Use volatile to prevent optimization */
        trigger += ssa_var2;
    }
    
    /* Use the results */
    asm volatile("" : : "r"(vec_c), "r"(vec_d), "r"(trigger) : "memory");
}

/* ========== IDENTIFIER_NODE and BLOCK coverage ========== */
/* Generate many unique identifiers */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

NOINLINE void test_identifiers_and_blocks(int iterations) {
    /* Multiple identifiers in different scopes */
    int MAKE_ID(1) = 1;
    int MAKE_ID(2) = 2;
    int MAKE_ID(3) = 3;
    int MAKE_ID(4) = 4;
    int MAKE_ID(5) = 5;
    
    /* Deeply nested blocks for BLOCK nodes */
    {
        int block_local_1 = MAKE_ID(1) * 2;
        
        {
            int block_local_2 = block_local_1 + MAKE_ID(2);
            
            {
                int block_local_3 = block_local_2 * MAKE_ID(3);
                
                {
                    int block_local_4 = block_local_3 / MAKE_ID(4);
                    asm volatile("" : "+r"(block_local_4) : : "memory");
                }
            }
        }
    }
    
    /* More identifiers in loop */
    for (int i = 0; i < iterations; i++) {
        /* BLOCK inside loop */
        {
            int loop_local = i + MAKE_ID(5);
            volatile int loop_volatile = loop_local;
            
            /* Switch with multiple cases creates more blocks */
            switch (i % 4) {
                case 0: {
                    int case_local = loop_volatile * 2;
                    asm volatile("" : "+r"(case_local) : : "memory");
                    break;
                }
                case 1: {
                    int case_local = loop_volatile + 3;
                    asm volatile("" : "+r"(case_local) : : "memory");
                    break;
                }
                case 2: {
                    int case_local = loop_volatile - 4;
                    asm volatile("" : "+r"(case_local) : : "memory");
                    break;
                }
                default: {
                    int case_local = loop_volatile / 2;
                    asm volatile("" : "+r"(case_local) : : "memory");
                    break;
                }
            }
        }
    }
    
    /* Function calls with different identifiers */
    int result_alpha = MAKE_ID(1) + MAKE_ID(2);
    int result_beta = MAKE_ID(3) * MAKE_ID(4);
    int result_gamma = result_alpha - result_beta;
    int result_delta = result_gamma / MAKE_ID(5);
    
    asm volatile("" : : "r"(result_delta) : "memory");
}

/* ========== CONSTRUCTOR coverage ========== */
struct ComplexStruct {
    int a, b, c;
    float x, y, z;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

NOINLINE int compute_value(int seed) {
    volatile int temp = seed * 1103515245 + 12345;
    return (temp >> 16) & 0x7FFF;
}

NOINLINE void test_constructors(void) {
    /* Non-constant initializers for CONSTRUCTOR nodes */
    struct ComplexStruct s1 = {
        .a = compute_value(1),
        .b = compute_value(2),
        .c = compute_value(3),
        .x = compute_value(4) / 1000.0f,
        .y = compute_value(5) / 1000.0f,
        .z = compute_value(6) / 1000.0f
    };
    
    /* Array with non-constant initializers */
    int dynamic_array[5] = {
        compute_value(10),
        compute_value(11),
        compute_value(12),
        compute_value(13),
        compute_value(14)
    };
    
    /* Nested struct constructor */
    struct NestedStruct ns = {
        .inner = {
            .a = compute_value(20),
            .b = compute_value(21),
            .c = compute_value(22),
            .x = 1.5f,
            .y = 2.5f,
            .z = 3.5f
        },
        .extra = compute_value(30)
    };
    
    /* Use all constructors to prevent optimization */
    volatile int sum = 0;
    sum += s1.a + s1.b + s1.c;
    sum += (int)(s1.x + s1.y + s1.z);
    
    for (int i = 0; i < 5; i++) {
        sum += dynamic_array[i];
    }
    
    sum += ns.inner.a + ns.inner.b + ns.inner.c;
    sum += ns.extra;
    
    asm volatile("" : : "r"(sum) : "memory");
}

/* ========== OMP_CLAUSE coverage ========== */
#ifdef _OPENMP
#include <omp.h>

NOINLINE void test_omp_clauses(int size) {
    int i;
    int *array = (int*)malloc(size * sizeof(int));
    int sum = 0;
    int max_val = 0;
    int min_val = 0;
    
    if (!array) return;
    
    /* Initialize array */
    for (i = 0; i < size; i++) {
        array[i] = compute_value(i);
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel shared(array, size) private(i) \
                         reduction(+:sum) reduction(max:max_val) \
                         reduction(min:min_val) default(none)
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for schedule(dynamic, 4) nowait
        for (i = 0; i < size; i++) {
            sum += array[i];
            
            /* Nested BLOCK in OpenMP region */
            {
                int local_calc = array[i] * thread_id;
                if (local_calc > max_val) max_val = local_calc;
                if (local_calc < min_val) min_val = local_calc;
            }
        }
        
        /* OpenMP barrier */
        #pragma omp barrier
        
        /* OpenMP single with copyprivate */
        #pragma omp single copyprivate(thread_id)
        {
            thread_id = -1;
        }
        
        /* Another parallel for with collapse */
        #pragma omp for collapse(2) schedule(static)
        for (int x = 0; x < 10; x++) {
            for (int y = 0; y < 10; y++) {
                int idx = x * 10 + y;
                if (idx < size) {
                    array[idx] += x + y;
                }
            }
        }
    }
    
    /* OpenMP task with depend clause */
    #pragma omp parallel
    #pragma omp single
    {
        int task_result1 = 0, task_result2 = 0;
        
        #pragma omp task depend(out: task_result1)
        {
            for (i = 0; i < size/2; i++) {
                task_result1 += array[i];
            }
        }
        
        #pragma omp task depend(out: task_result2)
        {
            for (i = size/2; i < size; i++) {
                task_result2 += array[i];
            }
        }
        
        #pragma omp task depend(in: task_result1, task_result2)
        {
            sum = task_result1 + task_result2;
        }
        
        #pragma omp taskwait
    }
    
    free(array);
    
    /* Use results */
    volatile int final = sum + max_val + min_val;
    asm volatile("" : : "r"(final) : "memory");
}
#endif

/* ========== TREE_BINFO coverage (C++ version) ========== */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int method1() { return 1; }
    virtual int method2() { return 2; }
    int data1;
    float data2;
};

class DerivedClass : public BaseClass {
public:
    virtual int method1() override { return 3; }
    virtual int method3() { return 4; }
    int extra_data;
};

class SecondDerived : public DerivedClass {
public:
    virtual int method2() override { return 5; }
    int more_data;
};

NOINLINE void test_binfo(void) {
    BaseClass* base_ptr;
    DerivedClass derived_obj;
    SecondDerived second_obj;
    
    /* Use polymorphism to trigger BINFO nodes */
    volatile int choice = compute_value(100) % 3;
    
    switch (choice) {
        case 0:
            base_ptr = &derived_obj;
            break;
        case 1:
            base_ptr = &second_obj;
            break;
        default:
            base_ptr = new BaseClass();
            break;
    }
    
    /* Virtual calls */
    int result = base_ptr->method1() + base_ptr->method2();
    
    /* Dynamic cast (requires RTTI) */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        result += derived_ptr->method3();
    }
    
    /* Cleanup */
    if (choice == 2) {
        delete base_ptr;
    }
    
    asm volatile("" : : "r"(result) : "memory");
}
#endif

/* ========== Main function ========== */
int main(int argc, char** argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Test all tree node types */
    test_identifiers_and_blocks(iterations);
    test_vectors_and_ssa(iterations);
    test_constructors();
    
    #ifdef _OPENMP
    test_omp_clauses(iterations);
    #endif
    
    #ifdef __cplusplus
    test_binfo();
    #endif
    
    /* Final computation to use all results */
    volatile int final_result = compute_value(iterations);
    printf("Test completed with result: %d\n", final_result);
    
    return 0;
}
