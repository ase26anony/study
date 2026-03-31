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
static int another_identifier;

void identifier_func(void) {
    int local_identifier = 42;
    some_unique_identifier = local_identifier + another_identifier;
}

/* ==================== TREE_VEC ==================== */
/* Use GCC statement expressions with multiple elements */
#ifdef __GNUC__
int tree_vec_example(void) {
    /* This creates a TREE_VEC in GCC's internal representation */
    int result = ({ 
        int a = 5; 
        int b = 10; 
        int c = a + b; 
        c * 2; 
    });
    return result;
}
#endif

/* ==================== SSA_NAME ==================== */
/* Complex enough to trigger SSA formation */
int ssa_name_example(int n) {
    int a = 0, b = 1, c;
    
    /* Loop with arithmetic to force SSA */
    for (int i = 0; i < n; ++i) {
        a = a + i;
        b = b * 2;
        c = a + b;
        
        /* Conditional to create phi nodes */
        if (i % 2 == 0) {
            a = c - i;
        } else {
            b = c + i;
        }
    }
    
    /* More SSA opportunities */
    int x = a;
    for (int j = 0; j < 5; ++j) {
        x = x * j + 1;
        a = a + x;
    }
    
    return a + b + c;
}

/* ==================== BLOCK ==================== */
/* Nested blocks with local variables */
int block_example(int val) {
    /* Outer block */
    int outer = val;
    
    {
        /* Inner block 1 */
        int inner1 = outer * 2;
        
        {
            /* Inner block 2 */
            int inner2 = inner1 + 5;
            
            {
                /* Deeply nested block */
                int inner3 = inner2 - 3;
                outer = inner3;
            }
        }
        
        /* Another block with different scope */
        if (outer > 0) {
            int temp = outer;
            outer = temp * temp;
        }
    }
    
    return outer;
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers */
struct my_struct {
    int a;
    float b;
    char c;
};

int constructor_example(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor (C99 designated initializer) */
    struct my_struct s = {.a = 10, .b = 3.14f, .c = 'X'};
    
    /* Nested initializer */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    
    /* Complex initializer with expressions */
    int complex_init[3] = {1 + 2, arr[0] * 2, s.a};
    
    return arr[0] + s.a + matrix[0][0] + complex_init[1];
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas */
#ifdef _OPENMP
void omp_clause_example(int n) {
    int i;
    int sum = 0;
    
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static)
    for (i = 0; i < n; i++) {
        sum += i;
    }
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            int local = 42;
        }
        
        #pragma omp for nowait
        for (int j = 0; j < 10; j++) {
            /* Do work */
        }
    }
}
#endif

#ifdef __cplusplus
/* ==================== TREE_BINFO (C++ only) ==================== */
/* Class hierarchy to generate BINFO nodes */
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
    
    BaseClass* bp = &d;
    bp->base_method();
}
#endif

/* ==================== MAIN DRIVER ==================== */
int main(void) {
    int result = 0;
    
    /* Trigger IDENTIFIER_NODE */
    identifier_func();
    result += some_unique_identifier;
    
    /* Trigger TREE_VEC */
    #ifdef __GNUC__
    result += tree_vec_example();
    #endif
    
    /* Trigger SSA_NAME */
    result += ssa_name_example(10);
    
    /* Trigger BLOCK */
    result += block_example(5);
    
    /* Trigger CONSTRUCTOR */
    result += constructor_example();
    
    /* Trigger OMP_CLAUSE */
    #ifdef _OPENMP
    omp_clause_example(100);
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
