/* test_tree_kind.c - Comprehensive test for GCC tree node kinds */

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Variable and function names create IDENTIFIER_NODE */
int some_unique_identifier_1;
double another_identifier_2;
void function_identifier_3(void) {}

/* ==================== SSA_NAME ==================== */
/* Complex arithmetic with loops forces SSA form */
int ssa_test(int n) {
    int a = 0, b = 1, c = 2;
    for (int i = 0; i < n; ++i) {
        a = a + b * c;          /* Creates SSA_NAME nodes */
        b = b + i;
        c = c - a;
        if (a > 100) {
            a = a / 2;          /* More SSA opportunities */
        }
    }
    
    /* Nested loop for additional SSA complexity */
    for (int j = 0; j < 10; ++j) {
        for (int k = 0; k < j; ++k) {
            a += k * j;
        }
    }
    
    return a + b + c;
}

/* ==================== BLOCK ==================== */
/* Nested blocks create BLOCK nodes */
void block_test(void) {
    int x = 0;
    {                           /* Outer block */
        int y = 1;
        {                       /* Inner block 1 */
            int z = 2;
            x = y + z;
        }
        {                       /* Inner block 2 */
            double w = 3.14;
            x += (int)w;
        }
    }
    
    /* Switch statement with blocks */
    switch (x) {
        case 1: {
            int temp = 10;
            x += temp;
            break;
        }
        case 2: {
            int temp = 20;
            x *= temp;
            break;
        }
        default: {
            int temp = 30;
            x = temp;
        }
    }
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers create CONSTRUCTOR nodes */
struct point {
    int x;
    int y;
    double z;
};

union data {
    int i;
    float f;
    char c;
};

void constructor_test(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor */
    struct point p1 = {10, 20, 3.14};
    struct point p2 = {.x = 5, .y = 15, .z = 2.71};
    
    /* Union constructor */
    union data d1 = {.i = 42};
    union data d2 = {.f = 3.14f};
    
    /* Nested struct constructor */
    struct nested {
        struct point pt;
        int id;
        char name[4];
    } n1 = {{1, 2, 3.0}, 100, {'A', 'B', 'C', '\0'}};
    
    /* Multi-dimensional array constructor */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_test(int n) {
    int i, sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Various OpenMP clauses */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static)
    for (i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    #pragma omp parallel shared(arr) firstprivate(n)
    {
        #pragma omp for nowait
        for (i = 0; i < 50; i++) {
            arr[i] *= 2;
        }
    }
    
    /* OMP sections with clauses */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < 25; i++) arr[i] += 1;
        }
        #pragma omp section
        {
            for (i = 25; i < 50; i++) arr[i] -= 1;
        }
    }
}
#endif

/* ==================== TREE_VEC ==================== */
/* GCC extensions create TREE_VEC nodes */
#ifdef __GNUC__
void tree_vec_test(void) {
    /* Using statement expressions - a GCC extension */
    int a = ({ 
        int x = 5; 
        int y = 10; 
        x + y;      /* Returns 15 */
    });
    
    /* Typeof extension */
    typeof(a) b = a * 2;
    
    /* Vector extension (if supported) */
    #ifdef __SSE2__
    typedef int v4si __attribute__ ((vector_size (16)));
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
    #endif
    
    /* Nested statement expressions */
    int c = ({
        int tmp = ({
            int inner = 42;
            inner * 2;
        });
        tmp + 100;
    });
}
#endif

#ifdef __cplusplus
/* ==================== TREE_BINFO (C++ only) ==================== */
/* Class hierarchies create TREE_BINFO nodes */

class Base1 {
public:
    virtual void foo() { cout << "Base1::foo" << endl; }
    int x;
};

class Base2 {
public:
    virtual void bar() { cout << "Base2::bar" << endl; }
    double y;
};

class Derived : public Base1, public Base2 {
public:
    virtual void foo() override { cout << "Derived::foo" << endl; }
    virtual void bar() override { cout << "Derived::bar" << endl; }
    char z;
};

/* Template with inheritance */
template<typename T>
class TemplateBase {
public:
    virtual T get_value() = 0;
};

template<typename T>
class TemplateDerived : public TemplateBase<T> {
public:
    virtual T get_value() override { return T(); }
};

void binfo_test(void) {
    Derived d;
    Base1* b1 = &d;
    Base2* b2 = &d;
    
    b1->foo();
    b2->bar();
    
    /* Multiple inheritance creates BINFO nodes */
    Derived* pd = dynamic_cast<Derived*>(b1);
    
    /* Template instantiation */
    TemplateDerived<int> td;
    int val = td.get_value();
}

/* Additional C++ features that generate various tree nodes */
namespace test_namespace {
    class Nested {
    public:
        enum Enum { VALUE1, VALUE2, VALUE3 };
        
        struct InnerStruct {
            int a;
            double b;
        };
    };
}

#endif /* __cplusplus */

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char** argv) {
    /* Force processing of all test cases */
    int result = 0;
    
    /* IDENTIFIER_NODE - already created by declarations above */
    some_unique_identifier_1 = 42;
    another_identifier_2 = 3.14;
    function_identifier_3();
    
    /* SSA_NAME */
    result += ssa_test(100);
    
    /* BLOCK */
    block_test();
    
    /* CONSTRUCTOR */
    constructor_test();
    
    /* OMP_CLAUSE */
    #ifdef _OPENMP
    omp_test(100);
    #endif
    
    /* TREE_VEC */
    #ifdef __GNUC__
    tree_vec_test();
    #endif
    
    #ifdef __cplusplus
    /* TREE_BINFO */
    binfo_test();
    
    /* Additional C++ constructs */
    test_namespace::Nested::InnerStruct s = {1, 2.0};
    test_namespace::Nested::Enum e = test_namespace::Nested::VALUE2;
    
    cout << "Test completed successfully" << endl;
    #else
    printf("Test completed successfully\n");
    #endif
    
    return result % 256;  /* Ensure non-trivial return value */
}
