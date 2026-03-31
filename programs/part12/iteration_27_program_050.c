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
void identifier_node_function() {
    int local_identifier = 42;
    some_unique_identifier_1 = local_identifier;
}

/* ==================== TREE_VEC ==================== */
/* GCC statement expressions can create TREE_VEC nodes */
#ifdef __GNUC__
int tree_vec_example() {
    /* Using statement expression with multiple elements */
    int result = ({ 
        int a = 5; 
        int b = 10; 
        int c = a + b; 
        c; 
    });
    return result;
}
#endif

/* ==================== SSA_NAME ==================== */
/* Complex arithmetic with loops to force SSA form */
int ssa_name_example(int n) {
    int a = 0;
    int b = 1;
    
    /* Loop with arithmetic to create SSA names */
    for (int i = 0; i < n; ++i) {
        a = a + b;      /* Creates SSA_NAME for 'a' */
        b = b * 2;      /* Creates SSA_NAME for 'b' */
        a = a - i;      /* More SSA_NAME creation */
    }
    
    /* Conditional to create phi nodes */
    int result = (a > 0) ? a : b;
    
    /* Nested arithmetic */
    for (int j = 0; j < 10; ++j) {
        result = result + j * 2;
        if (result > 100) {
            result = result / 2;
        }
    }
    
    return result;
}

/* ==================== BLOCK ==================== */
/* Nested blocks with local variables */
void block_example() {
    int outer = 10;
    
    /* First nested block */
    {
        int inner1 = outer + 5;
        
        /* Second nested block */
        {
            int inner2 = inner1 * 2;
            {
                /* Third nested block */
                int inner3 = inner2 - 3;
                outer = inner3;
            }
        }
    }
    
    /* Switch with blocks */
    switch (outer) {
        case 1: {
            int case_var = 100;
            outer = case_var;
            break;
        }
        case 2: {
            int case_var = 200;
            outer = case_var;
            break;
        }
        default: {
            int case_var = 300;
            outer = case_var;
        }
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
    int id;
    struct Point location;
    float values[4];
};

void constructor_example() {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor (C99 designated initializer) */
    struct Point p1 = {.x = 10, .y = 20, .z = 30};
    
    /* Nested struct with array constructor */
    struct Data d1 = {
        .id = 1,
        .location = {.x = 100, .y = 200, .z = 300},
        .values = {1.1f, 2.2f, 3.3f, 4.4f}
    };
    
    /* 2D array constructor */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    
    /* Union constructor */
    union Mixed {
        int i;
        float f;
    } u = {.f = 3.14f};
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_clause_example(int size) {
    int i;
    int sum = 0;
    int data[100];
    
    /* Initialize data */
    for (i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    /* Various OpenMP clauses */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static, 10)
    for (i = 0; i < 100; i++) {
        sum += data[i];
    }
    
    /* Parallel region with shared and private clauses */
    #pragma omp parallel private(i) shared(data, sum)
    {
        #pragma omp for nowait
        for (i = 0; i < 50; i++) {
            #pragma omp atomic
            sum += data[i] * 2;
        }
    }
    
    /* Sections with different clauses */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < 25; i++) {
                data[i] = 0;
            }
        }
        
        #pragma omp section
        {
            for (i = 25; i < 50; i++) {
                data[i] = 1;
            }
        }
    }
}
#endif

/* ==================== C++ Specific: TREE_BINFO ==================== */
#ifdef __cplusplus
/* Class hierarchy to create BINFO (base information) nodes */
class BaseClass1 {
public:
    virtual void base1_method() {
        cout << "Base1 method" << endl;
    }
    virtual ~BaseClass1() {}
    int base_data1;
};

class BaseClass2 {
public:
    virtual void base2_method() {
        cout << "Base2 method" << endl;
    }
    virtual ~BaseClass2() {}
    int base_data2;
};

class DerivedClass : public BaseClass1, public BaseClass2 {
public:
    virtual void base1_method() override {
        cout << "Derived::base1_method" << endl;
    }
    
    virtual void base2_method() override {
        cout << "Derived::base2_method" << endl;
    }
    
    void derived_method() {
        cout << "Derived method" << endl;
    }
    
    int derived_data;
};

class DeepDerived : public DerivedClass {
public:
    virtual void base1_method() override {
        cout << "DeepDerived::base1_method" << endl;
    }
    
    int deep_data;
};

void tree_binfo_example() {
    DerivedClass* d = new DerivedClass();
    BaseClass1* b1 = d;  /* Upcast - uses BINFO */
    BaseClass2* b2 = d;  /* Upcast - uses BINFO */
    
    b1->base1_method();
    b2->base2_method();
    
    /* Dynamic cast uses BINFO */
    DerivedClass* d2 = dynamic_cast<DerivedClass*>(b1);
    
    /* Multiple inheritance hierarchy */
    DeepDerived* dd = new DeepDerived();
    BaseClass1* b1_from_dd = dd;
    DerivedClass* d_from_dd = dd;
    
    delete d;
    delete dd;
}
#endif

/* ==================== Main Driver ==================== */
int main() {
    /* Ensure all constructs are referenced/used */
    identifier_node_function();
    
    #ifdef __GNUC__
    int vec_result = tree_vec_example();
    #endif
    
    int ssa_result = ssa_name_example(20);
    block_example();
    constructor_example();
    
    #ifdef _OPENMP
    omp_clause_example(100);
    #endif
    
    #ifdef __cplusplus
    tree_binfo_example();
    
    cout << "Test completed. SSA result: " << ssa_result << endl;
    #else
    printf("Test completed. SSA result: %d\n", ssa_result);
    #endif
    
    return 0;
}
