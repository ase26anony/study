/* test_tree_codes.c - Comprehensive test for GCC tree node classification */

/* Prevent inlining to ensure tree nodes are fully built */
#define NOINLINE __attribute__((noinline))

/* For TREE_VEC - vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* For CONSTRUCTOR - aggregate types */
struct ComplexStruct {
    int a;
    float b;
    double c;
    char d;
};

union MixedUnion {
    int i;
    float f;
    void* p;
};

/* For IDENTIFIER_NODE - generate many unique identifiers */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

/* Volatile variables to prevent optimization */
volatile int vol_int = 0;
volatile float vol_float = 1.0f;

/* Function declarations to prevent dead code elimination */
NOINLINE void use_int(int x);
NOINLINE void use_float(float x);
NOINLINE void use_ptr(void* p);
NOINLINE void use_vector(v4si v);

/* Memory barrier */
static inline void barrier(void) {
    asm volatile("" : : : "memory");
}

/* ========== Test IDENTIFIER_NODE and BLOCK ========== */
NOINLINE int test_identifiers_and_blocks(int seed) {
    int result = seed;
    
    /* Multiple identifiers in outer block */
    int MAKE_ID(0) = seed * 2;
    int MAKE_ID(1) = seed + 1;
    int MAKE_ID(2) = seed - 1;
    
    /* Nested block 1 */
    {
        int MAKE_ID(3) = MAKE_ID(0) + MAKE_ID(1);
        float MAKE_ID(4) = MAKE_ID(3) * 0.5f;
        
        /* Deeply nested block */
        {
            double MAKE_ID(5) = MAKE_ID(4) * 2.0;
            result += (int)MAKE_ID(5);
            barrier();
        }
        
        result += MAKE_ID(3);
    }
    
    /* Nested block 2 */
    {
        long MAKE_ID(6) = result * 3L;
        short MAKE_ID(7) = (short)MAKE_ID(6);
        
        if (vol_int > 0) {
            /* Another block inside if */
            unsigned MAKE_ID(8) = MAKE_ID(7) * 2;
            result += MAKE_ID(8);
        } else {
            /* Else block */
            unsigned MAKE_ID(9) = MAKE_ID(7) / 2;
            result -= MAKE_ID(9);
        }
    }
    
    /* Loop with block */
    for (int MAKE_ID(10) = 0; MAKE_ID(10) < 3; MAKE_ID(10)++) {
        int MAKE_ID(11) = result + MAKE_ID(10);
        result = MAKE_ID(11) ^ 0x55;
        barrier();
    }
    
    return result;
}

/* ========== Test TREE_VEC and SSA_NAME ========== */
NOINLINE v4si test_vectors_and_ssa(int base) {
    /* Vector declarations */
    v4si vec1 = {base, base + 1, base + 2, base + 3};
    v4si vec2 = {1, 2, 3, 4};
    v4si vec3;
    
    /* Vector operations - generates TREE_VEC nodes */
    vec3 = vec1 + vec2;
    v4si vec4 = vec3 * vec2;
    v4si vec5 = vec4 - vec1;
    
    /* SSA_NAME test - complex loop with conditional updates */
    int ssa_var1 = base;
    int ssa_var2 = 0;
    
    for (int i = 0; i < 10; i++) {
        barrier();
        
        /* This creates phi nodes in SSA form */
        if (vol_int > i) {
            ssa_var1 = ssa_var1 * 2 + 1;
            ssa_var2 = ssa_var1 + i;
        } else {
            ssa_var1 = ssa_var1 / 2 - 1;
            ssa_var2 = ssa_var1 - i;
        }
        
        /* Another SSA variable with multiple assignments */
        int ssa_var3;
        if (i % 3 == 0) {
            ssa_var3 = ssa_var1 * 3;
        } else if (i % 3 == 1) {
            ssa_var3 = ssa_var2 * 2;
        } else {
            ssa_var3 = ssa_var1 + ssa_var2;
        }
        
        vec5[i % 4] += ssa_var3;
    }
    
    /* More vector operations */
    v4si vec6 = vec5 << 2;
    v4si vec7 = vec6 >> 1;
    
    return vec7 + vec3;
}

