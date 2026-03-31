/* test_tree_kind.c - Comprehensive test for GCC tree node kinds */

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Variable and function names create IDENTIFIER_NODE */
int unique_identifier_1;
void identifier_function(void) {
    int local_identifier = 42;
    (void)local_identifier;
}

/* ==================== TREE_VEC ==================== */
/* GCC statement expressions with multiple elements can create TREE_VEC */
#ifdef __GNUC__
int tree_vec_example(void) {
    /* Using statement expression with multiple elements */
    int result = ({
        int a = 5;
        int b = 10;
        int c = 15;
        a + b + c;
    });
    return result;
}
#endif

/* ==================== SSA_NAME ==================== */
/* Complex arithmetic with loops forces SSA form */
int ssa_name_example(int n) {
    int a = 0;
    int b = 1;
    
    /* Loop with arithmetic to generate SSA names */
    for (int i = 0; i < n; ++i) {
        a = a + b * i;      /* Creates SSA names during optimization */
        b = b ^ a;          /* More complex operation */
    }
    
    /* Conditional to create phi nodes */
    int c = (a > 100) ? a : b;
    
    /* Nested arithmetic */
    for (int j = 0; j < 10; ++j) {
        c = c * 2 + j;
    }
    
    return a + b + c;
}

/* ==================== BLOCK ==================== */
/* Nested blocks with local variables */
int block_example(int x) {
    /* Outer block */
    int outer = x * 2;
    
    {
        /* Inner block 1 */
        int inner1 = outer + 5;
        
        {
            /* Inner block 2 */
            int inner2 = inner1 * 3;
            outer = inner2;
        }
        
        {
            /* Another inner block */
            int temp = 100;
            outer += temp;
        }
    }
    
    /* Yet another block */
    {
        int final = outer;
        return final;
    }
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers create CONSTRUCTOR nodes */
struct Point {
    int x;
    int y;
    int z;
};

struct Data {
    int values[5];
    struct Point pt;
};

int constructor_example(void) {
    /* Array constructor */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Struct constructor (C99 designated initializer) */
    struct Point p1 = {.x = 1, .y = 2, .z = 3};
    
    /* Nested constructor */
    struct Data d1 = {
        .values = {1, 2, 3, 4, 5},
        .pt = {.x = 10, .y = 20, .z = 30}
    };
    
    /* Compound literal */
    struct Point *ptr = &(struct Point){100, 200, 300};
    
    return arr[0] + p1.x + d1.values[0] + ptr->x;
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
#include <omp.h>

int omp_clause_example(int size) {
    int sum = 0;
    int i;
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static) if(size > 1000)
    for (i = 0; i < size; i++) {
        sum += i;
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel sections private(i) num_threads(4)
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
    
    return sum;
}
#endif

#ifdef __cplusplus
/* ==================== TREE_BINFO (C++ only) ==================== */
/* Class hierarchy creates BINFO nodes */
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

void tree_binfo_example(void) {
    DerivedClass d;
    d.base_method();
    
    MultipleDerived md;
    md.method1();
    md.method2();
    
    /* Virtual function calls through base pointers */
    BaseClass* bp = &d;
    bp->base_method();
    
    MultipleBase1* mbp1 = &md;
    MultipleBase2* mbp2 = &md;
    mbp1->method1();
    mbp2->method2();
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    int result = 0;
    
    /* Trigger IDENTIFIER_NODE */
    identifier_function();
    unique_identifier_1 = 5;
    
    /* Trigger TREE_VEC */
    #ifdef __GNUC__
    result += tree_vec_example();
    #endif
    
    /* Trigger SSA_NAME */
    result += ssa_name_example(100);
    
    /* Trigger BLOCK */
    result += block_example(50);
    
    /* Trigger CONSTRUCTOR */
    result += constructor_example();
    
    /* Trigger OMP_CLAUSE */
    #ifdef _OPENMP
    result += omp_clause_example(500);
    #endif
    
    #ifdef __cplusplus
    /* Trigger TREE_BINFO */
    tree_binfo_example();
    
    cout << "C++ Result: " << result << endl;
    #else
    printf("C Result: %d\n", result);
    #endif
    
    return 0;
}
