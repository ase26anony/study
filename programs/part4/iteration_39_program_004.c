/* test_tree_coverage.c - Comprehensive test to trigger specific tree node classifications */

/* Prevent inlining to ensure tree nodes are preserved */
#define NOINLINE __attribute__((noinline))

/* Generate many unique identifiers */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

/* Use volatile to prevent optimization */
volatile int global_volatile = 0;

/* ========== TREE_VEC (Vector Types) ========== */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE static void test_tree_vec(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    /* Use memory barrier to prevent optimization */
    asm volatile("" : : : "memory");
    
    /* Force use of results */
    global_volatile = c[0] + d[0];
}

/* ========== SSA_NAME and BLOCK ========== */
NOINLINE static int test_ssa_and_blocks(int n) {
    int result = 0;
    
    /* Outer block with local variables */
    {
        int block_var_1 = n * 2;
        int block_var_2 = n + 5;
        
        /* Complex loop to generate SSA names */
        for (int i = 0; i < n; i++) {
            /* Inner block */
            {
                int inner_var;
                if (i % 3 == 0) {
                    inner_var = block_var_1 + i;
                } else if (i % 3 == 1) {
                    inner_var = block_var_2 * i;
                } else {
                    inner_var = global_volatile + i;
                }
                
                /* Another conditional to create phi nodes */
                int temp;
                if (inner_var > 10) {
                    temp = inner_var / 2;
                } else {
                    temp = inner_var * 3;
                }
                
                result += temp;
            }
        }
        
        /* Switch with blocks */
        switch (n % 4) {
            case 0: {
                int case_var = 100;
                result += case_var;
                break;
            }
            case 1: {
                int case_var = 200;
                result -= case_var;
                break;
            }
            default: {
                int case_var = 300;
                result *= 2;
                break;
            }
        }
    }
    
    return result;
}

/* ========== CONSTRUCTOR (Aggregate Initialization) ========== */
struct ComplexStruct {
    int a, b, c;
    float x, y, z;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

NOINLINE static struct NestedStruct test_constructor(int x, int y) {
    /* Non-constant initializer with function calls */
    int val = test_ssa_and_blocks(3);
    
    /* Constructor with designated initializers */
    struct ComplexStruct cs = {
        .a = val + x,
        .b = global_volatile,
        .c = y * 2,
        .x = 1.0f,
        .y = 2.0f,
        .z = 3.0f
    };
    
    /* Array with non-constant initializers */
    int arr[4] = { val, x, y, global_volatile };
    
    /* Nested constructor */
    struct NestedStruct ns = {
        .inner = cs,
        .extra = arr[0] + arr[1]
    };
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    return ns;
}

/* ========== OMP_CLAUSE (OpenMP) ========== */
#ifdef _OPENMP
NOINLINE static int test_omp_clauses(int size) {
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP with multiple clauses */
    #pragma omp parallel for private(size) shared(arr) reduction(+:sum) schedule(dynamic) num_threads(2)
    for (int i = 0; i < 100; i++) {
        int local_var = arr[i];
        if (local_var % 2 == 0) {
            sum += local_var * 2;
        } else {
            sum += local_var;
        }
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel sections private(arr) firstprivate(sum)
    {
        #pragma omp section
        {
            int section_sum = 0;
            for (int i = 0; i < 50; i++) {
                section_sum += arr[i];
            }
            global_volatile = section_sum;
        }
        
        #pragma omp section
        {
            int section_sum = 0;
            for (int i = 50; i < 100; i++) {
                section_sum += arr[i] * 2;
            }
            global_volatile += section_sum;
        }
    }
    
    return sum;
}
#endif

/* ========== Many IDENTIFIER_NODEs ========== */
NOINLINE static void test_many_identifiers(void) {
    /* Generate many unique identifiers */
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
    int result = 
        MAKE_ID(1) * MAKE_ID(2) +
        MAKE_ID(3) / MAKE_ID(4) -
        MAKE_ID(5) % MAKE_ID(6) +
        MAKE_ID(7) << MAKE_ID(8) >>
        MAKE_ID(9) & MAKE_ID(10);
    
    /* More identifiers in different scopes */
    {
        long MAKE_ID(11) = result;
        double MAKE_ID(12) = 3.14159;
        float MAKE_ID(13) = MAKE_ID(12) * 2.0f;
        
        global_volatile = (int)MAKE_ID(13);
    }
    
    /* Function-like identifiers */
    void (*MAKE_ID(func_ptr))(void) = test_many_identifiers;
    
    /* Array with identifier name */
    int MAKE_ID(array)[10];
    for (int i = 0; i < 10; i++) {
        MAKE_ID(array)[i] = i * MAKE_ID(1);
    }
}

/* ========== TREE_BINFO (C++ Inheritance) ========== */
#ifdef __cplusplus
class BaseClass {
public:
    virtual void base_method() {
        global_volatile = 1;
    }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual void base_method() override {
        global_volatile = 2;
    }
    int derived_data;
};

NOINLINE static void test_binfo(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Virtual call to generate BINFO */
    base_ptr->base_method();
    
    /* Access through reference */
    BaseClass& base_ref = derived;
    base_ref.base_data = 100;
    
    /* Multiple inheritance-like access */
    derived.derived_data = 200;
    derived.base_data = 300;
}
#else
/* For C compilation, we'll use a different approach with LTO */
struct BaseStruct {
    int data;
    void (*method)(struct BaseStruct*);
};

struct DerivedStruct {
    struct BaseStruct base;
    int extra;
};

NOINLINE static void base_method(struct BaseStruct* b) {
    b->data++;
}

NOINLINE static void test_binfo_c(void) {
    struct DerivedStruct d;
    d.base.method = base_method;
    d.base.data = 42;
    d.extra = 100;
    
    /* Call through function pointer */
    d.base.method(&d.base);
    
    global_volatile = d.base.data + d.extra;
}
#endif

/* ========== Main Function ========== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Test TREE_VEC */
    test_tree_vec();
    
    /* Test SSA_NAME and BLOCK */
    result += test_ssa_and_blocks(argc + 10);
    
    /* Test CONSTRUCTOR */
    struct NestedStruct ns = test_constructor(argc, result);
    result += ns.extra;
    
    /* Test OMP_CLAUSE */
    #ifdef _OPENMP
    result += test_omp_clauses(argc * 10);
    #endif
    
    /* Test IDENTIFIER_NODE */
    test_many_identifiers();
    result += global_volatile;
    
    /* Test TREE_BINFO */
    #ifdef __cplusplus
    test_binfo();
    #else
    test_binfo_c();
    #endif
    result += global_volatile;
    
    /* Final computation to prevent optimization */
    volatile int final_result = result;
    
    return final_result % 256;
}
