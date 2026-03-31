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
void identifier_function(void) {
    int local_identifier = some_unique_identifier + another_identifier;
}

/* ==================== TREE_VEC ==================== */
/* Using GCC statement expressions with multiple elements */
#ifndef __cplusplus
#define VEC_EXPRESSION ({ \
    int a = 1; \
    int b = 2; \
    int c = 3; \
    a + b + c; \
})
#endif

#ifdef __cplusplus
/* In C++, template instantiations can create TREE_VEC */
template<typename T>
T vec_template(T x) {
    return x * 2;
}

template<typename T, typename U>
auto mixed_vec(T t, U u) -> decltype(t + u) {
    return t + u;
}
#endif

void test_tree_vec(void) {
#ifndef __cplusplus
    int result = VEC_EXPRESSION;
#else
    int r1 = vec_template<int>(5);
    auto r2 = mixed_vec(3, 4.5);
#endif
}

/* ==================== SSA_NAME ==================== */
/* Complex loop to force SSA form creation */
int ssa_test(int n) {
    int a = 0;
    int b = 1;
    
    /* Multiple assignments to same variable in loop */
    for (int i = 0; i < n; ++i) {
        a = a + i;
        b = b * 2 - a;
        
        /* Conditional creates phi nodes */
        if (i % 2 == 0) {
            a = b + 1;
        } else {
            a = b - 1;
        }
    }
    
    /* Another loop with induction variable */
    int sum = 0;
    for (int j = 0; j < n; ++j) {
        sum += j * j;
        if (sum > 100) {
            sum = sum / 2;
        }
    }
    
    return a + b + sum;
}

/* ==================== BLOCK ==================== */
/* Nested blocks with local variables */
void block_test(void) {
    int outer = 10;
    
    {
        /* Inner block 1 */
        int inner1 = outer * 2;
        {
            /* Nested inner block */
            int inner2 = inner1 + 5;
            inner2 += outer;
        }
    }
    
    {
        /* Another block with different scope */
        double block_var = 3.14159;
        {
            float another_block_var = 2.71828f;
            block_var += another_block_var;
        }
    }
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers for arrays and structs */
struct Point {
    int x;
    int y;
    int z;
};

union Data {
    int i;
    float f;
    char str[4];
};

void constructor_test(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor (designated initializer) */
    struct Point p1 = {.x = 10, .y = 20, .z = 30};
    struct Point p2 = {10, 20, 30};
    
    /* Union constructor */
    union Data d1 = {.i = 42};
    union Data d2 = {.f = 3.14f};
    
    /* Nested struct with constructor */
    struct Line {
        struct Point start;
        struct Point end;
    };
    
    struct Line line = {
        .start = {1, 2, 3},
        .end = {4, 5, 6}
    };
    
    /* Multi-dimensional array constructor */
    int matrix[2][3] = {
        {1, 2, 3},
        {4, 5, 6}
    };
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas generate OMP_CLAUSE nodes */
#ifdef _OPENMP
#include <omp.h>

void omp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    
    #pragma omp parallel for private(i) reduction(+:sum) firstprivate(private_var)
    for (i = 0; i < n; i++) {
        sum += i;
    }
    
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            { sum += 1; }
            
            #pragma omp section
            { sum += 2; }
        }
    }
    
    int data[100];
    #pragma omp parallel for simd schedule(static) aligned(data:32)
    for (i = 0; i < 100; i++) {
        data[i] = i * 2;
    }
}
#endif

/* ==================== C++ Specific: TREE_BINFO ==================== */
#ifdef __cplusplus
/* Class hierarchy to generate BINFO nodes */
class Base1 {
public:
    virtual ~Base1() {}
    virtual void foo() = 0;
    int base1_data;
};

class Base2 {
public:
    virtual ~Base2() {}
    virtual void bar() {}
    double base2_data;
};

class Derived : public Base1, public Base2 {
public:
    void foo() override {}
    void bar() override {}
    char derived_data;
};

class DeepDerived : public Derived {
public:
    void foo() override {}
    int deep_data;
};

void binfo_test(void) {
    Derived d;
    Base1* b1 = &d;
    Base2* b2 = &d;
    
    DeepDerived dd;
    Derived* pd = &dd;
    
    b1->foo();
    b2->bar();
}

/* More complex hierarchy with virtual inheritance */
class VirtualBase {
public:
    virtual ~VirtualBase() {}
    int virtual_data;
};

class Middle1 : virtual public VirtualBase {
public:
    int middle1_data;
};

class Middle2 : virtual public VirtualBase {
public:
    int middle2_data;
};

class MultiDerived : public Middle1, public Middle2 {
public:
    int multiderived_data;
};
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char** argv) {
    /* Reference all identifiers to ensure they're used */
    some_unique_identifier = 1;
    another_identifier = 2;
    identifier_function();
    
    /* Test TREE_VEC generation */
    test_tree_vec();
    
    /* Force SSA creation with complex computation */
    int ssa_result = ssa_test(100);
    
    /* Exercise block creation */
    block_test();
    
    /* Use constructors */
    constructor_test();
    
#ifdef _OPENMP
    /* Test OpenMP if available */
    omp_test(1000);
#endif

#ifdef __cplusplus
    /* C++ specific tests */
    binfo_test();
    
    /* Use templates for TREE_VEC */
    int t1 = vec_template<int>(10);
    auto t2 = mixed_vec(5, 3.14);
    
    cout << "Results: " << ssa_result << ", " << t1 << ", " << t2 << endl;
#else
    printf("Result: %d\n", ssa_result);
#endif
    
    return 0;
}
