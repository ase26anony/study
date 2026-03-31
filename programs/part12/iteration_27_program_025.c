/* test_tree_kind.c - Comprehensive test for GCC tree node kinds */

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Simple variable/function names create IDENTIFIER_NODE */
int global_identifier = 42;
void function_identifier(void) {
    int local_identifier = global_identifier;
}

/* ==================== SSA_NAME ==================== */
/* Complex operations that force SSA form */
int ssa_test(int n) {
    int a = 0;
    int b = 1;
    
    /* Loop with arithmetic to generate SSA names */
    for (int i = 0; i < n; ++i) {
        a = a + b * i;      /* Creates SSA_NAME nodes during optimization */
        b = a - b;
    }
    
    /* Conditional to create phi nodes */
    int result = (a > 0) ? a : b;
    return result;
}

/* ==================== BLOCK ==================== */
/* Nested blocks create BLOCK nodes */
void block_test(void) {
    /* Outer block */
    int x = 1;
    
    {
        /* Inner block 1 */
        int y = 2;
        x += y;
        
        {
            /* Inner block 2 */
            int z = 3;
            x += z;
        }
    }
    
    {
        /* Another block */
        int w = 4;
        x += w;
    }
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers create CONSTRUCTOR nodes */
struct point {
    int x;
    int y;
    int z;
};

void constructor_test(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor (designated initializer) */
    struct point p1 = {.x = 10, .y = 20, .z = 30};
    
    /* Nested struct constructor */
    struct point points[2] = {{1, 2, 3}, {4, 5, 6}};
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_test(int n) {
    int i;
    int sum = 0;
    
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static)
    for (i = 0; i < n; i++) {
        sum += i;
    }
    
    #pragma omp parallel sections
    {
        #pragma omp section
        { sum += 1; }
        
        #pragma omp section
        { sum += 2; }
    }
}
#endif

/* ==================== TREE_VEC ==================== */
/* GCC extensions that create TREE_VEC nodes */
#ifdef __GNUC__
void tree_vec_test(void) {
    /* Using statement expressions - a GCC extension */
    int a = ({ 
        int b = 5; 
        int c = 10; 
        b + c; 
    });
    
    /* Typeof is another GCC extension that can create TREE_VEC */
    typeof(a) b = a * 2;
    
    /* Vector types extension */
    typedef int v4si __attribute__((vector_size(16)));
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
}
#endif

#ifdef __cplusplus
/* ==================== TREE_BINFO (C++ only) ==================== */
/* Class hierarchies create TREE_BINFO nodes */
class BaseClass {
public:
    virtual void base_method() {
        int x = 0;
    }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual void base_method() override {
        int y = 1;
    }
    int derived_data;
};

class MultipleBase1 {
public:
    virtual void method1() {}
};

class MultipleBase2 {
public:
    virtual void method2() {}
};

class MultipleDerived : public MultipleBase1, public MultipleBase2 {
public:
    virtual void method1() override {}
    virtual void method2() override {}
};

void binfo_test(void) {
    DerivedClass d;
    BaseClass* b = &d;
    b->base_method();
    
    MultipleDerived md;
    MultipleBase1* mb1 = &md;
    MultipleBase2* mb2 = &md;
}
#endif

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
    
#ifdef __GNUC__
    tree_vec_test();
#endif
    
#ifdef __cplusplus
    binfo_test();
    cout << "C++ test complete. SSA result: " << ssa_result << endl;
#else
    printf("C test complete. SSA result: %d\n", ssa_result);
#endif
    
    return 0;
}
