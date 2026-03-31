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
    /* Generate multiple distinct identifiers */
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
    
    /* Use complex expressions with these identifiers */
    int result = MAKE_ID(0) + MAKE_ID(1) * MAKE_ID(2) - MAKE_ID(3) / 
                 MAKE_ID(4) + MAKE_ID(5) % MAKE_ID(6) + MAKE_ID(7) ^ 
                 MAKE_ID(8) | MAKE_ID(9);
    
    use(&result);
    
    /* Function calls with different identifiers */
    extern int external_func_1(int);
    extern int external_func_2(int);
    extern int external_func_3(int);
    
    result = external_func_1(result) + external_func_2(result) * 
             external_func_3(result);
    use(&result);
}

/* ========== TREE_VEC generation ========== */
/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

void test_vectors(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Multiple vector operations */
    v4si r1 = a + b;
    v4si r2 = a * b;
    v4si r3 = a & b;
    v4si r4 = a | b;
    v4si r5 = a ^ b;
    
    /* Mixed vector operations */
    v4si result = r1 + r2 * r3 - r4 / (r5 + 1);
    
    /* Float vectors */
    v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fb = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fc = fa * fb + fa / fb;
    
    /* Double vectors */
    v2df da = {1.0, 2.0};
    v2df db = {3.0, 4.0};
    v2df dc = da * db - da / db;
    
    use(&result);
    use(&fc);
    use(&dc);
}

/* ========== SSA_NAME generation ========== */
int test_ssa(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Complex loop with multiple assignments to create SSA */
    for (int i = 0; i < n; i++) {
        /* Multiple conditional assignments */
        if (i % 3 == 0) {
            x = y + z;
        } else if (i % 3 == 1) {
            x = y * z;
        } else {
            x = y - z;
        }
        
        /* Another variable with phi nodes */
        if (x > 10) {
            y = x / 2;
        } else {
            y = x * 2;
        }
        
        /* Third variable with complex flow */
        z = (z + x) * y;
        
        /* Volatile to prevent optimization */
        global_volatile = i;
    }
    
    /* Return complex expression to ensure SSA */
    return x + y * z - (x ^ y) | (z & y);
}

/* ========== BLOCK generation ========== */
void test_blocks(void) {
    int outer = 0;
    
    /* Level 1 block */
    {
        int level1_a = 1;
        int level1_b = 2;
        
        /* Level 2 block */
        {
            int level2_a = level1_a * 2;
            int level2_b = level1_b * 3;
            
            /* Level 3 block inside if */
            if (level2_a > level2_b) {
                int level3_a = level2_a + level2_b;
                int level3_b = level2_a - level2_b;
                
                /* Level 4 block inside loop */
                for (int i = 0; i < 5; i++) {
                    int level4 = level3_a * i + level3_b;
                    outer += level4;
                }
            } else {
                int level3_c = level2_a | level2_b;
                int level3_d = level2_a & level2_b;
                
                /* Another nested block */
                {
                    int level4 = level3_c ^ level3_d;
                    outer -= level4;
                }
            }
        }
        
        /* Another block at level 1 */
        {
            int another_var = outer * 2;
            use(&another_var);
        }
    }
    
    /* Block in switch */
    switch (outer % 4) {
        case 0: {
            int case0_var = outer + 1;
            use(&case0_var);
            break;
        }
        case 1: {
            int case1_var = outer * 2;
            use(&case1_var);
            break;
        }
        default: {
            int default_var = outer / 2;
            use(&default_var);
            break;
        }
    }
}

/* ========== CONSTRUCTOR generation ========== */
struct ComplexStruct {
    int a;
    int b;
    float c;
    double d;
    int arr[3];
};

int func1(void) { return global_volatile + 1; }
int func2(void) { return global_volatile + 2; }
float func3(void) { return global_volatile * 1.5f; }
double func4(void) { return global_volatile * 2.5; }

void test_constructors(void) {
    /* Struct with non-constant initializers */
    struct ComplexStruct s1 = {
        .a = func1(),
        .b = func2(),
        .c = func3(),
        .d = func4(),
        .arr = { func1(), func2(), func1() + func2() }
    };
    
    /* Array with non-constant initializers */
    int dynamic_array[4] = {
        func1(),
        func2(),
        func1() * func2(),
        func1() + func2()
    };
    
    /* Nested struct initialization */
    struct Nested {
        struct Inner {
            int x;
            int y;
        } inner;
        int z;
    };
    
    struct Nested n1 = {
        .inner = { func1(), func2() },
        .z = func1() + func2()
    };
    
    /* Union with constructor */
    union Mixed {
        int i;
        float f;
        void *p;
    };
    
    union Mixed u1 = { .i = func1() };
    union Mixed u2 = { .f = func3() };
    
    use(&s1);
    use(&dynamic_array);
    use(&n1);
    use(&u1);
    use(&u2);
}

