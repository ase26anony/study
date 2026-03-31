/* test_tree_coverage.c - Comprehensive test for GCC tree node classification */

/* Prevent excessive optimization */
volatile int global_volatile = 0;

/* Helper to prevent optimization */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* ========== IDENTIFIER_NODE generation ========== */
/* Generate many unique identifiers using macros */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

void test_identifiers(void) {
    /* Generate many distinct identifiers */
    int MAKE_ID(0), MAKE_ID(1), MAKE_ID(2), MAKE_ID(3), MAKE_ID(4);
    int MAKE_ID(5), MAKE_ID(6), MAKE_ID(7), MAKE_ID(8), MAKE_ID(9);
    
    /* Use them in expressions */
    MAKE_ID(0) = 1;
    MAKE_ID(1) = MAKE_ID(0) + 2;
    MAKE_ID(2) = MAKE_ID(1) * MAKE_ID(0);
    
    /* Complex expression with many identifiers */
    int result = MAKE_ID(0) + MAKE_ID(1) + MAKE_ID(2) + MAKE_ID(3) + 
                 MAKE_ID(4) + MAKE_ID(5) + MAKE_ID(6) + MAKE_ID(7) + 
                 MAKE_ID(8) + MAKE_ID(9);
    
    use(&result);
}

/* ========== TREE_VEC generation ========== */
/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

void test_vectors(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Multiple vector operations */
    v4si r1 = a + b;
    v4si r2 = a * b;
    v4si r3 = r1 + c;
    v4si r4 = r2 - c;
    
    /* Mixed vector operations */
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fr = f1 * f2 + f1;
    
    use(&r3);
    use(&r4);
    use(&fr);
}

/* ========== SSA_NAME generation ========== */
int test_ssa(int n) {
    int x = 0, y = 0, z = 0;
    
    /* Loop with multiple assignments to create SSA */
    for (int i = 0; i < n; i++) {
        /* Conditional creates phi nodes */
        if (i % 2 == 0) {
            x = i * 2;
            y = x + 1;
        } else {
            x = i * 3;
            y = x - 1;
        }
        
        /* Another level of SSA complexity */
        if (i % 3 == 0) {
            z = x + y;
        } else if (i % 3 == 1) {
            z = x - y;
        } else {
            z = x * y;
        }
        
        /* Use volatile to prevent optimization */
        global_volatile += z;
    }
    
    /* Complex SSA web */
    int result = x;
    for (int j = 0; j < 10; j++) {
        if (j % 2) {
            result += y;
        } else {
            result += z;
        }
    }
    
    return result;
}

/* ========== BLOCK generation ========== */
void test_blocks(void) {
    int outer = 0;
    
    /* Level 1 block */
    {
        int level1 = 1;
        
        /* Level 2 block */
        {
            int level2 = 2;
            
            /* Level 3 block inside loop */
            for (int i = 0; i < 5; i++) {
                /* Level 4 block */
                {
                    int level4 = i * 2;
                    level2 += level4;
                    
                    /* Level 5 block with if */
                    if (i % 2 == 0) {
                        /* Level 6 block */
                        {
                            int level6 = level4 * 3;
                            level1 += level6;
                        }
                    }
                }
            }
            
            outer += level2;
        }
        
        outer += level1;
    }
    
    /* Another complex block structure */
    if (outer > 0) {
        /* Block with its own locals */
        {
            int if_block_var = outer * 2;
            {
                int nested_in_if = if_block_var + 1;
                outer = nested_in_if;
            }
        }
    } else {
        /* Else block with different structure */
        {
            int else_block_var = 100;
            for (int i = 0; i < 3; i++) {
                int loop_in_else = else_block_var + i;
                outer += loop_in_else;
            }
        }
    }
    
    use(&outer);
}

/* ========== CONSTRUCTOR generation ========== */
struct ComplexStruct {
    int a;
    float b;
    double c;
    int arr[4];
};

int helper_func1(void) { return global_volatile + 1; }
float helper_func2(void) { return global_volatile * 1.5f; }
double helper_func3(void) { return global_volatile * 2.5; }

