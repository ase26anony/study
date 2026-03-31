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
void function_with_identifier() {
    int local_identifier = 42;
    (void)local_identifier;
}

/* ==================== TREE_VEC ==================== */
/* GCC statement expressions with multiple elements can create TREE_VEC */
#ifdef __GNUC__
int tree_vec_example() {
    /* Using GCC statement expression extension */
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
/* Complex arithmetic with loops forces SSA form */
int ssa_name_example(int n) {
    int a = 0;
    int b = 1;
    
    /* Loop with multiple assignments to create SSA names */
    for (int i = 0; i < n; ++i) {
        a = a + b;      /* Creates phi nodes in SSA */
        b = b * 2;      /* More SSA names */
        if (a > 100) {
            a = a / 2;  /* Conditional assignment */
        }
    }
    
    /* Complex expression with multiple uses */
    int c = (a * b) + (a / b) - (a % b);
    return c;
}

/* ==================== BLOCK ==================== */
/* Nested blocks with local variables */
void block_example() {
    int outer = 1;
    
    {  /* BLOCK node */
        int inner = 2;
        outer += inner;
        
        {  /* Another nested BLOCK */
            int deeper = 3;
            outer += deeper;
        }
    }
    
    /* Switch statement creates blocks */
    switch (outer) {
        case 1: {
            int case_var = 10;
            outer = case_var;
            break;
        }
        case 2: {
            int case_var = 20;
            outer = case_var;
            break;
        }
        default: {
            int case_var = 30;
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

void constructor_example() {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor */
    struct Point p1 = {10, 20, 30};
    
    /* Designated initializer */
    struct Point p2 = {.x = 5, .y = 15, .z = 25};
    
    /* Nested struct with constructor */
    struct Line {
        struct Point start;
        struct Point end;
    };
    
    struct Line line = {
        {1, 2, 3},
        {4, 5, 6}
    };
    
    (void)arr; (void)p1; (void)p2; (void)line;
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_clause_example(int size) {
    int i;
    int sum = 0;
    int array[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        array[i] = i;
    }
    
    /* Multiple OpenMP pragmas with different clauses */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static)
    for (i = 0; i < 100; i++) {
        sum += array[i];
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
    
    #pragma omp parallel firstprivate(sum) shared(array)
    {
        #pragma omp for nowait
        for (i = 0; i < 50; i++) {
            array[i] *= 2;
        }
    }
}
#endif

/* ==================== C++ Specific: TREE_BINFO ==================== */
#ifdef __cplusplus
/* Class hierarchy to create BINFO (base information) nodes */
class BaseClass1 {
public:
    virtual void method1() { cout << "Base1" << endl; }
    int base_data1;
};

class BaseClass2 {
public:
    virtual void method2() { cout << "Base2" << endl; }
    int base_data2;
};

/* Multiple inheritance creates BINFO nodes */
class DerivedClass : public BaseClass1, public BaseClass2 {
public:
    virtual void method1() override { cout << "Derived::method1" << endl; }
    virtual void method2() override { cout << "Derived::method2" << endl; }
    int derived_data;
};

/* Template with inheritance */
template<typename T>
class TemplateBase {
public:
    virtual T process(T x) { return x * 2; }
};

class ConcreteDerived : public TemplateBase<int> {
public:
    virtual int process(int x) override { return x * 3; }
};

void binfo_example() {
    DerivedClass obj;
    BaseClass1* ptr1 = &obj;  /* Upcast - uses BINFO */
    BaseClass2* ptr2 = &obj;  /* Upcast - uses BINFO */
    
    ptr1->method1();
    ptr2->method2();
    
    ConcreteDerived template_obj;
    int result = template_obj.process(10);
    cout << "Template result: " << result << endl;
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main() {
    /* Reference all examples to ensure they're compiled */
    function_with_identifier();
    
    #ifdef __GNUC__
    int vec_result = tree_vec_example();
    #else
    int vec_result = 15;
    #endif
    
    int ssa_result = ssa_name_example(20);
    block_example();
    constructor_example();
    
    #ifdef _OPENMP
    omp_clause_example(100);
    #endif
    
    #ifdef __cplusplus
    binfo_example();
    cout << "Results: " << vec_result << ", " << ssa_result << endl;
    return 0;
    #else
    printf("Results: %d, %d\n", vec_result, ssa_result);
    return 0;
    #endif
}
