/* test_tree_kind.c - Comprehensive test for GCC tree kind coverage */

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Variable and function names create IDENTIFIER_NODE */
int global_identifier = 42;
void function_identifier(void) {
    int local_identifier = global_identifier;
}

/* ==================== SSA_NAME ==================== */
/* Complex arithmetic and loops force SSA form */
int ssa_test(int n) {
    int a = 0, b = 1, c = 2;
    for (int i = 0; i < n; ++i) {
        /* Multiple assignments create SSA_NAME nodes */
        a = a + i;
        b = b * (i + 1);
        c = c - (a % (b + 1));
        
        /* Conditional creates phi nodes in SSA */
        if (i % 2 == 0) {
            a = b + c;
        } else {
            a = c - b;
        }
    }
    
    /* Complex expression with multiple operations */
    int result = (a * b) + (c << 2) - (a / (b | 1));
    for (int j = 0; j < result % 10; ++j) {
        result += j * j;
    }
    
    return result;
}

/* ==================== BLOCK ==================== */
/* Nested blocks create BLOCK nodes */
void block_test(void) {
    /* Outer block */
    int x = 0;
    {
        /* Inner block 1 */
        int y = 1;
        {
            /* Inner block 2 */
            int z = 2;
            x = y + z;
        }
        {
            /* Another inner block */
            int w = 3;
            y = x * w;
        }
    }
    
    /* Block with declarations */
    {
        int a = 5, b = 6, c = 7;
        {
            int d = 8;
            a = b + c + d;
        }
    }
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers create CONSTRUCTOR nodes */
struct point {
    int x;
    int y;
    int z;
};

struct data {
    int id;
    struct point p;
    float values[4];
};

void constructor_test(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor (C99 designated initializers) */
    struct point p1 = {.x = 10, .y = 20, .z = 30};
    struct point p2 = {x: 5, y: 15, z: 25};  /* GNU extension */
    
    /* Nested struct with array constructor */
    struct data d1 = {
        .id = 100,
        .p = {.x = 1, .y = 2, .z = 3},
        .values = {1.0f, 2.0f, 3.0f, 4.0f}
    };
    
    /* Complex constructor with mixed types */
    struct {
        int a;
        char b;
        float c[3];
    } complex = {42, 'X', {1.1f, 2.2f, 3.3f}};
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    
    #pragma omp parallel for private(i) reduction(+:sum) firstprivate(private_var)
    for (i = 0; i < n; i++) {
        sum += i * i;
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
    
    #pragma omp target teams distribute parallel for simd collapse(2)
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            sum += x * y;
        }
    }
}
#endif

/* ==================== TREE_VEC ==================== */
/* GCC extensions that create TREE_VEC nodes */
void tree_vec_test(void) {
    /* Using statement expressions (GNU extension) */
    int a = ({ 
        int x = 5; 
        int y = 10; 
        x + y; 
    });
    
    /* Vector extension (GNU C) */
    typedef int v4si __attribute__ ((vector_size (16)));
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
    
    /* Typeof with multiple elements */
    typeof(int[3]) arr_type = {10, 20, 30};
    
    /* Compound literals in complex expressions */
    struct point *ptr = &(struct point){.x = 1, .y = 2, .z = 3};
}

#ifdef __cplusplus
/* ==================== TREE_BINFO (C++ only) ==================== */
/* Class hierarchies create TREE_BINFO nodes */

class Base1 {
public:
    virtual void foo() { }
    int base1_data;
};

class Base2 {
public:
    virtual void bar() { }
    int base2_data;
};

class Derived : public Base1, public Base2 {
public:
    virtual void foo() override { }
    virtual void bar() override { }
    int derived_data;
};

class DeepDerived : public Derived {
public:
    virtual void foo() override { }
    int deep_data;
};

void binfo_test(void) {
    Derived d;
    Base1* b1 = &d;  // Upcast
    Base2* b2 = &d;  // Upcast (multiple inheritance)
    
    DeepDerived dd;
    Derived* pd = &dd;
    Base1* pb1 = &dd;
    
    // Virtual calls through base pointers
    b1->foo();
    b2->bar();
    
    // Dynamic casts (require RTTI, but still create binfo nodes)
    Derived* dp = dynamic_cast<Derived*>(b1);
    
    // Multiple inheritance with virtual base would create more complex binfo
}

/* Template with inheritance */
template<typename T>
class TemplateBase {
public:
    virtual void template_method() { }
    T data;
};

class Concrete : public TemplateBase<int> {
public:
    virtual void template_method() override { }
};

#endif /* __cplusplus */

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char** argv) {
    /* Reference all test functions to ensure they're compiled */
    function_identifier();
    
    int ssa_result = ssa_test(100);
    
    block_test();
    
    constructor_test();
    
#ifdef _OPENMP
    omp_test(1000);
#endif
    
    tree_vec_test();
    
#ifdef __cplusplus
    binfo_test();
    
    cout << "C++ test complete. SSA result: " << ssa_result << endl;
#else
    printf("C test complete. SSA result: %d\n", ssa_result);
#endif
    
    return 0;
}
