/* test_tree_coverage.c - Comprehensive test for GCC tree node classification */

/* Prevent inlining to ensure tree nodes are fully built */
#define NOINLINE __attribute__((noinline))

/* Generate many unique identifiers */
#define CONCAT(a, b) a##b
#define UNIQUE_ID(base) CONCAT(base, __LINE__)

/* For TREE_BINFO - use C++ if compiled as C++ */
#ifdef __cplusplus
class BaseClass {
public:
    virtual void method() {}
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual void method() override {}
    int derived_data;
};
#endif

/* Vector types for TREE_VEC */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile variables to prevent optimization */
volatile int vol_var = 0;
volatile float vol_float = 1.0f;

/* Memory barrier */
static inline void barrier(void) {
    asm volatile("" : : : "memory");
}

/* ========== IDENTIFIER_NODE and BLOCK coverage ========== */
NOINLINE void test_identifiers_and_blocks(int iterations) {
    /* Many unique identifiers */
    int UNIQUE_ID(var_) = 1;
    int UNIQUE_ID(var_) = 2;
    int UNIQUE_ID(var_) = 3;
    int UNIQUE_ID(var_) = 4;
    int UNIQUE_ID(var_) = 5;
    int UNIQUE_ID(var_) = 6;
    int UNIQUE_ID(var_) = 7;
    int UNIQUE_ID(var_) = 8;
    int UNIQUE_ID(var_) = 9;
    int UNIQUE_ID(var_) = 10;
    
    /* Nested blocks create BLOCK nodes */
    {
        int block_local_1 = vol_var;
        barrier();
        {
            int block_local_2 = block_local_1 + 1;
            barrier();
            {
                int block_local_3 = block_local_2 * 2;
                vol_var = block_local_3;
            }
        }
    }
    
    /* More blocks in loops */
    for (int i = 0; i < iterations; i++) {
        /* New block scope */
        {
            int loop_local = i * 2;
            if (loop_local > 10) {
                /* Another nested block */
                int inner_local = loop_local - 5;
                vol_var = inner_local;
            }
        }
    }
    
    /* Switch with blocks */
    switch (vol_var) {
        case 1: {
            int case1_var = 100;
            vol_var = case1_var;
            break;
        }
        case 2: {
            int case2_var = 200;
            vol_var = case2_var;
            break;
        }
        default: {
            int default_var = 300;
            vol_var = default_var;
        }
    }
}

/* ========== SSA_NAME coverage ========== */
NOINLINE int test_ssa_name(int n) {
    int x = 0;
    int y = 1;
    
    /* Complex control flow to generate SSA */
    for (int i = 0; i < n; i++) {
        barrier();
        if (i % 3 == 0) {
            x = y + i;
        } else if (i % 3 == 1) {
            x = y * i;
        } else {
            x = y - i;
        }
        
        /* Phi node candidate */
        y = x + y;
        
        /* Another branch */
        int temp;
        if (y > 100) {
            temp = y / 2;
        } else {
            temp = y * 2;
        }
        
        y = temp + 1;
    }
    
    /* More SSA complexity */
    int result = 0;
    for (int i = 0; i < n; i++) {
        int val;
        if (i % 2 == 0) {
            val = i * 2;
        } else {
            val = i + 2;
        }
        result += val;
    }
    
    return result + x + y;
}

/* ========== TREE_VEC coverage ========== */
NOINLINE v4si test_tree_vec(v4si a, v4si b) {
    /* Various vector operations */
    v4si add_result = a + b;
    v4si mul_result = a * b;
    v4si sub_result = a - b;
    
    /* Vector comparisons */
    v4si cmp_result = a > b;
    
    /* Vector shuffling-like operations */
    v4si blend;
    for (int i = 0; i < 4; i++) {
        if (i % 2 == 0) {
            blend[i] = a[i];
        } else {
            blend[i] = b[i];
        }
    }
    
    /* Mixed vector operations */
    v4si result = add_result + mul_result * sub_result;
    barrier();
    
    return result + blend;
}

/* ========== CONSTRUCTOR coverage ========== */
struct ComplexStruct {
    int a;
    float b;
    double c;
    int d[4];
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

NOINLINE struct ComplexStruct test_constructor(int x, float y) {
    /* Non-constant initializer */
    int dynamic_val = x * 2;
    float dynamic_float = y + 1.0f;
    
    /* Constructor with mixed initializers */
    struct ComplexStruct s1 = {
        .a = dynamic_val,
        .b = dynamic_float,
        .c = (double)dynamic_val / 2.0,
        .d = { dynamic_val, dynamic_val + 1, dynamic_val + 2, dynamic_val + 3 }
    };
    
    /* Array with non-constant initializer */
    int arr[4] = { 
        dynamic_val, 
        dynamic_val * 2,
        test_ssa_name(3),  /* Function call in initializer */
        vol_var            /* Volatile access in initializer */
    };
    
    /* Nested struct constructor */
    struct NestedStruct ns = {
        .inner = {
            .a = x,
            .b = y,
            .c = x * y,
            .d = {1, 2, 3, 4}
        },
        .extra = dynamic_val
    };
    
    barrier();
    s1.d[0] = arr[0] + ns.extra;
    
    return s1;
}

/* ========== OMP_CLAUSE coverage ========== */
#ifdef _OPENMP
NOINLINE int test_omp_clauses(int size) {
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP with multiple clauses */
    #pragma omp parallel for private(size) shared(arr) reduction(+:sum) \
            schedule(dynamic) num_threads(4) if(size > 1000)
    for (int i = 0; i < 100; i++) {
        int local_var = i * 2;  /* private variable */
        sum += arr[i] + local_var;
    }
    
    /* Another OpenMP construct with different clauses */
    int max_val = 0;
    #pragma omp parallel sections private(arr) firstprivate(size) \
            reduction(max:max_val)
    {
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
        }
        
        #pragma omp section
        {
            for (int i = 50; i < 100; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
        }
    }
    
    /* OMP task with clauses */
    int task_result = 0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: task_result) \
                    priority(2) untied mergeable
            {
                task_result = sum + max_val;
            }
        }
    }
    
    return sum + max_val + task_result;
}
#endif

/* ========== TREE_BINFO coverage (C++ only) ========== */
#ifdef __cplusplus
NOINLINE int test_binfo() {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Virtual call - uses BINFO for vtable */
    base_ptr->method();
    
    /* Access through base pointer */
    base_ptr->base_data = 42;
    derived.derived_data = 24;
    
    /* Reference to base */
    BaseClass& base_ref = derived;
    base_ref.base_data = 100;
    
    barrier();
    
    return base_ptr->base_data + derived.derived_data;
}
#endif

/* ========== Main function ========== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Test IDENTIFIER_NODE and BLOCK */
    test_identifiers_and_blocks(argc > 1 ? 10 : 5);
    
    /* Test SSA_NAME */
    result += test_ssa_name(20);
    
    /* Test TREE_VEC */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_result = test_tree_vec(vec_a, vec_b);
    result += vec_result[0] + vec_result[1];
    
    /* Test CONSTRUCTOR */
    struct ComplexStruct cs = test_constructor(10, 2.5f);
    result += cs.a + (int)cs.b;
    
    /* Test OMP_CLAUSE */
    #ifdef _OPENMP
    result += test_omp_clauses(2000);
    #endif
    
    /* Test TREE_BINFO if C++ */
    #ifdef __cplusplus
    result += test_binfo();
    #endif
    
    /* Prevent dead code elimination */
    barrier();
    vol_var = result;
    
    /* Print to ensure execution */
    printf("Result: %d\n", result);
    
    return result > 0 ? 0 : 1;
}
