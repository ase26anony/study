/* test_tree_kind.c - Comprehensive test for GCC tree kind coverage */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 test_tree_kind.c -o test_tree_kind */
/* For C++ mode: g++ -O2 -fopenmp -std=gnu++11 test_tree_kind.c -o test_tree_kind_cpp */

#include <stdio.h>
#include <stdlib.h>

/* Helper to prevent optimization */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* ========== IDENTIFIER_NODE ========== */
/* Variable and function names create IDENTIFIER_NODE */
int some_unique_identifier_1;
static int another_identifier_2;
void function_identifier_3(void) {
    int local_identifier_4 = 0;
    use(&local_identifier_4);
}

/* ========== SSA_NAME ========== */
/* Complex arithmetic with loops forces SSA form */
int ssa_test(int n) {
    int a = 0, b = 1, c = 2;
    for (int i = 0; i < n; ++i) {
        a = a + i * b;      /* Creates SSA_NAME for a */
        b = b + c;          /* Creates SSA_NAME for b */
        c = a - b + c * i;  /* Creates SSA_NAME for c */
    }
    
    /* More complex control flow */
    if (a > 100) {
        b = a * 2;
    } else {
        b = a / 2;
    }
    
    /* Phi node will be created for b here */
    return a + b + c;
}

/* ========== BLOCK ========== */
/* Nested blocks with local variables */
void block_test(void) {
    /* Outer block */
    int x = 10;
    use(&x);
    
    {
        /* Inner block 1 */
        int y = 20;
        use(&y);
        
        {
            /* Inner block 2 */
            int z = 30;
            use(&z);
            
            {
                /* Deeply nested block */
                int w = 40;
                use(&w);
            }
        }
    }
    
    /* Another block with control flow */
    if (x > 0) {
        int temp = x * 2;
        use(&temp);
    }
}

/* ========== CONSTRUCTOR ========== */
/* Aggregate initializers */
struct point {
    int x;
    int y;
    int z;
};

struct data {
    int id;
    struct point pt;
    float values[4];
};

void constructor_test(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor (C99 designated initializers) */
    struct point p1 = {.x = 10, .y = 20, .z = 30};
    struct point p2 = {10, 20, 30};  /* Traditional */
    
    /* Nested struct with array constructor */
    struct data d1 = {
        .id = 100,
        .pt = {.x = 1, .y = 2, .z = 3},
        .values = {1.1f, 2.2f, 3.3f, 4.4f}
    };
    
    /* Complex constructor with mixed types */
    struct {
        int a;
        float b;
        char c[3];
    } mixed = {42, 3.14f, {'a', 'b', 'c'}};
    
    use(arr);
    use(&p1);
    use(&p2);
    use(&d1);
    use(&mixed);
}

/* ========== TREE_VEC ========== */
/* Using GCC statement expressions (GNU extension) */
void tree_vec_test(void) {
    /* Statement expression creates TREE_VEC */
    int result = ({
        int a = 5;
        int b = 10;
        int c = a + b;
        c * 2;  /* Last expression is result */
    });
    
    /* Another example with multiple statements */
    int vec_result = ({
        int x = 1;
        for (int i = 0; i < 3; i++) {
            x += i;
        }
        x;
    });
    
    use(&result);
    use(&vec_result);
}

/* ========== OMP_CLAUSE ========== */
#ifdef _OPENMP
void omp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 42;
    
    /* Multiple OpenMP pragmas with different clauses */
    #pragma omp parallel for private(i) reduction(+:sum) firstprivate(private_var)
    for (i = 0; i < n; i++) {
        sum += i + private_var;
    }
    
    /* Another with different clauses */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            i = 1;
            sum += i;
        }
        #pragma omp section
        {
            i = 2;
            sum += i;
        }
    }
    
    /* OMP critical with clause */
    #pragma omp critical (my_critical)
    {
        sum += 100;
    }
    
    printf("OMP sum: %d\n", sum);
}
#else
void omp_test(int n) {
    printf("OpenMP not enabled\n");
    use(&n);
}
#endif

/* ========== C++ Specific Code for TREE_BINFO ========== */
#ifdef __cplusplus

/* Base class for BINFO generation */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual void base_method() { 
        int x = 0;
        use(&x);
    }
    int base_data;
};

/* Another base class for multiple inheritance */
class AnotherBase {
public:
    virtual ~AnotherBase() {}
    virtual void another_method() {
        int y = 0;
        use(&y);
    }
    int another_data;
};

/* Derived class with single inheritance */
class DerivedSingle : public BaseClass {
public:
    void base_method() override {
        base_data = 10;
    }
    int derived_data;
};

/* Derived class with multiple inheritance */
class DerivedMultiple : public BaseClass, public AnotherBase {
public:
    void base_method() override {
        base_data = 20;
    }
    
    void another_method() override {
        another_data = 30;
    }
    
    int multi_data;
};

/* Template class for additional complexity */
template<typename T>
class TemplateDerived : public BaseClass {
public:
    void base_method() override {
        base_data = sizeof(T);
    }
    T template_data;
};

void cpp_binfo_test(void) {
    /* Create objects to generate BINFO nodes */
    DerivedSingle ds;
    DerivedMultiple dm;
    TemplateDerived<int> td;
    
    /* Use virtual calls to ensure vtable usage */
    BaseClass* b1 = &ds;
    BaseClass* b2 = &dm;
    BaseClass* b3 = &td;
    
    b1->base_method();
    b2->base_method();
    b3->base_method();
    
    /* Cast to check inheritance */
    AnotherBase* ab = &dm;
    ab->another_method();
    
    use(&ds);
    use(&dm);
    use(&td);
}

#else
/* C version - dummy implementation */
void cpp_binfo_test(void) {
    printf("C++ mode not enabled, skipping BINFO tests\n");
}
#endif

/* ========== Main Driver ========== */
int main(int argc, char **argv) {
    int n = 100;
    
    /* Trigger all test functions */
    function_identifier_3();
    
    int ssa_result = ssa_test(n);
    printf("SSA test result: %d\n", ssa_result);
    
    block_test();
    constructor_test();
    tree_vec_test();
    omp_test(n);
    cpp_binfo_test();
    
    /* Use all global identifiers */
    some_unique_identifier_1 = 1;
    another_identifier_2 = 2;
    use(&some_unique_identifier_1);
    use(&another_identifier_2);
    
    return 0;
}
