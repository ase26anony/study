/* test_tree_kind.c - Comprehensive test to trigger all tree_kind cases */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 test_tree_kind.c -o test_tree_kind */
/* For C++ mode: g++ -O2 -fopenmp -std=gnu++11 test_tree_kind.c -o test_tree_kind_cpp */

#include <stdio.h>
#include <stdlib.h>

/* ========== IDENTIFIER_NODE ========== */
/* Variable and function names create IDENTIFIER_NODE */
int unique_identifier_1;
void function_with_identifiers(void) {
    int local_identifier = 42;
    unique_identifier_1 = local_identifier;
}

/* ========== SSA_NAME ========== */
/* Complex arithmetic and loops force SSA form */
int ssa_test_function(int n) {
    int a = 0, b = 1, c;
    
    /* Loop with multiple assignments to create SSA_NAME nodes */
    for (int i = 0; i < n; ++i) {
        a = a + i;      /* Creates SSA_NAME for 'a' */
        b = b * 2;      /* Creates SSA_NAME for 'b' */
        c = a + b;      /* Creates SSA_NAME for 'c' */
        
        /* Conditional to create phi nodes */
        if (c > 100) {
            a = c / 2;
        } else {
            a = c * 2;
        }
    }
    
    /* Another loop with induction variable */
    int sum = 0;
    for (int j = 0; j < n; ++j) {
        sum += j * j;
        if (sum > 1000) {
            sum = sum % 1000;
        }
    }
    
    return a + b + c + sum;
}

/* ========== BLOCK ========== */
/* Nested blocks create BLOCK nodes */
void block_test_function(void) {
    /* Outer block */
    int x = 10;
    
    {
        /* Inner block 1 - creates BLOCK node */
        int y = 20;
        x += y;
        
        {
            /* Deeper nested block */
            int z = 30;
            x += z;
        }
    }
    
    {
        /* Another inner block */
        float f = 3.14;
        x += (int)f;
    }
}

/* ========== CONSTRUCTOR ========== */
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
    struct my_struct s1 = {.a = 10, .b = 2.5, .c = 'X'};
    
    /* Nested struct constructor */
    struct nested {
        struct my_struct inner;
        int extra;
    } n1 = {{5, 1.5, 'Y'}, 100};
    
    /* Multi-dimensional array constructor */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
}

/* ========== TREE_VEC ========== */
/* Using GCC statement expressions to create TREE_VEC nodes */
#ifdef __GNUC__
void tree_vec_test(void) {
    /* GCC statement expression with multiple elements */
    int result = ({
        int a = 5;
        int b = 10;
        int c = a + b;
        c * 2;  /* Last expression is result */
    });
    
    /* Another statement expression */
    int vec_result = ({
        int x = 1, y = 2, z = 3;
        int sum = 0;
        for (int i = 0; i < 3; i++) {
            sum += i;
        }
        sum + x + y + z;
    });
    
    printf("Tree vec results: %d, %d\n", result, vec_result);
}
#endif

/* ========== OMP_CLAUSE ========== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    
    #pragma omp parallel for private(i) reduction(+:sum) firstprivate(private_var)
    for (i = 0; i < n; i++) {
        sum += i * i;
    }
    
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int j = 0; j < n; j++) {
            /* Do some work */
            sum += j;
        }
        
        #pragma omp single
        {
            printf("Thread %d executing single\n", omp_get_thread_num());
        }
    }
    
    printf("OMP sum: %d\n", sum);
}
#endif

/* ========== C++ Specific: TREE_BINFO ========== */
#ifdef __cplusplus
/* Class hierarchies create TREE_BINFO nodes */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual void base_method() { printf("Base method\n"); }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual void base_method() override { printf("Derived method\n"); }
    int derived_data;
};

class AnotherDerived : public BaseClass {
public:
    virtual void base_method() override { printf("Another derived\n"); }
    int more_data;
};

/* Multiple inheritance for more complex binfo */
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

class MultipleDerived : public Base1, public Base2 {
public:
    virtual void f1() override {}
    virtual void f2() override {}
    int md;
};

void cpp_binfo_test(void) {
    DerivedClass d;
    BaseClass* b = &d;
    b->base_method();
    
    AnotherDerived ad;
    BaseClass* b2 = &ad;
    b2->base_method();
    
    MultipleDerived md;
    Base1* b1 = &md;
    Base2* b2_ptr = &md;
}
#endif

/* ========== Main driver ========== */
int main(int argc, char** argv) {
    int n = 100;
    
    /* Trigger all test functions */
    function_with_identifiers();
    
    int ssa_result = ssa_test_function(n);
    printf("SSA test result: %d\n", ssa_result);
    
    block_test_function();
    constructor_test();
    
    #ifdef __GNUC__
    tree_vec_test();
    #endif
    
    #ifdef _OPENMP
    omp_test(n);
    #endif
    
    #ifdef __cplusplus
    cpp_binfo_test();
    #endif
    
    /* Complex control flow to engage middle-end passes */
    int final_result = 0;
    for (int i = 0; i < n; ++i) {
        if (i % 3 == 0) {
            final_result += i * 2;
        } else if (i % 3 == 1) {
            final_result += i * 3;
        } else {
            final_result += i;
        }
        
        /* Switch statement for more IR variety */
        switch (i % 4) {
            case 0: final_result += 1; break;
            case 1: final_result += 2; break;
            case 2: final_result += 3; break;
            case 3: final_result += 4; break;
        }
    }
    
    printf("Final result: %d\n", final_result);
    return 0;
}
