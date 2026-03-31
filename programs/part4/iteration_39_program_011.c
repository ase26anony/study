/* test_tree_coverage.c - Comprehensive test for GCC tree node coverage */

/* Prevent optimizations from removing our test constructs */
#define NO_OPTIMIZE __attribute__((noinline,noipa))

/* Generate many unique identifiers */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

/* Vector types for TREE_VEC */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Struct for CONSTRUCTOR */
struct ComplexStruct {
    int a, b, c;
    float f;
    double d;
};

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_trigger = 1;

/* Function to create many IDENTIFIER_NODEs */
NO_OPTIMIZE
void test_identifiers_and_blocks(void) {
    /* Generate many identifiers */
    int MAKE_ID(0) = 1;
    int MAKE_ID(1) = MAKE_ID(0) + 1;
    int MAKE_ID(2) = MAKE_ID(1) * 2;
    int MAKE_ID(3) = MAKE_ID(2) - MAKE_ID(1);
    int MAKE_ID(4) = MAKE_ID(3) / 2;
    int MAKE_ID(5) = MAKE_ID(4) % 3;
    int MAKE_ID(6) = MAKE_ID(5) << 1;
    int MAKE_ID(7) = MAKE_ID(6) >> 1;
    int MAKE_ID(8) = MAKE_ID(7) | 0xFF;
    int MAKE_ID(9) = MAKE_ID(8) & 0x0F;
    
    /* Nested blocks for BLOCK nodes */
    {
        int block_local_1 = MAKE_ID(9);
        {
            int block_local_2 = block_local_1 * 2;
            {
                int block_local_3 = block_local_2 + 3;
                global_counter += block_local_3;
            }
        }
    }
    
    if (global_trigger) {
        int if_block_var = 42;
        global_counter += if_block_var;
    } else {
        int else_block_var = 24;
        global_counter -= else_block_var;
    }
    
    for (int i = 0; i < 3; i++) {
        int for_block_var = i * 10;
        {
            int nested_in_for = for_block_var + 5;
            global_counter += nested_in_for;
        }
    }
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
}

/* Function for TREE_VEC and SSA_NAME */
NO_OPTIMIZE
void test_vectors_and_ssa(void) {
    /* Vector operations for TREE_VEC */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    v4si vec5 = vec3 - vec4;
    
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fvec3 = fvec1 * fvec2;
    
    /* Complex loop with SSA variables */
    int ssa_var = 0;
    int result = 0;
    
    for (int i = 0; i < 100; i++) {
        /* This creates phi nodes in SSA form */
        if (i % 3 == 0) {
            ssa_var = i * 2;
        } else if (i % 3 == 1) {
            ssa_var = i + 10;
        } else {
            ssa_var = i - 5;
        }
        
        /* Another SSA variable with multiple definitions */
        int temp;
        if (ssa_var > 50) {
            temp = ssa_var / 2;
        } else {
            temp = ssa_var * 2;
        }
        
        result += temp;
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(result) : : "memory");
    }
    
    /* Use vectors to affect global state */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += vec5[i];
        sum += (int)fvec3[i];
    }
    
    global_counter += result + sum;
}

/* Function for CONSTRUCTOR nodes */
NO_OPTIMIZE
int compute_value(int x) {
    return x * 2 + 1;
}

NO_OPTIMIZE
float compute_float(int x) {
    return (float)x / 3.0f;
}

NO_OPTIMIZE
void test_aggregate_init(void) {
    /* Non-constant initializers for CONSTRUCTOR nodes */
    struct ComplexStruct s1 = {
        .a = compute_value(1),
        .b = compute_value(2),
        .c = global_counter,
        .f = compute_float(3),
        .d = 3.14159 * global_trigger
    };
    
    /* Array with non-constant initializers */
    int arr[5] = {
        compute_value(10),
        compute_value(20),
        global_counter + 1,
        global_counter * 2,
        s1.a + s1.b
    };
    
    /* Nested struct initialization */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    };
    
    struct Nested n1 = {
        .inner = {
            .a = compute_value(5),
            .b = arr[0],
            .c = arr[1],
            .f = 2.71828f,
            .d = 1.41421
        },
        .extra = 999
    };
    
    /* Use the aggregates to affect global state */
    int sum = 0;
    sum += s1.a + s1.b + s1.c;
    sum += (int)s1.f;
    sum += (int)s1.d;
    
    for (int i = 0; i < 5; i++) {
        sum += arr[i];
    }
    
    sum += n1.inner.a + n1.inner.b + n1.extra;
    
    global_counter += sum;
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
}

