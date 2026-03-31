/* test_tree_kind.c - Comprehensive test for GCC tree kind coverage */

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Variable and function names create IDENTIFIER_NODE */
int some_unique_identifier;
void identifier_func(int param_identifier) {
    int local_identifier = 42;
    (void)local_identifier;
    (void)param_identifier;
}

/* ==================== TREE_VEC ==================== */
/* Using GCC statement expressions to create TREE_VEC nodes */
#ifdef __GNUC__
#define CREATE_VEC() ({ \
    int a = 1, b = 2, c = 3; \
    (typeof(a))((a + b) * c); \
})
#else
#define CREATE_VEC() (42)
#endif

/* ==================== SSA_NAME ==================== */
/* Complex enough to trigger SSA form */
int ssa_test(int n) {
    int a = 0;
    int b = 1;
    
    /* Loop with arithmetic to force SSA */
    for (int i = 0; i < n; ++i) {
        a = a + b * i;      /* Creates phi nodes in SSA */
        b = b ^ a;          /* More complex operations */
    }
    
    /* Conditional to create control flow merge */
    if (a > 100) {
        a = a % 100;
    } else {
        a = a * 2;
    }
    
    return a;
}

/* ==================== BLOCK ==================== */
/* Nested blocks with local variables */
void block_test(void) {
    int outer = 1;
    
    {
        /* Inner block 1 */
        int inner1 = outer + 1;
        
        {
            /* Inner block 2 */
            int inner2 = inner1 * 2;
            (void)inner2;
        }
        
        {
            /* Inner block 3 with its own scope */
            char inner3 = 'A';
            (void)inner3;
        }
    }
    
    /* Another block with different variable types */
    {
        double d = 3.14;
        float f = 2.71f;
        (void)d; (void)f;
    }
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers create CONSTRUCTOR nodes */
struct point {
    int x;
    int y;
    char label;
};

void constructor_test(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor (C99 designated initializer) */
    struct point p1 = {.x = 10, .y = 20, .label = 'A'};
    
    /* Nested struct constructor */
    struct rectangle {
        struct point top_left;
        struct point bottom_right;
    };
    
    struct rectangle rect = {
        .top_left = {1, 2, 'T'},
        .bottom_right = {10, 20, 'B'}
    };
    
    /* Zero initializer */
    int zeros[10] = {0};
    
    (void)arr;
    (void)p1;
    (void)rect;
    (void)zeros;
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
#include <omp.h>

void omp_test(int size) {
    int i;
    int sum = 0;
    int data[100];
    
    /* Initialize data */
    for (i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    /* Multiple OpenMP pragmas with different clauses */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static)
    for (i = 0; i < size && i < 100; i++) {
        sum += data[i];
    }
    
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            i = 1;
        }
        #pragma omp section
        {
            i = 2;
        }
    }
    
    #pragma omp parallel firstprivate(sum) shared(data)
    {
        int tid = omp_get_thread_num();
        (void)tid;
    }
}
#else
void omp_test(int size) {
    (void)size;
    /* Dummy implementation when OpenMP not available */
}
#endif

/* ==================== C++ Specific: TREE_BINFO ==================== */
#ifdef __cplusplus

/* Base class for BINFO generation */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual void base_method() = 0;
    int base_data;
};

/* Another base class for multiple inheritance */
class AnotherBase {
public:
    virtual ~AnotherBase() {}
    virtual void another_method() {}
    double another_data;
};

/* Derived class with multiple inheritance */
class DerivedClass : public BaseClass, public AnotherBase {
public:
    void base_method() override {
        base_data = 42;
    }
    
    void another_method() override {
        another_data = 3.14;
    }
    
    void derived_method() {
        /* Access through base pointers */
        BaseClass* bp = this;
        AnotherBase* ap = this;
        bp->base_method();
        ap->another_method();
    }
    
    int derived_data;
};

/* Template class for additional complexity */
template<typename T>
class TemplateClass : public BaseClass {
public:
    void base_method() override {
        value = T();
    }
    
    T value;
};

void cpp_binfo_test(void) {
    DerivedClass derived;
    derived.base_method();
    derived.another_method();
    derived.derived_method();
    
    TemplateClass<int> tpl;
    tpl.base_method();
    
    /* Polymorphic usage */
    BaseClass* poly = &derived;
    poly->base_method();
    
    /* Multiple inheritance casts */
    DerivedClass* dptr = &derived;
    BaseClass* bptr = dptr;
    AnotherBase* aptr = dptr;
    
    (void)bptr;
    (void)aptr;
}

#endif /* __cplusplus */

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char** argv) {
    /* Reference all identifiers to ensure they're used */
    some_unique_identifier = 0;
    identifier_func(0);
    
    /* Test TREE_VEC generation */
    int vec_result = CREATE_VEC();
    
    /* Test SSA_NAME generation */
    int ssa_result = ssa_test(100);
    
    /* Test BLOCK generation */
    block_test();
    
    /* Test CONSTRUCTOR generation */
    constructor_test();
    
    /* Test OMP_CLAUSE generation */
    omp_test(50);
    
#ifdef __cplusplus
    /* Test C++ BINFO generation */
    cpp_binfo_test();
    
    cout << "C++ Test Complete. Results: " 
         << vec_result << ", " << ssa_result << endl;
#else
    printf("C Test Complete. Results: %d, %d\n", vec_result, ssa_result);
#endif
    
    return 0;
}
