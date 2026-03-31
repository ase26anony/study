/* test_tree_kind.c - Comprehensive test for GCC tree kind coverage */

/* Enable GNU extensions for TREE_VEC generation */
#define _GNU_SOURCE

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Simple variable declaration creates IDENTIFIER_NODE */
int some_unique_identifier_123;

/* ==================== TREE_VEC ==================== */
/* Use GCC statement expression extension to create TREE_VEC */
#ifdef __GNUC__
#define CREATE_VEC() ({ \
    int a = 1, b = 2, c = 3; \
    (typeof(a)[]){a, b, c}; \
})
#endif

/* ==================== SSA_NAME ==================== */
/* Function with arithmetic operations to generate SSA names */
int generate_ssa_names(int n) {
    int x = 0;
    int y = 1;
    
    /* Loop with arithmetic to force SSA form */
    for (int i = 0; i < n; ++i) {
        x = x + y * i;      /* This creates SSA_NAME nodes */
        y = y + x;          /* More SSA transformations */
    }
    
    /* Conditional to create phi nodes */
    int result = (x > 100) ? x : y;
    return result;
}

/* ==================== BLOCK ==================== */
/* Function with nested blocks to generate BLOCK nodes */
void generate_blocks(void) {
    /* Outer block */
    int outer = 10;
    
    {
        /* Inner block 1 */
        int inner1 = outer * 2;
        
        {
            /* Inner block 2 */
            int inner2 = inner1 + 5;
            (void)inner2; /* Use variable */
        }
        
        {
            /* Another inner block */
            volatile int temp = inner1;
            (void)temp;
        }
    }
    
    /* Another block with different scope */
    if (outer > 0) {
        int conditional_var = outer * 3;
        (void)conditional_var;
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

void generate_constructors(void) {
    /* Array constructor */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Struct constructor */
    struct Point p1 = {.x = 1, .y = 2, .z = 3};
    
    /* Nested struct constructor */
    struct Data d1 = {
        .id = 1001,
        .location = {.x = 5, .y = 6, .z = 7},
        .values = {1.1f, 2.2f, 3.3f, 4.4f}
    };
    
    /* Complex array constructor */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    
    (void)arr; (void)p1; (void)d1; (void)matrix;
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas generate OMP_CLAUSE nodes */
#ifdef _OPENMP
void generate_omp_clauses(void) {
    int i;
    int sum = 0;
    int data[100];
    
    /* Initialize data */
    for (i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    /* OpenMP parallel for with multiple clauses */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static, 10)
    for (i = 0; i < 100; i++) {
        sum += data[i];
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel num_threads(4) shared(data)
    {
        #pragma omp for nowait
        for (i = 0; i < 50; i++) {
            data[i] *= 2;
        }
    }
}
#else
void generate_omp_clauses(void) {
    /* Dummy function when OpenMP not available */
    printf("OpenMP not enabled\n");
}
#endif

#ifdef __cplusplus
/* ==================== TREE_BINFO (C++ only) ==================== */
/* Class hierarchy to generate BINFO nodes */
class BaseClass1 {
public:
    virtual ~BaseClass1() {}
    virtual void method1() = 0;
    int base_data1;
};

class BaseClass2 {
public:
    virtual ~BaseClass2() {}
    virtual void method2() = 0;
    float base_data2;
};

/* Derived class with multiple inheritance creates TREE_BINFO */
class DerivedClass : public BaseClass1, public BaseClass2 {
public:
    virtual ~DerivedClass() {}
    virtual void method1() override {
        base_data1 = 42;
    }
    virtual void method2() override {
        base_data2 = 3.14f;
    }
    
    void derived_method() {
        method1();
        method2();
    }
    
private:
    double derived_data;
};

/* Template class for additional BINFO complexity */
template<typename T>
class TemplateBase {
public:
    virtual T get_value() const = 0;
};

class ConcreteClass : public TemplateBase<int>, public BaseClass1 {
public:
    virtual int get_value() const override {
        return 123;
    }
    virtual void method1() override {
        base_data1 = 456;
    }
};

void generate_binfo(void) {
    DerivedClass* obj1 = new DerivedClass();
    ConcreteClass* obj2 = new ConcreteClass();
    
    BaseClass1* base1 = obj1;
    BaseClass2* base2 = obj1;
    
    base1->method1();
    base2->method2();
    
    obj1->derived_method();
    int val = obj2->get_value();
    
    delete obj1;
    delete obj2;
    
    (void)val;
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Use the identifier */
    some_unique_identifier_123 = 42;
    
    /* Generate TREE_VEC if supported */
    #ifdef __GNUC__
    {
        auto vec = CREATE_VEC();
        (void)vec;
    }
    #endif
    
    /* Generate SSA names */
    result = generate_ssa_names(100);
    
    /* Generate blocks */
    generate_blocks();
    
    /* Generate constructors */
    generate_constructors();
    
    /* Generate OpenMP clauses */
    generate_omp_clauses();
    
    #ifdef __cplusplus
    /* Generate BINFO nodes (C++ only) */
    generate_binfo();
    
    cout << "Result: " << result << endl;
    #else
    printf("Result: %d\n", result);
    #endif
    
    return 0;
}
