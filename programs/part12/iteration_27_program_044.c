/* test_tree_kind_coverage.c - Generate all tree node types for GCC coverage */

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Variable and function names create IDENTIFIER_NODE */
int some_unique_identifier;
void identifier_func(void) {
    int another_identifier = 42;
    (void)another_identifier;
}

/* ==================== SSA_NAME ==================== */
/* Complex arithmetic with loops forces SSA form */
int ssa_test(int n) {
    int a = 0, b = 1, c;
    for (int i = 0; i < n; ++i) {
        a = a + i;      /* Creates SSA_NAME for 'a' */
        b = b * 2;      /* Creates SSA_NAME for 'b' */
        c = a + b;      /* Creates SSA_NAME for 'c' */
    }
    return c;
}

/* ==================== BLOCK ==================== */
/* Nested blocks with local variables */
void block_test(void) {
    int outer = 1;
    { /* BLOCK node starts here */
        int inner = 2;
        { /* Another nested BLOCK */
            int innermost = 3;
            outer = inner + innermost;
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

void constructor_test(void) {
    /* Array constructor */
    int arr[3] = {1, 2, 3};
    
    /* Struct constructor */
    struct point p1 = {.x = 10, .y = 20, .z = 30};
    
    /* Nested struct constructor */
    struct point points[2] = {{1, 2, 3}, {4, 5, 6}};
    
    (void)arr;
    (void)p1;
    (void)points;
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas generate OMP_CLAUSE nodes */
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
/* GCC extensions create TREE_VEC nodes */
#ifdef __GNUC__
void tree_vec_test(void) {
    /* Using statement expression - creates TREE_VEC */
    int x = ({ 
        int y = 5; 
        int z = 10; 
        y + z; 
    });
    
    /* Vector extension (if available) */
    #ifdef __VECTOR_TYPES_SUPPORTED_P
    typedef int v4si __attribute__((vector_size(16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    (void)c;
    #endif
    
    (void)x;
}
#endif

/* ==================== C++ Specific: TREE_BINFO ==================== */
#ifdef __cplusplus
/* Class hierarchies generate TREE_BINFO nodes */
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

void binfo_test(void) {
    Derived d;
    Base1* b1 = &d;
    Base2* b2 = &d;
    
    b1->foo();
    b2->bar();
    
    /* Multiple inheritance creates complex BINFO structures */
    Derived* array[3];
    for (int i = 0; i < 3; ++i) {
        array[i] = new Derived();
    }
    
    for (int i = 0; i < 3; ++i) {
        delete array[i];
    }
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char** argv) {
    /* Reference all identifiers to ensure they're processed */
    some_unique_identifier = 1;
    identifier_func();
    
    /* Force SSA generation */
    int ssa_result = ssa_test(100);
    
    /* Execute block test */
    block_test();
    
    /* Execute constructor test */
    constructor_test();
    
    /* Execute OpenMP test if available */
    #ifdef _OPENMP
    omp_test(1000);
    #endif
    
    /* Execute TREE_VEC test if using GCC */
    #ifdef __GNUC__
    tree_vec_test();
    #endif
    
    /* Execute C++ specific tests */
    #ifdef __cplusplus
    binfo_test();
    cout << "C++ mode: All tree nodes generated" << endl;
    #else
    printf("C mode: All tree nodes generated\n");
    printf("SSA test result: %d\n", ssa_result);
    #endif
    
    return 0;
}
