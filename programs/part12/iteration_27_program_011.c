/* test_tree_kind_coverage.c - Comprehensive test for GCC tree kind coverage */

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Variable and function names create IDENTIFIER_NODE */
int some_unique_identifier_1;
static int another_identifier_2;
void identifier_function_3(void) {
    int local_identifier_4 = 0;
}

/* ==================== SSA_NAME ==================== */
/* Complex arithmetic with loops forces SSA form */
int ssa_test(int n) {
    int a = 0, b = 1, c;
    for (int i = 0; i < n; ++i) {
        a = a + i;          /* Creates SSA_NAME for 'a' */
        b = b * 2;          /* Creates SSA_NAME for 'b' */
        c = a + b;          /* Creates SSA_NAME for 'c' */
        if (c > 100) {
            a = c / 2;      /* Creates phi node in SSA */
        }
    }
    return a + b + c;
}

/* ==================== BLOCK ==================== */
/* Nested blocks with local variables */
void block_test(void) {
    int outer = 0;
    { /* BLOCK 1 */
        int inner1 = 1;
        { /* BLOCK 2 */
            int inner2 = 2;
            { /* BLOCK 3 */
                int inner3 = 3;
                outer = inner1 + inner2 + inner3;
            }
        }
    }
    
    /* Another block with control flow */
    if (outer > 0) {
        int conditional_var = 5;
        outer += conditional_var;
    } else {
        int else_var = 10;
        outer += else_var;
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
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Struct constructor (C99 designated initializer) */
    struct point p1 = {.x = 1, .y = 2, .z = 3};
    
    /* Nested struct constructor */
    struct nested {
        struct point p;
        int id;
    } n1 = {{4, 5, 6}, 100};
    
    /* Zero initializer */
    struct point p2 = {0};
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas generate OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_test(int n) {
    int i, sum = 0;
    int private_var = 42;
    
    #pragma omp parallel for private(i) reduction(+:sum) firstprivate(private_var)
    for (i = 0; i < n; i++) {
        sum += i + private_var;
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
}
#endif

/* ==================== TREE_VEC ==================== */
/* GCC extensions that create TREE_VEC nodes */
#ifdef __GNUC__
void tree_vec_test(void) {
    /* Using statement expressions - creates TREE_VEC */
    int a = ({ 
        int x = 5; 
        int y = 10; 
        x + y; 
    });
    
    /* Typeof with multiple elements */
    typeof(int[3]) arr_type;
    
    /* Using __builtin_choose_expr */
    int b = __builtin_choose_expr(1, 100, 200);
}
#endif

#ifdef __cplusplus
/* ==================== TREE_BINFO (C++ only) ==================== */
/* Class hierarchies create BINFO nodes */
class BaseClass {
public:
    virtual void base_method() {}
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual void base_method() override {}
    int derived_data;
};

class MultipleBase1 {
public:
    virtual void m1() {}
};

class MultipleBase2 {
public:
    virtual void m2() {}
};

class MultipleDerived : public MultipleBase1, public MultipleBase2 {
public:
    virtual void m1() override {}
    virtual void m2() override {}
};

void binfo_test(void) {
    DerivedClass d;
    BaseClass* b = &d;  /* Uses BINFO for upcast */
    b->base_method();
    
    MultipleDerived md;
    MultipleBase1* mb1 = &md;
    MultipleBase2* mb2 = &md;
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char** argv) {
    /* Reference all identifiers to ensure they're processed */
    some_unique_identifier_1 = 1;
    another_identifier_2 = 2;
    identifier_function_3();
    
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
    
#ifdef __cplusplus
    /* Execute C++ specific tests */
    binfo_test();
    
    cout << "C++ Tree kind coverage test complete. Result: " << ssa_result << endl;
    return 0;
#else
    printf("C Tree kind coverage test complete. Result: %d\n", ssa_result);
    return 0;
#endif
}
