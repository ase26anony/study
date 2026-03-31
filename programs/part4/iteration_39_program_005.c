/* test_tree_coverage.c - Comprehensive test for GCC tree node coverage */

/* Prevent inlining to ensure tree nodes are fully built */
#define NOINLINE __attribute__((noinline))

/* Generate many unique identifiers for IDENTIFIER_NODE */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

/* Vector types for TREE_VEC */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Struct for CONSTRUCTOR */
struct ComplexStruct {
    int a;
    float b;
    double c;
    char d;
};

/* For TREE_BINFO (C++ only) */
#ifdef __cplusplus
class BaseClass {
public:
    virtual void method() {}
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
public:
    virtual void method() override {}
};
#endif

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile float global_float = 1.0f;

/* Function to create many IDENTIFIER_NODEs and BLOCKs */
NOINLINE void test_identifiers_and_blocks(int iterations) {
    /* Outer block */
    {
        int MAKE_ID(0) = 1;
        volatile int MAKE_ID(1) = MAKE_ID(0) + 1;
        
        /* Inner block 1 */
        {
            float MAKE_ID(2) = 3.14f;
            double MAKE_ID(3) = MAKE_ID(2) * 2.0;
            
            /* Deeper block */
            {
                char MAKE_ID(4) = 'A';
                short MAKE_ID(5) = MAKE_ID(4) + 1;
                
                /* Prevent optimization */
                asm volatile("" : : : "memory");
            }
        }
        
        /* Inner block 2 */
        {
            long MAKE_ID(6) = 100L;
            long long MAKE_ID(7) = MAKE_ID(6) * 2LL;
            
            for (int MAKE_ID(8) = 0; MAKE_ID(8) < 3; MAKE_ID(8)++) {
                unsigned MAKE_ID(9) = MAKE_ID(8) * 10;
                /* Another nested block in loop */
                {
                    static int MAKE_ID(10) = 0;
                    MAKE_ID(10)++;
                }
            }
        }
    }
    
    /* More identifiers with different scopes */
    if (iterations > 0) {
        auto int MAKE_ID(11) = iterations;  /* auto creates another identifier */
        register int MAKE_ID(12) = MAKE_ID(11) * 2;
        
        /* Block in if statement */
        {
            extern int MAKE_ID(13);  /* external declaration */
            (void)MAKE_ID(13);
        }
    }
}

/* Function for TREE_VEC and SSA_NAME */
NOINLINE v4si test_vectors_and_ssa(v4si a, v4si b) {
    v4si result = a + b;
    v4si temp = result * a;
    
    /* Create SSA_NAMEs through control flow */
    int ssa_var1 = 0;
    int ssa_var2 = 0;
    
    /* Loop with multiple assignments to create SSA form */
    for (int i = 0; i < 100; i++) {
        volatile int cond = global_counter + i;
        
        if (cond % 3 == 0) {
            ssa_var1 = i * 2;
            ssa_var2 = ssa_var1 + 1;
        } else if (cond % 3 == 1) {
            ssa_var1 = i * 3;
            ssa_var2 = ssa_var1 - 1;
        } else {
            ssa_var1 = i * 4;
            ssa_var2 = ssa_var1 * 2;
        }
        
        /* Use both variables to prevent elimination */
        result[i % 4] += ssa_var1 + ssa_var2;
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
    
    /* More vector operations */
    v4si mask = {1, 0, 1, 0};
    v4si masked_result = result & mask;
    
    /* Mixed-type vector operations */
    v4sf float_vec = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf float_result = float_vec * 2.0f;
    
    /* Prevent optimization */
    global_counter += masked_result[0] + (int)float_result[0];
    
    return result;
}

/* Function for CONSTRUCTOR nodes */
NOINLINE struct ComplexStruct test_aggregate_init(int x, float y) {
    /* Non-constant initializers create CONSTRUCTOR nodes */
    struct ComplexStruct s1 = {
        .a = x + global_counter,
        .b = y * global_float,
        .c = (double)x / (y + 1.0f),
        .d = (char)(x % 26 + 'A')
    };
    
    /* Array with non-constant initializers */
    int arr[4] = {
        x,
        x * 2,
        x + global_counter,
        test_identifiers_and_blocks(1),  /* function call in initializer */
    };
    
    /* Nested struct initializer */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    };
    
    struct Nested n = {
        .inner = s1,
        .extra = arr[0] + arr[1]
    };
    
    /* Designated initializers with complex expressions */
    struct ComplexStruct s2 = {
        a: x * y,
        b: (float)global_counter / 10.0f,
        c: 3.14159 * (double)x,
        d: 'Z' - (x % 26)
    };
    
    /* Use all constructs to prevent elimination */
    s1.a += n.extra;
    s1.b += s2.b;
    
    return s1;
}

