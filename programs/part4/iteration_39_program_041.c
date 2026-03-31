/* test_tree_coverage.c - Comprehensive test for GCC tree node coverage */

/* Prevent excessive inlining */
#define NOINLINE __attribute__((noinline))

/* Generate many identifiers */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

/* Use volatile to prevent optimization */
volatile int global_trigger = 0;

/* ========== TREE_VEC and SSA_NAME coverage ========== */
NOINLINE void test_vectors_and_ssa(int n) {
    /* Vector types create TREE_VEC nodes */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c;
    
    /* Vector operations */
    c = a + b;
    c = c * a;
    
    /* Complex loop with SSA variables */
    int ssa_var1 = 0;
    int ssa_var2 = 0;
    
    for (int i = 0; i < n; i++) {
        /* This creates SSA_NAME nodes due to phi nodes */
        int temp;
        if (i % 2 == 0) {
            temp = ssa_var1 + i;
            ssa_var1 = temp * 2;
        } else {
            temp = ssa_var2 - i;
            ssa_var2 = temp / 2;
        }
        
        /* Use temp to prevent dead code elimination */
        asm volatile("" : "+r"(temp) : : "memory");
        
        /* Nested block for BLOCK coverage */
        {
            int block_local = temp * 3;
            ssa_var1 += block_local;
        }
    }
    
    /* Use results */
    global_trigger += ssa_var1 + ssa_var2;
}

/* ========== IDENTIFIER_NODE and BLOCK coverage ========== */
NOINLINE void test_identifiers_and_blocks(void) {
    /* Generate many unique identifiers */
    int MAKE_ID(0) = 1;
    int MAKE_ID(1) = 2;
    int MAKE_ID(2) = 3;
    int MAKE_ID(3) = 4;
    int MAKE_ID(4) = 5;
    int MAKE_ID(5) = 6;
    int MAKE_ID(6) = 7;
    int MAKE_ID(7) = 8;
    int MAKE_ID(8) = 9;
    int MAKE_ID(9) = 10;
    
    /* Deeply nested blocks */
    {
        int block_a = MAKE_ID(0);
        {
            int block_b = block_a + MAKE_ID(1);
            {
                int block_c = block_b * MAKE_ID(2);
                {
                    int block_d = block_c - MAKE_ID(3);
                    {
                        int block_e = block_d / MAKE_ID(4);
                        global_trigger += block_e;
                    }
                }
            }
        }
    }
    
    /* Switch with blocks */
    switch (MAKE_ID(5)) {
        case 6: {
            int case_var = 100;
            global_trigger += case_var;
            break;
        }
        default: {
            int default_var = 200;
            global_trigger += default_var;
        }
    }
}

/* ========== CONSTRUCTOR coverage ========== */
struct ComplexStruct {
    int x;
    double y;
    char z[4];
};

NOINLINE int compute_value(int x) {
    return x * 2 + 1;
}

NOINLINE void test_constructors(void) {
    /* Non-constant struct initializer */
    struct ComplexStruct s1 = {
        .x = compute_value(10),
        .y = 3.14159,
        .z = {'a', 'b', 'c', '\0'}
    };
    
    /* Array with non-constant initializers */
    int arr[4] = {
        compute_value(1),
        compute_value(2),
        compute_value(3),
        compute_value(4)
    };
    
    /* Nested struct initializer */
    struct {
        struct ComplexStruct inner;
        int extra;
    } nested = {
        .inner = { compute_value(5), 2.71828, {'d','e','f','\0'} },
        .extra = compute_value(6)
    };
    
    global_trigger += s1.x + arr[0] + nested.extra;
}

/* ========== OMP_CLAUSE coverage ========== */
#ifdef _OPENMP
NOINLINE void test_omp_clauses(int n) {
    int i;
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) \
            schedule(dynamic) num_threads(2) if(n > 1000)
    for (i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel sections private(i) firstprivate(sum)
    {
        #pragma omp section
        {
            int section_var = sum * 2;
            global_trigger += section_var;
        }
        
        #pragma omp section
        {
            int section_var2 = sum / 2;
            global_trigger += section_var2;
        }
    }
    
    /* OpenMP task with clauses */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: arr[0]) priority(1)
            {
                arr[0] = compute_value(arr[0]);
            }
        }
    }
}
#endif

/* ========== TREE_BINFO coverage (C++ version) ========== */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int method() { return 42; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int method() override { return base_data * 2; }
    int derived_data;
};

NOINLINE void test_binfo(void) {
    DerivedClass d;
    BaseClass* b = &d;
    
    /* Virtual call through base pointer - needs BINFO */
    int result = b->method();
    
    /* Access through reference */
    BaseClass& br = d;
    result += br.method();
    
    global_trigger += result;
}
#else
/* C version attempt with LTO structures */
NOINLINE void test_binfo(void) {
    /* In C, we rely on LTO to generate BINFO-like structures */
    struct lto_type {
        int kind;
        struct lto_type* base;
    };
    
    struct lto_type t1 = {1, 0};
    struct lto_type t2 = {2, &t1};
    
    global_trigger += t2.kind;
}
#endif

/* ========== Main orchestrator ========== */
int main(int argc, char** argv) {
    int iterations = 100;
    if (argc > 1) iterations = atoi(argv[1]);
    
    /* Test all constructs */
    test_identifiers_and_blocks();
    test_vectors_and_ssa(iterations);
    test_constructors();
    
#ifdef _OPENMP
    test_omp_clauses(iterations);
#endif
    
    test_binfo();
    
    /* Print result to ensure execution */
    printf("Result: %d\n", global_trigger);
    
    return global_trigger != 0 ? 0 : 1;
}
