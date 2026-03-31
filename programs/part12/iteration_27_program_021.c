/* test_tree_kind_coverage.c - Comprehensive test for GCC tree kind coverage */

/* Enable GNU extensions for TREE_VEC */
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
    (typeof(a))((a + b) * c); \
})
#endif

/* ==================== SSA_NAME ==================== */
/* Function with loop to generate SSA names */
int generate_ssa_names(int n) {
    int result = 0;
    for (int i = 0; i < n; ++i) {
        result = result + i * 2;  /* This creates SSA_NAME nodes */
    }
    
    /* More complex SSA generation */
    int x = 0, y = 0;
    for (int j = 0; j < n; ++j) {
        x = y + j;
        y = x * 2;
        result += x + y;
    }
    
    return result;
}

/* ==================== BLOCK ==================== */
/* Nested blocks with local variables */
void nested_blocks(void) {
    int outer = 10;
    {
        int inner1 = outer * 2;
        {
            int inner2 = inner1 + 5;
            {
                int inner3 = inner2 * 3;
                (void)inner3; /* Use variable to avoid warnings */
            }
        }
    }
    
    /* Another block with switch */
    {
        int block_var = 42;
        switch (block_var) {
            case 42: {
                int case_var = 100;
                (void)case_var;
                break;
            }
            default: {
                int default_var = 200;
                (void)default_var;
            }
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
    int values[5];
    struct Point pt;
};

void use_constructors(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor (C99 designated initializer) */
    struct Point p1 = {.x = 10, .y = 20, .z = 30};
    
    /* Nested constructor */
    struct Data d1 = {
        .values = {10, 20, 30, 40, 50},
        .pt = {.x = 100, .y = 200, .z = 300}
    };
    
    /* More complex constructor with expressions */
    int arr2[3] = {1 + 1, 2 * 2, 3 << 1};
    
    (void)arr; (void)p1; (void)d1; (void)arr2; /* Use variables */
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void openmp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 42;
    
    #pragma omp parallel for private(i) reduction(+:sum) firstprivate(private_var)
    for (i = 0; i < n; i++) {
        sum += i * private_var;
    }
    
    /* More OpenMP constructs */
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            { int section_var = 1; (void)section_var; }
            
            #pragma omp section  
            { int section_var = 2; (void)section_var; }
        }
    }
    
    (void)sum; /* Use variable */
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
    int base_data2;
};

class DerivedClass : public BaseClass1, public BaseClass2 {
public:
    virtual ~DerivedClass() {}
    virtual void method1() override { base_data1 = 1; }
    virtual void method2() override { base_data2 = 2; }
    int derived_data;
};

/* Template to generate more complex BINFO structures */
template<typename T>
class TemplateBase {
public:
    virtual void template_method() {}
    T template_data;
};

class ConcreteClass : public TemplateBase<int>, public TemplateBase<double> {
public:
    void template_method() override {}
};

void use_classes(void) {
    DerivedClass d;
    ConcreteClass c;
    
    BaseClass1* b1 = &d;
    BaseClass2* b2 = &d;
    
    b1->method1();
    b2->method2();
    
    (void)c; /* Use variable */
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    int result = 0;
    
    /* Use the identifier */
    some_unique_identifier_123 = 42;
    
    /* Generate TREE_VEC if supported */
    #ifdef __GNUC__
    result += CREATE_VEC();
    #endif
    
    /* Generate SSA names */
    result += generate_ssa_names(100);
    
    /* Use nested blocks */
    nested_blocks();
    
    /* Use constructors */
    use_constructors();
    
    /* Use OpenMP if available */
    #ifdef _OPENMP
    openmp_test(1000);
    #endif
    
    #ifdef __cplusplus
    /* Use C++ classes for BINFO */
    use_classes();
    
    cout << "Result: " << result << endl;
    #else
    printf("Result: %d\n", result);
    #endif
    
    return 0;
}
