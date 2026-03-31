/* test_tree_kind.c - Comprehensive test for GCC tree kind coverage */

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Variable and function names create IDENTIFIER_NODE */
int global_identifier = 42;
void function_identifier(void) {
    int local_identifier = global_identifier;
}

/* ==================== TREE_VEC ==================== */
/* Using GCC statement expressions to create TREE_VEC */
#ifdef __GNUC__
int tree_vec_example(void) {
    /* Statement expression with multiple elements */
    int result = ({ 
        int a = 1; 
        int b = 2; 
        int c = 3; 
        a + b + c; 
    });
    return result;
}
#endif

/* ==================== SSA_NAME ==================== */
/* Complex arithmetic to force SSA form */
int ssa_name_example(int n) {
    int x = 0;
    int y = 1;
    
    /* Loop with phi nodes in SSA */
    for (int i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            x = x + y;
        } else {
            x = x - y;
        }
        y = y * 2;
    }
    
    /* Complex expression with multiple assignments */
    int a = x;
    int b = y;
    for (int j = 0; j < 10; ++j) {
        a = a + b;
        b = b - a;
        int temp = a;
        a = b;
        b = temp;
    }
    
    return a + b;
}

/* ==================== BLOCK ==================== */
/* Nested blocks create BLOCK nodes */
void block_example(void) {
    /* Outer block */
    int outer = 1;
    {
        /* Inner block 1 */
        int inner1 = 2;
        {
            /* Inner block 2 */
            int inner2 = 3;
            outer = inner1 + inner2;
        }
        {
            /* Another inner block */
            int inner3 = 4;
            outer += inner3;
        }
    }
    
    /* Switch with blocks */
    switch (outer) {
        case 1: {
            int case1_var = 10;
            outer = case1_var;
            break;
        }
        case 2: {
            int case2_var = 20;
            outer = case2_var;
            break;
        }
        default: {
            int default_var = 30;
            outer = default_var;
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

void constructor_example(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor */
    struct my_struct s1 = {10, 3.14f, 'A'};
    
    /* Designated initializers */
    struct my_struct s2 = {.a = 20, .b = 2.71f, .c = 'B'};
    
    /* Nested initializers */
    struct nested {
        struct my_struct inner;
        int extra;
    } n1 = {{15, 1.23f, 'C'}, 100};
    
    /* Union constructor */
    union my_union u1 = {.x = 42};
    
    /* Zero initializer */
    struct my_struct s3 = {0};
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_clause_example(void) {
    int i;
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Multiple OpenMP clauses */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static, 10)
    for (i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* Another with different clauses */
    #pragma omp parallel sections private(i) shared(sum)
    {
        #pragma omp section
        {
            for (i = 0; i < 50; i++) {
                #pragma omp atomic
                sum++;
            }
        }
        #pragma omp section
        {
            for (i = 50; i < 100; i++) {
                #pragma omp atomic
                sum--;
            }
        }
    }
    
    /* Single directive with clause */
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            sum = sum * 2;
        }
    }
}
#endif

#ifdef __cplusplus
/* ==================== TREE_BINFO (C++ only) ==================== */
/* Class hierarchy creates TREE_BINFO nodes */
class BaseClass {
public:
    virtual void base_method() {
        int x = 0;
    }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
public:
    virtual void base_method() override {
        int y = 1;
    }
};

class MultipleBase1 {
public:
    virtual void method1() {}
};

class MultipleBase2 {
public:
    virtual void method2() {}
};

class MultipleDerived : public MultipleBase1, public MultipleBase2 {
public:
    virtual void method1() override {}
    virtual void method2() override {}
};

void tree_binfo_example(void) {
    DerivedClass d;
    BaseClass* b = &d;
    b->base_method();
    
    MultipleDerived md;
    MultipleBase1* mb1 = &md;
    MultipleBase2* mb2 = &md;
    mb1->method1();
    mb2->method2();
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    /* Reference all examples to ensure they're compiled */
    function_identifier();
    
    #ifdef __GNUC__
    int vec_result = tree_vec_example();
    #endif
    
    int ssa_result = ssa_name_example(20);
    block_example();
    constructor_example();
    
    #ifdef _OPENMP
    omp_clause_example();
    #endif
    
    #ifdef __cplusplus
    tree_binfo_example();
    #endif
    
    /* Print something to avoid dead code elimination */
    #ifdef __cplusplus
    cout << "Test completed: " << ssa_result << endl;
    #else
    printf("Test completed: %d\n", ssa_result);
    #endif
    
    return 0;
}
