/* test_tree_coverage.c - Comprehensive test for GCC tree node coverage */

/* Force generation of IDENTIFIER_NODE through macros and many identifiers */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

/* Generate many unique identifiers */
volatile int MAKE_ID(0), MAKE_ID(1), MAKE_ID(2), MAKE_ID(3), MAKE_ID(4);
volatile int MAKE_ID(5), MAKE_ID(6), MAKE_ID(7), MAKE_ID(8), MAKE_ID(9);

/* For TREE_VEC - use GCC vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* For CONSTRUCTOR - aggregate types with initializers */
struct ComplexStruct {
    int x;
    float y;
    double z;
    char c;
};

union MixedUnion {
    int i;
    float f;
    void *p;
};

/* Non-inline functions to prevent optimization */
static int get_value(void) {
    volatile int v = 42;
    asm volatile("" : : : "memory");
    return v;
}

static float get_float(void) {
    volatile float f = 3.14f;
    asm volatile("" : : : "memory");
    return f;
}

/* Function to test IDENTIFIER_NODE and BLOCK nodes */
static void test_identifiers_and_blocks(int iterations) {
    /* Outer block scope */
    {
        volatile int outer_var = 0;
        
        for (int i = 0; i < iterations; i++) {
            /* Inner block scope 1 - creates BLOCK nodes */
            {
                volatile int inner_var_1 = i * 2;
                volatile int inner_var_2 = i * 3;
                
                /* Use all the macro-generated identifiers */
                outer_var += MAKE_ID(0) + MAKE_ID(1) + MAKE_ID(2);
                asm volatile("" : : "r"(inner_var_1), "r"(inner_var_2) : "memory");
            }
            
            /* Inner block scope 2 */
            {
                volatile float float_var = i * 1.5f;
                volatile double double_var = i * 2.5;
                
                outer_var += MAKE_ID(3) + MAKE_ID(4) + MAKE_ID(5);
                asm volatile("" : : "r"(float_var), "r"(double_var) : "memory");
            }
            
            /* Inner block scope 3 with switch */
            {
                volatile int switch_var = i % 3;
                switch (switch_var) {
                    case 0:
                        outer_var += MAKE_ID(6);
                        break;
                    case 1:
                        outer_var += MAKE_ID(7);
                        break;
                    case 2:
                        outer_var += MAKE_ID(8);
                        break;
                }
            }
        }
        
        /* Use the last identifier */
        outer_var += MAKE_ID(9);
        asm volatile("" : : "r"(outer_var) : "memory");
    }
}

/* Function to test TREE_VEC and SSA_NAME nodes */
static void test_vectors_and_ssa(int n) {
    /* Vector operations - generate TREE_VEC nodes */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {0, 0, 0, 0};
    
    /* Multiple vector operations */
    vec_c = vec_a + vec_b;
    vec_c = vec_c * vec_a;
    vec_c = vec_c - vec_b;
    
    /* SSA_NAME generation through complex control flow */
    volatile int ssa_var = 0;
    volatile int temp;
    
    for (int i = 0; i < n; i++) {
        /* Complex conditional assignments create SSA nodes */
        if (i % 2 == 0) {
            temp = i * 2;
            ssa_var += temp;
        } else if (i % 3 == 0) {
            temp = i * 3;
            ssa_var -= temp;
        } else {
            temp = i;
            ssa_var *= (temp + 1);
        }
        
        /* Nested conditionals for more SSA complexity */
        for (int j = 0; j < 3; j++) {
            volatile int inner_ssa;
            if (j == 0) {
                inner_ssa = ssa_var + j;
            } else if (j == 1) {
                inner_ssa = ssa_var - j;
            } else {
                inner_ssa = ssa_var * j;
            }
            asm volatile("" : : "r"(inner_ssa) : "memory");
        }
    }
    
    /* Use vectors to prevent dead code elimination */
    v4si vec_result = vec_c + (v4si){ssa_var, ssa_var, ssa_var, ssa_var};
    asm volatile("" : : "r"(vec_result) : "memory");
}

/* Function to test CONSTRUCTOR nodes */
static void test_aggregate_init(void) {
    /* Struct with non-constant initializers - creates CONSTRUCTOR nodes */
    struct ComplexStruct s1 = {
        .x = get_value(),
        .y = get_float(),
        .z = get_value() * 1.5,
        .c = 'A' + (get_value() % 26)
    };
    
    /* Array with non-constant initializers */
    int dynamic_array[4] = {
        get_value(),
        get_value() + 1,
        get_value() * 2,
        get_value() / 2
    };
    
    /* Nested struct initialization */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    };
    
    struct Nested nested = {
        .inner = {
            .x = get_value(),
            .y = get_float(),
            .z = 99.9,
            .c = 'Z'
        },
        .extra = 1000
    };
    
    /* Union with designated initializer */
    union MixedUnion u = {
        .f = get_float() * 2.0f
    };
    
    /* Complex array of structs */
    struct ComplexStruct struct_array[3] = {
        {get_value(), get_float(), 1.1, 'X'},
        {get_value() + 10, get_float() + 1.0f, 2.2, 'Y'},
        {get_value() * 2, get_float() * 2.0f, 3.3, 'Z'}
    };
    
    /* Prevent optimization */
    asm volatile("" : : "r"(s1), "r"(dynamic_array), "r"(nested), "r"(u), "r"(struct_array) : "memory");
}