/* ========== Test CONSTRUCTOR ========== */
NOINLINE struct ComplexStruct test_aggregate_init(int val) {
    /* Non-constant initializers for aggregates */
    int get_value(void) { return val * 2; }
    float get_float(void) { return val * 3.14f; }
    
    /* CONSTRUCTOR nodes for struct with non-constant initializers */
    struct ComplexStruct s1 = {
        .a = get_value() + vol_int,
        .b = get_float() * vol_float,
        .c = (double)val / (vol_int + 1),
        .d = (char)(val % 256)
    };
    
    /* Array with non-constant initializer */
    int arr[4] = {
        get_value(),
        val + 1,
        val * 2,
        s1.a
    };
    
    /* Nested struct initialization */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    };
    
    struct Nested n1 = {
        .inner = s1,
        .extra = arr[0] + arr[1]
    };
    
    /* Union constructor */
    union MixedUnion u1 = {
        .f = s1.b * 2.0f
    };
    
    /* Update based on union */
    s1.a += (int)u1.f;
    
    barrier();
    return s1;
}

/* ========== Test OMP_CLAUSE ========== */
#ifdef _OPENMP
NOINLINE int test_omp_clauses(int* data, int n) {
    int sum = 0;
    int i;
    
    /* OpenMP parallel with multiple clauses */
    #pragma omp parallel for private(i) shared(data, n) reduction(+:sum) \
            schedule(dynamic, 4) if(n > 1000) num_threads(2)
    for (i = 0; i < n; i++) {
        sum += data[i] * (i + 1);
    }
    
    /* OpenMP sections with different clauses */
    #pragma omp parallel sections private(i) \
            nowait
    {
        #pragma omp section
        {
            for (i = 0; i < n/2; i++) {
                data[i] = data[i] * 2;
            }
        }
        
        #pragma omp section
        {
            for (i = n/2; i < n; i++) {
                data[i] = data[i] / 2;
            }
        }
    }
    
    /* OpenMP task with clauses */
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (i = 0; i < 5; i++) {
                #pragma omp task firstprivate(i) shared(data, sum) \
                        depend(out: data[i])
                {
                    data[i] = sum + i;
                    barrier();
                }
            }
        }
    }
    
    return sum;
}
#endif

/* ========== C++ specific for TREE_BINFO ========== */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int method1() { return 1; }
    virtual int method2(int x) { return x * 2; }
    int data;
};

class DerivedClass : public BaseClass {
public:
    virtual int method1() override { return data + 1; }
    virtual int method2(int x) override { return x * x + data; }
    int extra_data;
};

class SecondDerived : public DerivedClass {
public:
    virtual int method1() override { return extra_data * 2; }
    int more_data;
};

NOINLINE int test_binfo_nodes(int val) {
    BaseClass* base_ptr;
    DerivedClass derived;
    SecondDerived second;
    
    derived.data = val;
    derived.extra_data = val * 2;
    
    second.data = val + 1;
    second.extra_data = val * 3;
    second.more_data = val * 4;
    
    /* Polymorphic calls to generate BINFO nodes */
    if (vol_int > 0) {
        base_ptr = &derived;
    } else {
        base_ptr = &second;
    }
    
    int result = base_ptr->method1();
    result += base_ptr->method2(val);
    
    /* Casts that require BINFO lookups */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        result += derived_ptr->extra_data;
    }
    
    /* Reference to base */
    BaseClass& base_ref = second;
    result += base_ref.method2(result);
    
    return result;
}
#endif

/* ========== Main function ========== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Test IDENTIFIER_NODE and BLOCK */
    result += test_identifiers_and_blocks(argc);
    
    /* Test TREE_VEC and SSA_NAME */
    v4si vec_result = test_vectors_and_ssa(result);
    for (int i = 0; i < 4; i++) {
        result += vec_result[i];
    }
    
    /* Test CONSTRUCTOR */
    struct ComplexStruct s = test_aggregate_init(result);
    result += s.a + (int)s.b + (int)s.c + s.d;
    
    /* Test OMP_CLAUSE */
    #ifdef _OPENMP
    int data[100];
    for (int i = 0; i < 100; i++) {
        data[i] = i + result;
    }
    result += test_omp_clauses(data, 100);
    #endif
    
    /* Test TREE_BINFO (C++ only) */
    #ifdef __cplusplus
    result += test_binfo_nodes(result);
    #endif
    
    /* Use results to prevent optimization */
    use_int(result);
    
    /* Final barrier */
    barrier();
    
    return result % 256;
}

/* Function definitions to prevent optimization */
NOINLINE void use_int(int x) {
    vol_int = x;
}

NOINLINE void use_float(float x) {
    vol_float = x;
}

NOINLINE void use_ptr(void* p) {
    asm volatile("" : : "r"(p) : "memory");
}

NOINLINE void use_vector(v4si v) {
    vol_int = v[0];
}