/* ========== OMP_CLAUSE generation ========== */
#ifdef _OPENMP
#include <omp.h>

void test_omp_clauses(void) {
    int i;
    int n = 1000;
    int sum = 0;
    int arr[1000];
    int private_var = 0;
    int firstprivate_var = 42;
    int lastprivate_var = 0;
    int reduction_sum = 0;
    
    /* Initialize array */
    for (i = 0; i < n; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel with multiple clauses */
    #pragma omp parallel private(private_var) firstprivate(firstprivate_var) \
                         shared(arr) reduction(+:reduction_sum) \
                         default(none) num_threads(4) if(n > 100)
    {
        int thread_id = omp_get_thread_num();
        private_var = thread_id;
        
        #pragma omp for schedule(dynamic, 16) nowait \
                     ordered collapse(1) lastprivate(lastprivate_var)
        for (i = 0; i < n; i++) {
            private_var++;
            arr[i] += private_var + firstprivate_var;
            reduction_sum += arr[i];
            
            /* Ordered directive with clause */
            #pragma omp ordered depend(source)
            {
                if (i == n - 1) {
                    lastprivate_var = arr[i];
                }
            }
        }
        
        /* Barrier with clause */
        #pragma omp barrier
        
        /* Single directive with copyprivate */
        #pragma omp single copyprivate(private_var)
        {
            private_var = 999;
        }
        
        /* Critical with name */
        #pragma omp critical(my_critical)
        {
            sum += reduction_sum;
        }
    }
    
    /* Another OpenMP construct: sections */
    #pragma omp parallel sections private(private_var) \
                               firstprivate(firstprivate_var)
    {
        #pragma omp section
        {
            private_var = 1;
            sum += private_var + firstprivate_var;
        }
        
        #pragma omp section
        {
            private_var = 2;
            sum += private_var + firstprivate_var;
        }
    }
    
    /* Task with depend clauses */
    int task_dep_var = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: task_dep_var)
        {
            task_dep_var = 100;
        }
        
        #pragma omp task depend(in: task_dep_var)
        {
            sum += task_dep_var;
        }
    }
    
    use(&sum);
    use(&lastprivate_var);
}
#endif

/* ========== TREE_BINFO generation (requires C++ or LTO) ========== */
#ifdef __cplusplus
/* C++ version for TREE_BINFO */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int method1() { return 1; }
    int data1;
};

class DerivedClass : public BaseClass {
public:
    virtual int method1() override { return 2; }
    virtual int method2() { return 3; }
    int data2;
};

class MultiDerived : public DerivedClass {
public:
    virtual int method1() override { return 4; }
    virtual int method3() { return 5; }
    int data3;
};

void test_binfo(void) {
    BaseClass* base_ptr = new DerivedClass();
    DerivedClass* derived_ptr = new MultiDerived();
    
    /* Virtual calls to generate BINFO lookups */
    int result = base_ptr->method1();
    result += derived_ptr->method1();
    result += derived_ptr->method2();
    
    /* Dynamic cast for RTTI */
    MultiDerived* multi_ptr = dynamic_cast<MultiDerived*>(derived_ptr);
    if (multi_ptr) {
        result += multi_ptr->method3();
    }
    
    /* Multiple inheritance-like access */
    BaseClass& base_ref = *derived_ptr;
    result += base_ref.method1();
    
    delete base_ptr;
    delete derived_ptr;
    
    use(&result);
}
#else
/* C version that might generate BINFO with LTO */
/* Complex struct with function pointers to simulate vtable */
struct VTable {
    int (*method1)(void*);
    int (*method2)(void*);
};

struct Base {
    struct VTable* vtable;
    int data;
};

struct Derived {
    struct Base base;
    int extra_data;
};

int base_method1(void* self) { return ((struct Base*)self)->data; }
int derived_method1(void* self) { return ((struct Derived*)self)->extra_data; }

struct VTable base_vtable = { base_method1, NULL };
struct VTable derived_vtable = { derived_method1, NULL };

void test_binfo(void) {
    struct Derived d;
    d.base.vtable = &derived_vtable;
    d.base.data = 10;
    d.extra_data = 20;
    
    /* Call through vtable */
    int result = d.base.vtable->method1(&d);
    
    /* Cast to base */
    struct Base* base_ptr = (struct Base*)&d;
    result += base_ptr->vtable->method1(base_ptr);
    
    use(&result);
}
#endif

/* ========== Main function combining everything ========== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Trigger all test functions */
    test_identifiers();
    
    test_vectors();
    
    result = test_ssa(argc > 1 ? argc : 100);
    
    test_blocks();
    
    test_constructors();
    
#ifdef _OPENMP
    test_omp_clauses();
#endif
    
    test_binfo();
    
    /* Final computation to prevent dead code elimination */
    result += global_volatile;
    
    /* Print something to ensure execution */
    printf("Result: %d\n", result);
    
    return result != 0;
}
