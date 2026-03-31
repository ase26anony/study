/* test_tree_kind_coverage.c - Comprehensive test for GCC tree kind coverage */

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Variable and function names create IDENTIFIER_NODE */
int global_identifier;
void function_identifier(void) {
    int local_identifier;
    local_identifier = 42;
    global_identifier = local_identifier;
}

/* ==================== SSA_NAME ==================== */
/* Complex arithmetic with loops forces SSA form */
int ssa_test(int n) {
    int a = 0, b = 1, c;
    for (int i = 0; i < n; ++i) {
        c = a + b;      /* This will create SSA_NAME nodes */
        a = b;
        b = c;
        /* Complex enough to trigger SSA optimization passes */
        for (int j = 0; j < i; ++j) {
            a = a ^ j;  /* More SSA opportunities */
            b = b * 2;
        }
    }
    return c;
}

/* ==================== BLOCK ==================== */
/* Nested blocks with local variables */
void block_test(void) {
    int outer = 0;
    {
        int inner1 = 1;
        {
            int inner2 = 2;
            {
                int inner3 = 3;
                outer = inner1 + inner2 + inner3;
            }
        }
    }
    
    /* Switch statement creates additional blocks */
    switch (outer) {
        case 0: {
            int case_var = 10;
            break;
        }
        case 6: {
            int case_var = 20;
            break;
        }
        default: {
            int case_var = 30;
            break;
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
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor (designated initializer) */
    struct point p1 = {.x = 10, .y = 20, .z = 30};
    
    /* Nested struct constructor */
    struct nested {
        struct point p;
        int id;
    } n1 = {{1, 2, 3}, 100};
    
    /* Complex array constructor */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_test(void) {
    int i, sum = 0;
    int array[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        array[i] = i;
    }
    
    /* Multiple OpenMP clauses to generate different OMP_CLAUSE nodes */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static, 10)
    for (i = 0; i < 100; i++) {
        sum += array[i];
    }
    
    /* Another OpenMP construct */
    #pragma omp parallel
    {
        #pragma omp sections private(i)
        {
            #pragma omp section
            { i = 1; }
            #pragma omp section
            { i = 2; }
        }
    }
}
#else
void omp_test(void) {
    /* Dummy function when OpenMP not available */
}
#endif

/* ==================== TREE_VEC ==================== */
/* GCC extensions that create TREE_VEC nodes */
#ifdef __GNUC__
void tree_vec_test(void) {
    /* Using statement expressions - can create TREE_VEC */
    int a = ({ 
        int x = 5; 
        int y = 10; 
        x + y; 
    });
    
    /* Typeof with multiple elements */
    typeof(int[3]) arr_type;
    
    /* Vector types (GCC extension) */
    typedef int v4si __attribute__((vector_size(16)));
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
}
#else
void tree_vec_test(void) {
    /* Dummy for non-GCC compilers */
}
#endif

/* ==================== TREE_BINFO (C++ only) ==================== */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual void base_method() { }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual void base_method() override { }
    int derived_data;
};

class MultiBase1 {
public:
    virtual ~MultiBase1() {}
    int data1;
};

class MultiBase2 {
public:
    virtual ~MultiBase2() {}
    int data2;
};

class MultipleDerived : public MultiBase1, public MultiBase2 {
public:
    int derived_data;
};

void binfo_test(void) {
    DerivedClass d;
    BaseClass* b = &d;  /* This requires BINFO for casting */
    
    MultipleDerived md;
    MultiBase1* mb1 = &md;
    MultiBase2* mb2 = &md;
    
    /* Virtual calls use BINFO for vtable lookups */
    b->base_method();
    
    /* Dynamic cast (requires RTTI, but still uses BINFO) */
    DerivedClass* d2 = dynamic_cast<DerivedClass*>(b);
}
#else
void binfo_test(void) {
    /* Dummy for C mode */
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char** argv) {
    /* Reference all test functions to ensure they're compiled */
    function_identifier();
    
    int ssa_result = ssa_test(10);
    
    block_test();
    
    constructor_test();
    
    omp_test();
    
    tree_vec_test();
    
    binfo_test();
    
    /* Use results to prevent dead code elimination */
#ifdef __cplusplus
    cout << "SSA test result: " << ssa_result << endl;
    cout << "All tree kind tests completed." << endl;
#else
    printf("SSA test result: %d\n", ssa_result);
    printf("All tree kind tests completed.\n");
#endif
    
    return 0;
}