/* OpenMP test for OMP_CLAUSE nodes */
#ifdef _OPENMP
#include <omp.h>

NO_OPTIMIZE
void test_omp_clauses(void) {
    int i;
    const int N = 1000;
    int data[N];
    int sum = 0;
    int private_var = 0;
    
    /* Initialize array */
    for (i = 0; i < N; i++) {
        data[i] = i + 1;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(i) shared(data) reduction(+:sum) \
        schedule(dynamic, 16) num_threads(4) if(N > 100) \
        private(private_var) firstprivate(global_counter)
    for (i = 0; i < N; i++) {
        private_var = omp_get_thread_num();
        sum += data[i] + private_var;
        
        /* Nested block inside OpenMP region */
        {
            int local_in_omp = data[i] % 10;
            sum += local_in_omp;
        }
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel sections private(i) \
        reduction(*:global_counter) nowait
    {
        #pragma omp section
        {
            int section_sum = 0;
            for (i = 0; i < 100; i++) {
                section_sum += i;
            }
            global_counter += section_sum;
        }
        
        #pragma omp section
        {
            int section_prod = 1;
            for (i = 1; i < 10; i++) {
                section_prod *= i;
            }
            global_counter *= section_prod;
        }
    }
    
    /* OpenMP task with clauses */
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (i = 0; i < 10; i++) {
                #pragma omp task firstprivate(i) depend(out: data[0]) \
                    priority(omp_get_thread_num())
                {
                    data[i] *= 2;
                }
            }
        }
    }
    
    global_counter += sum;
}
#endif

/* C++ test for TREE_BINFO (compile with g++) */
#ifdef __cplusplus
class BaseClass {
public:
    virtual void virtual_method() {
        global_counter += 1;
    }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual void virtual_method() override {
        global_counter += 2;
    }
    int derived_data;
};

NO_OPTIMIZE
void test_cpp_binfo(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Virtual call through base pointer */
    base_ptr->virtual_method();
    
    /* Access through reference */
    BaseClass& base_ref = derived;
    base_ref.virtual_method();
    
    /* Multiple inheritance-like access */
    DerivedClass* derived_ptr = static_cast<DerivedClass*>(base_ptr);
    derived_ptr->virtual_method();
    
    /* Array of base pointers */
    BaseClass* ptr_array[3];
    ptr_array[0] = &derived;
    ptr_array[1] = new DerivedClass();
    ptr_array[2] = nullptr;
    
    for (int i = 0; i < 2; i++) {
        if (ptr_array[i]) {
            ptr_array[i]->virtual_method();
        }
    }
    
    delete ptr_array[1];
}
#else
/* C version that might generate BINFO with LTO */
NO_OPTIMIZE
void test_lto_binfo(void) {
    /* Complex structure that might generate type hierarchy in LTO */
    struct TypeInfo {
        const char* name;
        int size;
        struct TypeInfo* base;
    };
    
    struct TypeInfo base_type = {"Base", 4, NULL};
    struct TypeInfo derived_type = {"Derived", 8, &base_type};
    
    /* Use the type hierarchy */
    global_counter += derived_type.size;
    if (derived_type.base) {
        global_counter += derived_type.base->size;
    }
}
#endif

/* Main function that orchestrates everything */
int main(void) {
    /* Test all tree node types */
    test_identifiers_and_blocks();
    test_vectors_and_ssa();
    test_aggregate_init();
    
#ifdef _OPENMP
    test_omp_clauses();
#endif

#ifdef __cplusplus
    test_cpp_binfo();
#else
    test_lto_binfo();
#endif
    
    /* Final computation to ensure all code is used */
    int final_result = global_counter;
    
    /* Use final result to prevent dead code elimination */
    asm volatile("" : "+r"(final_result) : : "memory");
    
    /* Return checksum */
    return final_result % 256;
}
