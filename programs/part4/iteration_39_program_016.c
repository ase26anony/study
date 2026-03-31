/* test_tree_coverage.c - Comprehensive test for GCC tree node coverage */

/* Prevent excessive inlining */
#define NOINLINE __attribute__((noinline))

/* Generate many unique identifiers */
#define CONCAT(a, b) a##b
#define UNIQUE_ID(base) CONCAT(base, __LINE__)

/* For TREE_BINFO coverage via LTO */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual void method() = 0;
};

class DerivedClass : public BaseClass {
public:
    void method() override {}
};
#endif

/* Volatile variables to prevent optimization */
volatile int g_volatile = 0;

/* Function to prevent dead code elimination */
NOINLINE void use(void* ptr) {
    asm volatile("" : : "r"(ptr) : "memory");
}

/* ========== IDENTIFIER_NODE and BLOCK coverage ========== */
NOINLINE int test_identifiers_and_blocks(int n) {
    /* Generate many unique identifiers */
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
    
    /* Nested blocks for BLOCK nodes */
    {
        int block_local_1 = n * 2;
        {
            int block_local_2 = block_local_1 + 1;
            {
                int block_local_3 = block_local_2 * 3;
                g_volatile = block_local_3;
            }
        }
    }
    
    if (n > 0) {
        int if_block_var = 100;
        g_volatile = if_block_var;
    } else {
        int else_block_var = 200;
        g_volatile = else_block_var;
    }
    
    for (int i = 0; i < 3; i++) {
        int for_block_var = i * 10;
        {
            int nested_in_for = for_block_var + 5;
            g_volatile = nested_in_for;
        }
    }
    
    return g_volatile;
}

/* ========== TREE_VEC coverage ========== */
/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE v4si test_vector_operations(v4si a, v4si b) {
    v4si result;
    
    /* Various vector operations */
    result = a + b;
    result = result * a;
    result = result - b;
    result = result & a;
    result = result | b;
    
    /* Vector comparisons */
    v4si cmp = a > b;
    result = result + cmp;
    
    /* Mixed vector types */
    v4sf fvec = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fresult = fvec * fvec;
    
    /* Prevent optimization */
    asm volatile("" : : : "memory");
    
    return result;
}

/* ========== SSA_NAME coverage ========== */
NOINLINE int test_ssa_formation(int n) {
    int x, y, z;
    
    /* Complex control flow for SSA */
    if (n > 0) {
        x = n * 2;
        y = x + 1;
    } else {
        x = n / 2;
        y = x - 1;
    }
    
    /* Loop with phi nodes */
    z = 0;
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            x = y + i;
        } else {
            x = y - i;
        }
        z += x;
        
        /* Nested condition */
        if (z > 100) {
            y = z / 2;
        } else {
            y = z * 2;
        }
    }
    
    /* Another SSA opportunity */
    int w = z;
    for (int j = 0; j < 10; j++) {
        w = w + (j % 3 ? x : y);
    }
    
    return w + z;
}

/* ========== CONSTRUCTOR coverage ========== */
struct ComplexStruct {
    int a;
    int b;
    int c;
    int d[4];
};

struct NestedStruct {
    struct ComplexStruct inner;
    float f;
    double d;
};

NOINLINE struct NestedStruct test_aggregate_init(int val) {
    /* Non-constant initializers */
    int dynamic_val = val * 2;
    
    /* Constructor with mixed initializers */
    struct ComplexStruct cs = {
        .a = dynamic_val,
        .b = dynamic_val + 1,
        .c = dynamic_val * 2,
        .d = {dynamic_val, dynamic_val + 1, 
              dynamic_val + 2, dynamic_val + 3}
    };
    
    /* Nested constructor */
    struct NestedStruct ns = {
        .inner = cs,
        .f = dynamic_val * 1.5f,
        .d = dynamic_val * 2.5
    };
    
    /* Array with non-constant initializer */
    int arr[4] = {dynamic_val, dynamic_val + g_volatile, 
                  dynamic_val * 2, dynamic_val / 2};
    
    /* Prevent optimization */
    use(&cs);
    use(&ns);
    use(arr);
    
    return ns;
}

/* ========== OMP_CLAUSE coverage ========== */
#ifdef _OPENMP
NOINLINE int test_omp_clauses(int n) {
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP with multiple clauses */
    #pragma omp parallel for private(n) shared(arr) reduction(+:sum) \
            schedule(dynamic, 4) num_threads(2) if(n > 1000)
    for (int i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel
    {
        int local_sum = 0;
        #pragma omp for nowait
        for (int i = 0; i < 50; i++) {
            local_sum += i;
        }
        #pragma omp atomic
        sum += local_sum;
    }
    
    /* OpenMP sections */
    #pragma omp parallel sections private(n)
    {
        #pragma omp section
        {
            sum += 1;
        }
        #pragma omp section
        {
            sum += 2;
        }
    }
    
    return sum;
}
#endif

/* ========== TREE_BINFO coverage via LTO ========== */
#ifdef __cplusplus
NOINLINE void test_binfo() {
    DerivedClass d;
    BaseClass* b = &d;
    b->method();
    
    /* Multiple inheritance-like pattern */
    struct Base1 { virtual void f1() {} };
    struct Base2 { virtual void f2() {} };
    struct Derived : Base1, Base2 {
        void f1() override {}
        void f2() override {}
    };
    
    Derived dd;
    Base1* b1 = &dd;
    Base2* b2 = &dd;
    b1->f1();
    b2->f2();
}
#endif

/* ========== Main orchestrator ========== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Test all constructs */
    result += test_identifiers_and_blocks(argc);
    
    /* Vector operations */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_result = test_vector_operations(vec_a, vec_b);
    result += vec_result[0];
    
    /* SSA formation */
    result += test_ssa_formation(argc * 10);
    
    /* Aggregate initialization */
    struct NestedStruct ns = test_aggregate_init(argc);
    result += ns.inner.a + ns.inner.b;
    
    /* OpenMP clauses */
    #ifdef _OPENMP
    result += test_omp_clauses(argc * 100);
    #endif
    
    /* BINFO via C++ */
    #ifdef __cplusplus
    test_binfo();
    #endif
    
    /* Final result to prevent optimization */
    asm volatile("" : "+r"(result) : : "memory");
    
    return result % 256;
}