/* Function to test OMP_CLAUSE nodes */
#ifdef _OPENMP
static void test_omp_clauses(int size) {
    volatile int shared_data = 0;
    int private_data;
    float reduction_sum = 0.0f;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel private(private_data) shared(shared_data, arr) \
                         reduction(+:reduction_sum) num_threads(4) \
                         default(none) if(size > 10)
    {
        private_data = omp_get_thread_num();
        
        #pragma omp for schedule(dynamic, 4) nowait
        for (int i = 0; i < size; i++) {
            /* Nested block for BLOCK nodes */
            {
                volatile int local = arr[i] * private_data;
                reduction_sum += local;
                shared_data += private_data;
            }
        }
        
        /* OpenMP barrier */
        #pragma omp barrier
        
        /* OpenMP single construct */
        #pragma omp single copyprivate(private_data)
        {
            private_data = 999;
        }
        
        /* OpenMP sections with different clauses */
        #pragma omp sections private(private_data) lastprivate(shared_data)
        {
            #pragma omp section
            {
                private_data = 1;
                shared_data = 100;
            }
            
            #pragma omp section
            {
                private_data = 2;
                shared_data = 200;
            }
        }
    }
    
    /* Another OpenMP construct - parallel for with collapse */
    #pragma omp parallel for collapse(2) ordered
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            #pragma omp ordered
            {
                arr[i * 10 + j] = i + j;
            }
        }
    }
    
    /* OpenMP task with depend clause */
    #pragma omp parallel
    #pragma omp single
    {
        int task_result1, task_result2;
        
        #pragma omp task depend(out: task_result1)
        {
            task_result1 = get_value();
        }
        
        #pragma omp task depend(in: task_result1) depend(out: task_result2)
        {
            task_result2 = task_result1 * 2;
        }
        
        #pragma omp task depend(in: task_result2)
        {
            shared_data = task_result2;
        }
    }
    
    asm volatile("" : : "r"(shared_data), "r"(reduction_sum), "r"(arr) : "memory");
}
#endif

/* C++ specific code for TREE_BINFO nodes */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual void method1() { volatile int x = 1; asm volatile("" : : "r"(x) : "memory"); }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual void method1() override { volatile int y = 2; asm volatile("" : : "r"(y) : "memory"); }
    int derived_data;
};

class MultipleBase1 {
public:
    virtual void mb1() {}
    int mb1_data;
};

class MultipleBase2 {
public:
    virtual void mb2() {}
    int mb2_data;
};

class MultipleDerived : public MultipleBase1, public MultipleBase2 {
public:
    virtual void mb1() override {}
    virtual void mb2() override {}
    int md_data;
};

static void test_cpp_binfo(void) {
    BaseClass* base_ptr;
    DerivedClass derived_obj;
    MultipleDerived multi_obj;
    
    /* Use polymorphism to trigger BINFO nodes */
    base_ptr = &derived_obj;
    base_ptr->method1();
    
    /* Multiple inheritance usage */
    MultipleBase1* mb1_ptr = &multi_obj;
    MultipleBase2* mb2_ptr = &multi_obj;
    
    mb1_ptr->mb1();
    mb2_ptr->mb2();
    
    /* Virtual base access */
    volatile int offset1 = (char*)mb1_ptr - (char*)&multi_obj;
    volatile int offset2 = (char*)mb2_ptr - (char*)&multi_obj;
    
    asm volatile("" : : "r"(offset1), "r"(offset2) : "memory");
}
#endif

/* Main function that orchestrates all tests */
int main(int argc, char *argv[]) {
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = 10;
    }
    
    /* Test all tree node types */
    test_identifiers_and_blocks(iterations);
    test_vectors_and_ssa(iterations);
    test_aggregate_init();
    
#ifdef _OPENMP
    test_omp_clauses(iterations);
#endif
    
#ifdef __cplusplus
    test_cpp_binfo();
#endif
    
    /* Create a checksum to ensure all code runs */
    volatile int checksum = 0;
    for (int i = 0; i < iterations; i++) {
        checksum += i;
        /* Nested block inside loop */
        {
            volatile int inner = checksum * 2;
            checksum = inner / 2;
        }
    }
    
    /* Final aggregate initialization with computed values */
    struct FinalStruct {
        int a, b, c;
    } final = {checksum, checksum * 2, checksum * 3};
    
    asm volatile("" : : "r"(checksum), "r"(final) : "memory");
    
    return checksum > 0 ? 0 : 1;
}
