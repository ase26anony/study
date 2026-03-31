/* test_tree_kind.c - Coverage test for GCC's tree.cc get_kind function */

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
int ssa_test_function(int n) {
    int a = 0, b = 1, c = 2;
    for (int i = 0; i < n; ++i) {
        a = a + b * c;          /* Creates SSA_NAME nodes */
        b = b + i;
        c = c * 2 - i;
        if (a > 100) {
            a = a / 2;          /* More SSA transformations */
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
/* Nested blocks with local variables */
void block_test_function(void) {
    int outer = 0;
    { /* BLOCK 1 */
        int inner1 = 10;
        outer += inner1;
        { /* BLOCK 2 - deeper nesting */
            int inner2 = 20;
            outer += inner2;
            { /* BLOCK 3 - even deeper */
                int inner3 = 30;
                outer += inner3;
            }
        }
    }
    
    /* Switch creates multiple blocks */
    switch (outer) {
        case 10: {
            int case_var = 100;
            outer += case_var;
            break;
        }
        case 60: {
            int case_var = 200;
            outer += case_var;
            break;
        }
        default: {
            int case_var = 300;
            outer += case_var;
        }
    }
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers create CONSTRUCTOR nodes */
struct my_struct {
    int a;
    float b;
    char c;
};

union my_union {
    int x;
    double y;
};

void constructor_test(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor */
    struct my_struct s1 = {10, 3.14f, 'A'};
    struct my_struct s2 = {.a = 20, .c = 'B', .b = 2.71f};
    
    /* Union constructor */
    union my_union u1 = {42};
    union my_union u2 = {.y = 3.14159};
    
    /* Nested struct with constructor */
    struct nested {
        struct my_struct inner;
        int extra;
    } n1 = {{5, 1.23f, 'X'}, 100};
    
    /* Multi-dimensional array constructor */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
}

/* ==================== TREE_VEC ==================== */
/* Using GCC statement expressions for TREE_VEC */
#ifdef __GNUC__
void tree_vec_test(void) {
    /* Statement expression with multiple elements */
    int result = ({
        int x = 10;
        int y = 20;
        int z = 30;
        x + y + z;  /* Returns value, creates TREE_VEC internally */
    });
    
    /* Another statement expression */
    int vec_result = ({
        int a = 5;
        int b = a * 2;
        int c = b + 3;
        c;
    });
}
#endif

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 42;
    
    #pragma omp parallel for private(i) reduction(+:sum) firstprivate(private_var)
    for (i = 0; i < n; i++) {
        sum += i * private_var;
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
    
    #pragma omp task shared(sum) if(n > 100)
    {
        sum *= 2;
    }
}
#endif

#ifdef __cplusplus
/* ==================== TREE_BINFO ==================== */
/* C++ class hierarchy for BINFO nodes */
class BaseClass1 {
public:
    virtual void method1() {}
    int base_data1;
};

class BaseClass2 {
public:
    virtual void method2() {}
    double base_data2;
};

/* Multiple inheritance creates BINFO nodes */
class DerivedClass : public BaseClass1, public BaseClass2 {
public:
    virtual void method1() override {}
    virtual void method2() override {}
    int derived_data;
};

/* Template class for additional complexity */
template<typename T>
class TemplateBase {
public:
    virtual T process(T x) { return x * 2; }
};

class ConcreteDerived : public TemplateBase<int> {
public:
    virtual int process(int x) override { return x * 3; }
};

void cpp_binfo_test(void) {
    DerivedClass obj1;
    ConcreteDerived obj2;
    
    BaseClass1* ptr1 = &obj1;
    BaseClass2* ptr2 = &obj1;
    
    ptr1->method1();
    ptr2->method2();
    
    int result = obj2.process(10);
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char** argv) {
    int test_value = 100;
    
    /* Exercise all test functions */
    int ssa_result = ssa_test_function(test_value);
    block_test_function();
    constructor_test();
    
#ifdef __GNUC__
    tree_vec_test();
#endif
    
#ifdef _OPENMP
    omp_test(test_value);
#endif
    
#ifdef __cplusplus
    cpp_binfo_test();
    
    cout << "Test completed. SSA result: " << ssa_result << endl;
    return 0;
#else
    printf("Test completed. SSA result: %d\n", ssa_result);
    return 0;
#endif
}