/* Function for OMP_CLAUSE nodes */
NOINLINE int test_omp_clauses(int n) {
    int sum = 0;
    int arr[1000];
    
    /* Initialize array */
    for (int i = 0; i < 1000; i++) {
        arr[i] = i + global_counter;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(n) shared(arr, global_counter) \
        reduction(+:sum) schedule(dynamic, 10) num_threads(4) \
        if(n > 1000) default(none)
    for (int i = 0; i < 1000; i++) {
        sum += arr[i];
        /* Memory barrier in parallel region */
        asm volatile("" : : : "memory");
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel sections private(sum) \
        firstprivate(arr) nowait
    {
        #pragma omp section
        {
            int local_sum = 0;
            for (int i = 0; i < 500; i++) {
                local_sum += arr[i];
            }
            #pragma omp atomic
            global_counter += local_sum;
        }
        
        #pragma omp section
        {
            float local_float = 0.0f;
            for (int i = 500; i < 1000; i++) {
                local_float += arr[i] * 0.5f;
            }
            #pragma omp atomic
            global_float += local_float;
        }
    }
    
    /* OpenMP task with clauses */
    #pragma omp task depend(inout: sum) final(n > 100) \
        priority(10) untied mergeable
    {
        sum *= 2;
    }
    
    #pragma omp taskwait
    
    return sum;
}

#ifdef __cplusplus
/* C++ specific function for TREE_BINFO */
NOINLINE void test_binfo_nodes() {
    BaseClass* base_ptr = new DerivedClass();
    DerivedClass derived_obj;
    BaseClass& base_ref = derived_obj;
    
    /* Virtual calls to use vtable (involves BINFO) */
    base_ptr->method();
    base_ref.method();
    
    /* Dynamic cast (uses BINFO) */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    
    /* Type identification */
    const std::type_info& info = typeid(*base_ptr);
    
    delete base_ptr;
    
    /* Multiple inheritance would create more BINFO nodes */
    class AnotherBase {
    public:
        virtual void another_method() {}
    };
    
    class MultipleDerived : public BaseClass, public AnotherBase {
    public:
        virtual void method() override {}
        virtual void another_method() override {}
    };
    
    MultipleDerived md;
    BaseClass* bp = &md;
    AnotherBase* ap = &md;
    
    /* Cross-casts use BINFO */
    AnotherBase* cross_cast = dynamic_cast<AnotherBase*>(bp);
    
    /* Prevent optimization */
    global_counter += (derived_ptr != nullptr);
}
#endif

/* Main function that ties everything together */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Test 1: Identifiers and Blocks */
    test_identifiers_and_blocks(argc);
    
    /* Test 2: Vectors and SSA */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_result = test_vectors_and_ssa(vec_a, vec_b);
    result += vec_result[0] + vec_result[1];
    
    /* Test 3: Aggregate initialization */
    struct ComplexStruct cs = test_aggregate_init(argc, 2.5f);
    result += cs.a + (int)cs.b;
    
    /* Test 4: OpenMP clauses */
    #ifdef _OPENMP
    int omp_result = test_omp_clauses(argc * 100);
    result += omp_result;
    #endif
    
    /* Test 5: C++ BINFO nodes */
    #ifdef __cplusplus
    test_binfo_nodes();
    #endif
    
    /* Additional complex expression with many tree nodes */
    {
        /* Block with local variables */
        int local_var = result;
        
        /* Vector operation inside block */
        v4si temp_vec = vec_result + local_var;
        
        /* Conditional with complex expression */
        result = (local_var > 100) ? 
                 test_identifiers_and_blocks(2), local_var * 2 :
                 test_identifiers_and_blocks(1), local_var / 2;
                 
        /* Loop with SSA variable */
        for (int i = 0; i < 10; i++) {
            static int counter = 0;
            volatile int mod = i % 3;
            
            switch (mod) {
                case 0:
                    counter += i * 2;
                    break;
                case 1:
                    counter += i * 3;
                    break;
                case 2:
                    counter += i * 4;
                    break;
            }
            
            /* Nested block in switch case */
            {
                int nested_var = counter;
                result += nested_var;
            }
        }
    }
    
    /* Final result to prevent dead code elimination */
    volatile int final_result = result;
    asm volatile("" : : "r"(final_result) : "memory");
    
    return final_result % 256;
}

/* External identifier declaration */
int identifier_13 = 42;
