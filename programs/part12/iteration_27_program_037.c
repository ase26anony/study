/* test_tree_kind.c - Comprehensive test for GCC tree kind coverage */

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
void function_identifier_3(void) {
    int local_identifier = 42;
}

/* ==================== SSA_NAME ==================== */
/* Complex arithmetic with loops forces SSA form */
int ssa_test(int n) {
    int x = 0;
    int y = 1;
    
    /* This loop creates SSA_NAME nodes during optimization */
    for (int i = 0; i < n; ++i) {
        x = x + y * i;      /* Creates phi nodes in SSA */
        y = y + x;          /* More SSA complexity */
        if (x > 100) {
            x = x / 2;      /* Conditional assignment creates more SSA */
        }
    }
    
    /* Nested loop for additional SSA complexity */
    for (int j = 0; j < 10; ++j) {
        for (int k = 0; k < j; ++k) {
            x += k * j;
        }
    }
    
    return x;
}

/* ==================== BLOCK ==================== */
/* Nested blocks create BLOCK nodes */
void block_test(void) {
    int outer = 0;
    
    {  /* BLOCK 1 */
        int inner1 = 10;
        outer += inner1;
        
        {  /* BLOCK 2 - deeper nesting */
            int inner2 = 20;
            outer += inner2;
            
            {  /* BLOCK 3 - even deeper */
                int inner3 = 30;
                outer += inner3;
            }
        }
    }
    
    /* Switch statement creates blocks */
    switch (outer) {
        case 10: {
            int case_var = 100;  /* Block in case */
            break;
        }
        case 60: {
            int another_case_var = 200;
            break;
        }
        default: {
            int default_var = 300;
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

struct rectangle {
    struct point top_left;
    struct point bottom_right;
};

void constructor_test(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor (C99 designated initializer) */
    struct point p1 = {.x = 10, .y = 20, .z = 30};
    
    /* Nested struct constructor */
    struct rectangle rect = {
        .top_left = {1, 2, 3},
        .bottom_right = {4, 5, 6}
    };
    
    /* Union constructor */
    union data {
        int i;
        float f;
        char str[20];
    } data1 = {.i = 42};
    
    /* Zero initialization */
    struct point zero_point = {0};
    
    /* Partial initialization */
    int partial[10] = {[0] = 1, [5] = 2, [9] = 3};
}

/* ==================== TREE_VEC ==================== */
/* GCC extensions create TREE_VEC nodes */
#ifdef __GNUC__
void tree_vec_test(void) {
    /* Using statement expression - creates TREE_VEC */
    int vec_result = ({
        int a = 5;
        int b = 10;
        int c = 15;
        a + b + c;  /* Last expression is result */
    });
    
    /* Typeof with multiple values */
    typeof(int[3]) arr_type;
    
    /* Vector types extension */
    typedef int v4si __attribute__ ((vector_size (16)));
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
}
#endif

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 100;
    
    #pragma omp parallel for private(i) reduction(+:sum) firstprivate(private_var) \
            schedule(static, 10) if(n > 1000)
    for (i = 0; i < n; i++) {
        sum += i * private_var;
    }
    
    /* More OpenMP constructs */
    #pragma omp parallel
    {
        #pragma omp sections nowait
        {
            #pragma omp section
            { int section1 = 1; }
            
            #pragma omp section
            { int section2 = 2; }
        }
        
        #pragma omp barrier
        
        #pragma omp single copyprivate(private_var)
        {
            private_var = 200;
        }
    }
    
    /* OMP task with dependencies */
    #pragma omp task depend(inout: sum) priority(10)
    {
        sum *= 2;
    }
}
#endif

#ifdef __cplusplus
/* ==================== TREE_BINFO (C++ only) ==================== */
/* Class hierarchies create TREE_BINFO nodes */

/* Base class */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual void base_method() {
        int x = 10;
    }
    int base_data;
};

/* Another base for multiple inheritance */
class AnotherBase {
public:
    virtual void another_method() {}
    double another_data;
};

/* Derived class with multiple inheritance */
class DerivedClass : public BaseClass, public AnotherBase {
public:
    virtual void base_method() override {
        base_data = 42;
    }
    
    virtual void another_method() override {
        another_data = 3.14;
    }
    
    void derived_method() {
        int y = 20;
    }
    
    int derived_data;
};

/* More complex hierarchy */
class DeepDerived : public DerivedClass {
public:
    virtual void base_method() override {
        base_data = 100;
        derived_data = 200;
    }
    
    void deep_method() {
        int z = 30;
    }
};

/* Template class for additional complexity */
template<typename T>
class TemplateBase {
public:
    T template_data;
    virtual void template_method(T val) {
        template_data = val;
    }
};

class ConcreteDerived : public TemplateBase<int> {
public:
    virtual void template_method(int val) override {
        template_data = val * 2;
    }
};

void cpp_binfo_test(void) {
    BaseClass* obj1 = new DerivedClass();
    AnotherBase* obj2 = new DerivedClass();
    DeepDerived* obj3 = new DeepDerived();
    ConcreteDerived* obj4 = new ConcreteDerived();
    
    obj1->base_method();
    obj2->another_method();
    obj3->deep_method();
    obj4->template_method(42);
    
    delete obj1;
    delete obj2;
    delete obj3;
    delete obj4;
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char** argv) {
    /* Reference all test functions to ensure they're compiled */
    
    /* IDENTIFIER_NODE - already referenced by declarations */
    function_identifier_3();
    
    /* SSA_NAME */
    int ssa_result = ssa_test(100);
    
    /* BLOCK */
    block_test();
    
    /* CONSTRUCTOR */
    constructor_test();
    
    /* TREE_VEC */
    #ifdef __GNUC__
    tree_vec_test();
    #endif
    
    /* OMP_CLAUSE */
    #ifdef _OPENMP
    omp_test(10000);
    #endif
    
    /* TREE_BINFO (C++ only) */
    #ifdef __cplusplus
    cpp_binfo_test();
    
    cout << "C++ test completed. SSA result: " << ssa_result << endl;
    return 0;
    #else
    printf("C test completed. SSA result: %d\n", ssa_result);
    return 0;
    #endif
}
