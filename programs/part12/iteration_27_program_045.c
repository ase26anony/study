/* test_tree_kind.c - Coverage test for GCC's get_kind function */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 test_tree_kind.c -o test_tree_kind */
/* For C++ mode: g++ -O2 -fopenmp -std=gnu++11 test_tree_kind.c -o test_tree_kind_cpp */

#include <stdio.h>
#include <stdlib.h>

/* ==================== IDENTIFIER_NODE ==================== */
/* Any variable/function name creates an IDENTIFIER_NODE */
int some_unique_identifier_123;
static int another_identifier;

/* ==================== SSA_NAME ==================== */
/* Complex loop to force SSA form */
int ssa_test_function(int n) {
    int a = 0;
    int b = 1;
    
    /* This complex loop with multiple assignments forces SSA */
    for (int i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            a = a + i * b;
            b = b + 1;
        } else {
            a = a - i;
            b = b * 2;
        }
        
        /* Nested loop for more SSA complexity */
        for (int j = 0; j < 5; ++j) {
            a = a + j;
            b = b - j;
        }
    }
    
    return a + b;
}

/* ==================== BLOCK ==================== */
/* Multiple nested blocks with local variables */
void block_test_function(void) {
    int x = 0;
    
    { /* BLOCK 1 */
        int y = 10;
        x += y;
        
        { /* BLOCK 2 */
            int z = 20;
            x += z;
            
            { /* BLOCK 3 */
                int w = 30;
                x += w;
            }
        }
    }
    
    /* Another block in a loop */
    for (int i = 0; i < 3; i++) {
        int temp = i * 2;
        x += temp;
    }
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers create CONSTRUCTOR nodes */
struct my_struct {
    int a;
    float b;
    char c;
};

void constructor_test(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor (C99 designated initializer) */
    struct my_struct s1 = {.a = 10, .b = 3.14f, .c = 'X'};
    
    /* Nested struct with constructor */
    struct nested {
        struct my_struct inner;
        int extra;
    } n1 = {{5, 2.71f, 'Y'}, 100};
    
    /* Multi-dimensional array constructor */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
}

/* ==================== TREE_VEC ==================== */
/* Using GCC statement expressions (GNU extension) */
#ifdef __GNUC__
int tree_vec_test(void) {
    /* Statement expression creates TREE_VEC */
    int result = ({
        int a = 5;
        int b = 10;
        int c = a + b;
        c * 2;
    });
    
    /* Another example with multiple values */
    int x = ({
        int tmp1 = 1;
        int tmp2 = 2;
        int tmp3 = 3;
        (tmp1 + tmp2) * tmp3;
    });
    
    return result + x;
}
#endif

/* ==================== OMP_CLAUSE ==================== */
#ifdef _OPENMP
void omp_test(int n) {
    int i;
    int sum = 0;
    
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static)
    for (i = 0; i < n; i++) {
        sum += i * i;
    }
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("OpenMP thread count may vary\n");
        }
        
        #pragma omp for nowait
        for (i = 0; i < 10; i++) {
            /* Some computation */
        }
    }
}
#endif

/* ==================== C++ Specific: TREE_BINFO ==================== */
#ifdef __cplusplus

class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual void method() = 0;
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual void method() override {
        base_data = 42;
    }
    int derived_data;
};

class AnotherDerived : public BaseClass {
public:
    virtual void method() override {
        base_data = 24;
    }
};

/* Multiple inheritance for more BINFO complexity */
class Base1 {
public:
    virtual void f1() {}
    int b1;
};

class Base2 {
public:
    virtual void f2() {}
    int b2;
};

class MultiDerived : public Base1, public Base2 {
public:
    virtual void f1() override {}
    virtual void f2() override {}
    int md;
};

void cpp_binfo_test(void) {
    DerivedClass d;
    AnotherDerived ad;
    MultiDerived md;
    
    BaseClass* bp1 = &d;
    BaseClass* bp2 = &ad;
    
    bp1->method();
    bp2->method();
    
    Base1* b1p = &md;
    Base2* b2p = &md;
    
    b1p->f1();
    b2p->f2();
}

#endif /* __cplusplus */

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char** argv) {
    int n = 100;
    
    /* Ensure all test functions are called */
    int ssa_result = ssa_test_function(n);
    printf("SSA test result: %d\n", ssa_result);
    
    block_test_function();
    
    constructor_test();
    
    #ifdef __GNUC__
    int vec_result = tree_vec_test();
    printf("TREE_VEC test result: %d\n", vec_result);
    #endif
    
    #ifdef _OPENMP
    omp_test(1000);
    printf("OpenMP test completed\n");
    #endif
    
    #ifdef __cplusplus
    cpp_binfo_test();
    printf("C++ BINFO test completed\n");
    #endif
    
    /* Use the identifier */
    some_unique_identifier_123 = 42;
    another_identifier = ssa_result;
    
    printf("All tests completed successfully\n");
    return 0;
}
