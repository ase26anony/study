/* test_tree_kind.c - Comprehensive test for GCC tree.cc get_kind function */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 test_tree_kind.c -o test_tree_kind */
/* For C++ specific parts: g++ -O2 -fopenmp -std=gnu++11 test_tree_kind.c -o test_tree_kind_cpp */

#include <stdio.h>
#include <stdlib.h>

/* ========== IDENTIFIER_NODE ========== */
/* Variable and function names create IDENTIFIER_NODE */
int global_identifier_test = 42;
void function_with_identifiers(void) {
    int local_identifier = global_identifier_test;
    printf("Identifier test: %d\n", local_identifier);
}

/* ========== TREE_VEC ========== */
/* GCC statement expressions can create TREE_VEC nodes */
int tree_vec_example(void) {
    /* Using GCC statement expression extension */
    int result = ({ 
        int a = 5; 
        int b = 10; 
        a + b; 
    });
    return result;
}

/* ========== SSA_NAME ========== */
/* Complex arithmetic with loops creates SSA_NAME nodes */
int ssa_name_generator(int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        sum = sum + i * 2;  /* This creates SSA_NAME nodes during optimization */
    }
    
    /* More complex SSA patterns */
    int x = sum;
    int y = x * 2;
    int z = y + x;
    
    for (int j = 0; j < 100; ++j) {
        x = x + j;
        y = y - j;
        z = z * (j + 1);
    }
    
    return x + y + z;
}

/* ========== BLOCK ========== */
/* Nested blocks create BLOCK nodes */
void block_node_example(void) {
    /* Outer block */
    int outer = 10;
    {
        /* Inner block 1 */
        int inner1 = outer * 2;
        {
            /* Inner block 2 */
            int inner2 = inner1 + 5;
            printf("Block test: %d\n", inner2);
        }
    }
    
    /* Another block with different scope */
    {
        int a = 1;
        {
            int b = 2;
            {
                int c = 3;
                printf("Nested blocks: %d %d %d\n", a, b, c);
            }
        }
    }
}

/* ========== CONSTRUCTOR ========== */
/* Aggregate initializers create CONSTRUCTOR nodes */
struct test_struct {
    int a;
    float b;
    char c;
};

int constructor_example(void) {
    /* Array constructor */
    int array_constructor[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor */
    struct test_struct s = {.a = 10, .b = 3.14f, .c = 'X'};
    
    /* Nested struct constructor */
    struct nested {
        struct test_struct inner;
        int extra;
    } n = {{5, 2.71f, 'Y'}, 100};
    
    return array_constructor[0] + s.a + n.inner.a;
}

/* ========== OMP_CLAUSE ========== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_clause_example(void) {
    int i;
    int sum = 0;
    int array[100];
    
    for (i = 0; i < 100; i++) {
        array[i] = i;
    }
    
    /* Multiple OpenMP clauses to generate various OMP_CLAUSE nodes */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static, 10)
    for (i = 0; i < 100; i++) {
        sum += array[i];
    }
    
    printf("OpenMP sum: %d\n", sum);
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel sections private(i) num_threads(4)
    {
        #pragma omp section
        {
            printf("Section 1\n");
        }
        #pragma omp section
        {
            printf("Section 2\n");
        }
    }
}
#else
void omp_clause_example(void) {
    printf("OpenMP not enabled\n");
}
#endif

/* ========== C++ Specific Code for TREE_BINFO ========== */
#ifdef __cplusplus

/* Base class for BINFO generation */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual void base_method() { printf("Base method\n"); }
    int base_data;
};

/* Derived class with inheritance creates TREE_BINFO nodes */
class DerivedClass : public BaseClass {
public:
    virtual void base_method() override { printf("Derived method\n"); }
    int derived_data;
};

/* Multiple inheritance for more complex BINFO structures */
class AnotherBase {
public:
    virtual ~AnotherBase() {}
    virtual void another_method() = 0;
};

class MultipleDerived : public BaseClass, public AnotherBase {
public:
    virtual void base_method() override { printf("MultipleDerived base\n"); }
    virtual void another_method() override { printf("MultipleDerived another\n"); }
};

void tree_binfo_example(void) {
    DerivedClass d;
    d.base_method();
    
    MultipleDerived md;
    md.base_method();
    md.another_method();
    
    BaseClass* bp = &d;
    bp->base_method();
}

#else
/* C version - TREE_BINFO requires C++ */
void tree_binfo_example(void) {
    printf("C++ required for TREE_BINFO nodes\n");
}
#endif

/* ========== Main function to trigger all cases ========== */
int main(void) {
    printf("=== Testing GCC tree.cc get_kind function ===\n");
    
    /* Trigger IDENTIFIER_NODE */
    function_with_identifiers();
    
    /* Trigger TREE_VEC */
    int vec_result = tree_vec_example();
    printf("TREE_VEC result: %d\n", vec_result);
    
    /* Trigger SSA_NAME */
    int ssa_result = ssa_name_generator(50);
    printf("SSA_NAME result: %d\n", ssa_result);
    
    /* Trigger BLOCK */
    block_node_example();
    
    /* Trigger CONSTRUCTOR */
    int constr_result = constructor_example();
    printf("CONSTRUCTOR result: %d\n", constr_result);
    
    /* Trigger OMP_CLAUSE */
    omp_clause_example();
    
    /* Trigger TREE_BINFO (C++ only) */
    tree_binfo_example();
    
    /* Complex mixed usage to ensure middle-end processing */
    int final = 0;
    for (int i = 0; i < 10; i++) {
        int temp[3] = {i, i*2, i*3};  /* CONSTRUCTOR */
        {
            /* BLOCK */
            int block_var = temp[0];
            final += block_var + ssa_name_generator(5);
        }
    }
    
    printf("Final result: %d\n", final);
    printf("=== Test complete ===\n");
    
    return 0;
}
