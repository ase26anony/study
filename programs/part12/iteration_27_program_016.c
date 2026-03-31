/* test_tree_kind_coverage.c - Comprehensive test to trigger all tree_kind cases */
#include <stdio.h>
#include <stdlib.h>

/* Enable OpenMP if available */
#ifdef _OPENMP
#include <omp.h>
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Variable and function names create IDENTIFIER_NODE */
int global_unique_identifier_123;
static int static_identifier_456;

void function_with_identifiers(void) {
    int local_identifier_789 = 42;
    global_unique_identifier_123 = local_identifier_789 + static_identifier_456;
}

/* ==================== SSA_NAME ==================== */
/* Complex operations that force SSA form */
int ssa_test_function(int n) {
    int a = 0, b = 1, c;
    
    /* Loop with arithmetic to create SSA names */
    for (int i = 0; i < n; ++i) {
        a = a + i;          /* Creates SSA_NAME for 'a' */
        b = b * 2;          /* Creates SSA_NAME for 'b' */
        c = a + b;          /* Creates SSA_NAME for 'c' */
        
        /* Conditional to create phi nodes */
        if (c > 100) {
            a = c / 2;
        } else {
            a = c * 2;
        }
    }
    
    /* More complex SSA with nested loops */
    int sum = 0;
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            sum += i * j;
            if (sum > 50) {
                sum = sum % 50;  /* Creates more SSA names */
            }
        }
    }
    
    return a + b + c + sum;
}

/* ==================== BLOCK ==================== */
/* Nested blocks create BLOCK nodes */
void block_test_function(void) {
    /* Outer block */
    int outer = 1;
    
    {
        /* Inner block 1 */
        int inner1 = 2;
        outer += inner1;
        
        {
            /* Deeper nested block */
            int inner2 = 3;
            inner1 += inner2;
            
            {
                /* Even deeper */
                int inner3 = 4;
                inner2 += inner3;
            }
        }
    }
    
    /* Another block with different scope */
    {
        int x = 10;
        int y = 20;
        int z = x + y;
        (void)z;  /* Use to avoid warnings */
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
    int i;
    float f;
    double d;
};

void constructor_test(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor (C99 designated initializers) */
    struct my_struct s1 = {.a = 10, .b = 3.14f, .c = 'X'};
    
    /* Nested struct with constructor */
    struct nested {
        struct my_struct inner;
        int extra;
    } n1 = {{5, 2.718f, 'Y'}, 100};
    
    /* Union constructor */
    union my_union u1 = {.i = 42};
    
    /* Multi-dimensional array constructor */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    
    /* Use variables to avoid optimization */
    (void)arr[0];
    (void)s1.a;
    (void)n1.extra;
    (void)u1.i;
    (void)matrix[0][0];
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
void omp_test_function(int size) {
    int i;
    int *array = malloc(size * sizeof(int));
    
    if (!array) return;
    
#ifdef _OPENMP
    /* Various OpenMP clauses to generate different OMP_CLAUSE nodes */
    #pragma omp parallel for private(i) shared(array) schedule(static) num_threads(4)
    for (i = 0; i < size; ++i) {
        array[i] = i * 2;
    }
    
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < size; ++i) {
        sum += array[i];
    }
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("OpenMP thread count: %d\n", omp_get_num_threads());
        }
    }
#endif
    
    free(array);
}

/* ==================== TREE_VEC ==================== */
/* GCC extensions for TREE_VEC */
#ifdef __GNUC__
void tree_vec_test(void) {
    /* Using statement expressions - a GCC extension that can create TREE_VEC */
    int result = ({
        int x = 5;
        int y = 10;
        int z;
        z = x + y;
        z * 2;
    });
    
    /* Another GCC extension: vector types */
    typedef int v4si __attribute__ ((vector_size (16)));
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    
    /* Using __builtin_choose_expr which may create TREE_VEC */
    int chosen = __builtin_choose_expr(sizeof(int) == 4, 42, 24);
    
    (void)result;
    (void)c[0];
    (void)chosen;
}
#endif

/* ==================== C++ Specific: TREE_BINFO ==================== */
#ifdef __cplusplus
/* C++ code for TREE_BINFO (base class information) */
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

class MultipleBase1 {
public:
    virtual ~MultipleBase1() {}
    int data1;
};

class MultipleBase2 {
public:
    virtual ~MultipleBase2() {}
    int data2;
};

class MultipleDerived : public MultipleBase1, public MultipleBase2 {
public:
    int derived_data;
};

void cpp_binfo_test(void) {
    DerivedClass d;
    BaseClass* b = &d;
    b->base_method();
    
    MultipleDerived md;
    MultipleBase1* mb1 = &md;
    MultipleBase2* mb2 = &md;
    
    (void)mb1;
    (void)mb2;
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char **argv) {
    /* Call all test functions to ensure code generation */
    function_with_identifiers();
    
    int ssa_result = ssa_test_function(20);
    printf("SSA test result: %d\n", ssa_result);
    
    block_test_function();
    constructor_test();
    
    omp_test_function(100);
    
#ifdef __GNUC__
    tree_vec_test();
#endif
    
#ifdef __cplusplus
    cpp_binfo_test();
#endif
    
    /* Complex control flow to engage more compiler passes */
    int total = 0;
    for (int i = 0; i < 1000; ++i) {
        if (i % 3 == 0) {
            total += i * 2;
        } else if (i % 3 == 1) {
            total += i / 2;
        } else {
            total -= i;
        }
    }
    
    printf("Final total: %d\n", total);
    return 0;
}