void test_constructors(void) {
    /* Array with non-constant initializers */
    int dynamic_array[4] = {
        helper_func1(),
        helper_func1() * 2,
        global_volatile + 3,
        helper_func1() + global_volatile
    };
    
    /* Struct with designated initializers and non-constants */
    struct ComplexStruct s1 = {
        .a = helper_func1(),
        .b = helper_func2(),
        .c = helper_func3(),
        .arr = { helper_func1(), global_volatile, 3, 4 }
    };
    
    /* Nested struct initialization */
    struct {
        struct ComplexStruct inner;
        int extra;
    } nested = {
        .inner = {
            .a = global_volatile,
            .b = 2.0f,
            .c = 3.0,
            .arr = { 1, 2, 3, 4 }
        },
        .extra = helper_func1()
    };
    
    /* Array of structs with mixed initializers */
    struct ComplexStruct struct_array[2] = {
        { helper_func1(), 1.0f, 2.0, {1, 2, 3, 4} },
        { global_volatile, helper_func2(), helper_func3(), 
          {global_volatile, 2, helper_func1(), 4} }
    };
    
    use(dynamic_array);
    use(&s1);
    use(&nested);
    use(struct_array);
}

/* ========== OMP_CLAUSE generation ========== */
#ifdef _OPENMP
#include <omp.h>

void test_omp_clauses(void) {
    int i;
    const int N = 100;
    int data[N];
    int sum = 0;
    int private_var = 0;
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        data[i] = i + 1;
    }
    
    /* OpenMP with multiple clauses */
    #pragma omp parallel for private(i) shared(data) \
            reduction(+:sum) schedule(dynamic, 4) \
            num_threads(2) if(N > 50)
    for (i = 0; i < N; i++) {
        sum += data[i];
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel private(private_var) \
            default(none) shared(sum, data)
    {
        private_var = omp_get_thread_num();
        
        #pragma omp for nowait
        for (i = 0; i < N; i++) {
            data[i] += private_var;
        }
        
        #pragma omp barrier
        
        #pragma omp single copyprivate(private_var)
        {
            private_var = sum % 100;
        }
    }
    
    /* OpenMP sections with clause */
    #pragma omp parallel sections private(i) \
            lastprivate(private_var)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                data[i] *= 2;
            }
            private_var = 1;
        }
        
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                data[i] /= 2;
            }
            private_var = 2;
        }
    }
    
    use(&sum);
    use(data);
    use(&private_var);
}
#endif

/* ========== TREE_BINFO generation (requires C++ or LTO) ========== */
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

class SecondDerived : public DerivedClass {
public:
    virtual int method() override { return 3; }
    int second_data;
};

void test_binfo(void) {
    DerivedClass d;
    BaseClass* b = &d;
    SecondDerived sd;
    
    /* Use virtual calls to require vtable/binfo */
    int r1 = b->method();
    int r2 = d.method();
    int r3 = sd.method();
    
    /* Casts that require binfo lookup */
    DerivedClass* d2 = dynamic_cast<DerivedClass*>(b);
    BaseClass& b2 = sd;
    
    use(&r1);
    use(&r2);
    use(&r3);
    use(&d2);
    use(&b2);
}
#else
/* For C-only, we rely on LTO to generate BINFO-like structures */
/* Complex structure with function pointers to mimic vtable */
struct VTable {
    int (*method)(void*);
    void (*destroy)(void*);
};

struct Base {
    struct VTable* vtable;
    int data;
};

struct Derived {
    struct Base base;
    int extra;
};

int derived_method(void* self) { return ((struct Derived*)self)->extra; }
void derived_destroy(void* self) { /* nothing */ }

struct VTable derived_vtable = { derived_method, derived_destroy };

void test_binfo(void) {
    struct Derived d = { { &derived_vtable, 10 }, 20 };
    struct Base* b = (struct Base*)&d;
    
    /* Indirect call through vtable */
    int result = b->vtable->method(b);
    
    use(&result);
    use(&d);
}
#endif

/* ========== Main function to orchestrate everything ========== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Test all tree node types */
    test_identifiers();
    test_vectors();
    
    result += test_ssa(argc > 1 ? 100 : 50);
    
    test_blocks();
    test_constructors();
    
    #ifdef _OPENMP
    test_omp_clauses();
    #endif
    
    test_binfo();
    
    /* Ensure everything is used */
    result += global_volatile;
    
    /* Print to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result % 256;
}
