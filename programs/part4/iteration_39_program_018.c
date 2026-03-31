/* test_tree_coverage.c - Comprehensive test to trigger specific tree node classifications */

/* Prevent inlining to ensure tree nodes are preserved */
#define NOINLINE __attribute__((noinline))

/* Generate many identifiers for IDENTIFIER_NODE coverage */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(var_, n)
#define MAKE_FUNC(n) CONCAT(func_, n)

/* Vector types for TREE_VEC coverage */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Struct for CONSTRUCTOR coverage */
struct ComplexStruct {
    int a, b, c;
    float f;
    double d;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

/* Function declarations to prevent optimization */
NOINLINE int external_func(int x);
NOINLINE float external_float_func(float x);
NOINLINE void use_pointer(void *p);
NOINLINE void use_int(int x);
NOINLINE void use_float(float x);
NOINLINE void use_double(double x);

/* ========== IDENTIFIER_NODE and BLOCK coverage ========== */
NOINLINE int test_identifiers_and_blocks(int seed) {
    /* Generate many unique identifiers */
    int MAKE_ID(0) = seed;
    int MAKE_ID(1) = seed * 2;
    int MAKE_ID(2) = seed * 3;
    int MAKE_ID(3) = seed * 4;
    int MAKE_ID(4) = seed * 5;
    int MAKE_ID(5) = seed * 6;
    int MAKE_ID(6) = seed * 7;
    int MAKE_ID(7) = seed * 8;
    int MAKE_ID(8) = seed * 9;
    int MAKE_ID(9) = seed * 10;
    
    volatile int result = 0;
    
    /* Nested blocks for BLOCK coverage */
    {
        int block_local_1 = MAKE_ID(0) + MAKE_ID(1);
        {
            int block_local_2 = block_local_1 * 2;
            {
                int block_local_3 = block_local_2 / 3;
                result += block_local_3;
                
                /* Another deeply nested block */
                {
                    volatile int inner_block = 42;
                    result += inner_block;
                }
            }
        }
    }
    
    /* Block in loop */
    for (int i = 0; i < 3; i++) {
        int loop_block_var = i * MAKE_ID(i % 10);
        {
            int inner_loop_var = loop_block_var + 1;
            result += inner_loop_var;
        }
    }
    
    /* Block with if statement */
    if (result > 100) {
        int if_block_var = result - 50;
        result = if_block_var;
    } else {
        int else_block_var = result + 50;
        result = else_block_var;
    }
    
    return result;
}

/* ========== SSA_NAME coverage ========== */
NOINLINE int test_ssa_name(int iterations) {
    int x = 0, y = 0, z = 0;
    volatile int trigger = 1;
    
    /* Complex loop with multiple branches for SSA */
    for (int i = 0; i < iterations; i++) {
        int temp;
        
        /* Multiple conditional assignments */
        if (trigger > 0) {
            temp = x + y;
            x = temp * 2;
        } else {
            temp = y - x;
            x = temp / 2;
        }
        
        /* Another branch */
        if (i % 3 == 0) {
            y = z + i;
        } else if (i % 3 == 1) {
            y = z * i;
        } else {
            y = z - i;
        }
        
        /* Phi node candidate */
        z = (z > 100) ? z - 50 : z + 50;
        
        /* Memory clobber to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    return x + y + z;
}

/* ========== TREE_VEC coverage ========== */
NOINLINE v4si test_tree_vec(v4si a, v4si b) {
    v4si result;
    
    /* Various vector operations */
    result = a + b;
    result = result * a;
    result = result - b;
    
    /* Vector comparisons and selects */
    v4si mask = a > b;
    result = (mask & a) | (~mask & b);
    
    /* Vector with different type */
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fresult = fvec1 * fvec2;
    
    /* Use result to prevent optimization */
    use_pointer(&result);
    use_pointer(&fresult);
    
    return result;
}

/* ========== CONSTRUCTOR coverage ========== */
NOINLINE struct NestedStruct test_constructor(int a_val, float f_val) {
    /* Non-constant initializers for CONSTRUCTOR nodes */
    struct NestedStruct ns = {
        .inner = {
            .a = external_func(a_val),
            .b = a_val * 2,
            .c = a_val + external_func(a_val - 1),
            .f = external_float_func(f_val),
            .d = (double)f_val * 3.14
        },
        .extra = a_val % 7
    };
    
    /* Array with non-constant initializer */
    int dynamic_array[4] = {
        external_func(1),
        external_func(2),
        external_func(3),
        external_func(4)
    };
    
    /* Struct array */
    struct ComplexStruct struct_array[2] = {
        { 
            .a = external_func(10),
            .b = 20,
            .c = 30,
            .f = 40.0f,
            .d = 50.0
        },
        {
            .a = external_func(60),
            .b = 70,
            .c = 80,
            .f = 90.0f,
            .d = 100.0
        }
    };
    
    /* Use values to prevent optimization */
    for (int i = 0; i < 4; i++) {
        ns.extra += dynamic_array[i];
    }
    
    ns.extra += struct_array[0].a + struct_array[1].c;
    
    return ns;
}

/* ========== OMP_CLAUSE coverage ========== */
#ifdef _OPENMP
NOINLINE int test_omp_clauses(int size) {
    int sum = 0;
    int array[1000];
    
    /* Initialize array */
    for (int i = 0; i < 1000; i++) {
        array[i] = i;
    }
    
    /* OpenMP with multiple clauses for OMP_CLAUSE coverage */
    #pragma omp parallel for private(size) shared(array) reduction(+:sum) \
        schedule(dynamic, 4) num_threads(4) if(size > 1000)
    for (int i = 0; i < 1000; i++) {
        sum += array[i];
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel sections private(sum) firstprivate(array) \
        copyin(array[0:100]) nowait
    {
        #pragma omp section
        {
            int section_sum = 0;
            for (int i = 0; i < 500; i++) {
                section_sum += array[i];
            }
            #pragma omp atomic
            sum += section_sum;
        }
        
        #pragma omp section
        {
            int section_sum = 0;
            for (int i = 500; i < 1000; i++) {
                section_sum += array[i];
            }
            #pragma omp atomic
            sum += section_sum;
        }
    }
    
    /* OpenMP task with clauses */
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < 10; i++) {
                #pragma omp task depend(in: array) depend(out: sum) \
                    priority(i) untied mergeable
                {
                    sum += i;
                }
            }
        }
    }
    
    return sum;
}
#endif

/* ========== TREE_BINFO coverage (requires C++ or LTO) ========== */
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

NOINLINE int test_tree_binfo() {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Virtual call through base pointer */
    int result = base_ptr->method();
    
    /* Access through reference */
    BaseClass& base_ref = derived;
    result += base_ref.method();
    
    /* Typeid might generate BINFO nodes */
    result += sizeof(DerivedClass);
    
    return result;
}
#else
/* For C, we rely on LTO to potentially generate BINFO-like nodes */
NOINLINE int test_tree_binfo(void) {
    /* Complex structure that might trigger BINFO in LTO */
    struct TypeInfo {
        const char* name;
        int size;
        int align;
        struct TypeInfo* base;
    };
    
    struct TypeInfo base_type = {"Base", 4, 4, NULL};
    struct TypeInfo derived_type = {"Derived", 8, 4, &base_type};
    
    volatile int result = 0;
    result += (int)(derived_type.base != NULL);
    
    return result;
}
#endif

/* ========== Main function ========== */
int main(int argc, char** argv) {
    int result = 0;
    volatile int seed = (argc > 1) ? argv[1][0] : 42;
    
    /* Test all tree node types */
    result += test_identifiers_and_blocks(seed);
    result += test_ssa_name(100);
    
    /* Test vectors */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_result = test_tree_vec(vec_a, vec_b);
    result += vec_result[0] + vec_result[1];
    
    /* Test constructors */
    struct NestedStruct ns = test_constructor(seed, seed * 1.5f);
    result += ns.inner.a + ns.extra;
    
    /* Test OpenMP if available */
    #ifdef _OPENMP
    result += test_omp_clauses(2000);
    #endif
    
    /* Test BINFO (C++ or LTO) */
    result += test_tree_binfo();
    
    /* External function calls to prevent dead code elimination */
    use_int(result);
    
    /* Final result depends on all computations */
    return result % 256;
}

/* ========== External function definitions ========== */
/* These are defined in the same file to prevent interprocedural optimization */
NOINLINE int external_func(int x) {
    volatile int result = x * 3 + 7;
    asm volatile("" : : : "memory");
    return result;
}

NOINLINE float external_float_func(float x) {
    volatile float result = x * 2.5f + 1.0f;
    asm volatile("" : : : "memory");
    return result;
}

NOINLINE void use_pointer(void *p) {
    volatile int dummy = (p != NULL);
    asm volatile("" : : : "memory");
}

NOINLINE void use_int(int x) {
    volatile int dummy = x;
    asm volatile("" : : : "memory");
}

NOINLINE void use_float(float x) {
    volatile float dummy = x;
    asm volatile("" : : : "memory");
}

NOINLINE void use_double(double x) {
    volatile double dummy = x;
    asm volatile("" : : : "memory");
}
