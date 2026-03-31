/* test_tree_kind.c - Generate GCC tree nodes for coverage testing */

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Variable and function names create IDENTIFIER_NODE */
int some_unique_identifier_1;
void identifier_node_func(void) {
    int another_identifier = 42;
    (void)another_identifier;
}

/* ==================== SSA_NAME ==================== */
/* Complex operations that force SSA form */
int ssa_name_test(int n) {
    int a = 0;
    int b = 1;
    
    /* Loop with arithmetic to generate SSA names */
    for (int i = 0; i < n; ++i) {
        a = a + b * i;      /* Creates SSA_NAME for a */
        b = b + a;          /* Creates SSA_NAME for b */
        int c = a * 2;      /* Creates SSA_NAME for c */
        (void)c;
    }
    
    /* Conditional with phi nodes */
    int result;
    if (a > 100) {
        result = a * 2;
    } else {
        result = b * 3;
    }
    
    return result;
}

/* ==================== BLOCK ==================== */
/* Nested blocks create BLOCK nodes */
void block_node_test(void) {
    /* Outer block */
    int x = 10;
    
    {
        /* Inner block 1 */
        int y = 20;
        {
            /* Inner block 2 */
            int z = x + y;
            (void)z;
        }
    }
    
    {
        /* Another inner block */
        float f = 3.14;
        (void)f;
    }
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers create CONSTRUCTOR nodes */
struct point {
    int x;
    int y;
    int z;
};

void constructor_node_test(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor (C99 designated initializer) */
    struct point p1 = {.x = 10, .y = 20, .z = 30};
    
    /* Nested struct constructor */
    struct {
        struct point p;
        int id;
    } obj = {.p = {1, 2, 3}, .id = 100};
    
    (void)arr;
    (void)p1;
    (void)obj;
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_clause_test(void) {
    int i;
    int sum = 0;
    int data[100];
    
    for (i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    /* Multiple OpenMP clauses */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static, 10)
    for (i = 0; i < 100; i++) {
        sum += data[i];
    }
    
    /* Another OpenMP construct */
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Do something in single thread */
        }
        
        #pragma omp for nowait
        for (i = 0; i < 50; i++) {
            data[i] *= 2;
        }
    }
    
    (void)sum;
}
#endif

/* ==================== TREE_VEC ==================== */
/* GCC extensions that create TREE_VEC nodes */
#ifdef __GNUC__
void tree_vec_test(void) {
    /* Using statement expressions - GCC extension */
    int a = ({ 
        int x = 5; 
        int y = 10; 
        x + y; 
    });
    
    /* Vector extension (if available) */
    #ifdef __SSE2__
    typedef int v4si __attribute__ ((vector_size (16)));
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
    (void)v3;
    #endif
    
    (void)a;
}
#endif

#ifdef __cplusplus
/* ==================== TREE_BINFO (C++ only) ==================== */
/* Class hierarchies create TREE_BINFO nodes */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual void base_method() = 0;
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual void base_method() override {
        base_data = 42;
    }
    int derived_data;
};

class MultipleBase1 {
public:
    virtual void method1() {}
    int data1;
};

class MultipleBase2 {
public:
    virtual void method2() {}
    int data2;
};

class MultipleDerived : public MultipleBase1, public MultipleBase2 {
public:
    virtual void method1() override {}
    virtual void method2() override {}
    int derived_data;
};

void tree_binfo_test(void) {
    DerivedClass d;
    d.base_method();
    
    MultipleDerived md;
    md.method1();
    md.method2();
    
    BaseClass* ptr = &d;
    ptr->base_method();
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    /* Reference all test functions to ensure they're processed */
    identifier_node_func();
    
    int ssa_result = ssa_name_test(100);
    
    block_node_test();
    
    constructor_node_test();
    
    #ifdef _OPENMP
    omp_clause_test();
    #endif
    
    #ifdef __GNUC__
    tree_vec_test();
    #endif
    
    #ifdef __cplusplus
    tree_binfo_test();
    
    cout << "C++ test complete. SSA result: " << ssa_result << endl;
    return 0;
    #else
    printf("C test complete. SSA result: %d\n", ssa_result);
    return 0;
    #endif
}
