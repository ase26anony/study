/* test_tree_kind.c - Coverage test for GCC's get_kind function */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 test_tree_kind.c -o test_program */
/* For C++ features: g++ -O2 -fopenmp -std=gnu++11 test_tree_kind.c -o test_cpp_program */

#include <stdio.h>
#include <stdlib.h>

/* ===== IDENTIFIER_NODE ===== */
/* Variable and function names create IDENTIFIER_NODE */
int unique_identifier_1;
void function_with_identifier(void) {
    int local_identifier = 42;
    unique_identifier_1 = local_identifier;
}

/* ===== SSA_NAME ===== */
/* Complex arithmetic with loops forces SSA form */
int ssa_test_function(int n) {
    int a = 0, b = 1, c;
    
    /* Loop creates phi nodes and SSA names */
    for (int i = 0; i < n; ++i) {
        c = a + b;      /* Creates SSA_NAME for c */
        a = b;          /* Creates SSA_NAME for a */
        b = c;          /* Creates SSA_NAME for b */
        
        /* Conditional creates more SSA complexity */
        if (c % 2 == 0) {
            a = a + i;  /* Another SSA_NAME for a */
        }
    }
    
    /* Multiple return paths create SSA phi nodes */
    if (n > 0) {
        return a;
    } else {
        return b;
    }
}

/* ===== BLOCK ===== */
/* Nested blocks with local variables */
void block_test_function(void) {
    int outer = 10;
    
    { /* BLOCK 1 */
        int inner1 = outer * 2;
        
        { /* BLOCK 2 (nested) */
            int inner2 = inner1 + 5;
            outer = inner2;
            
            { /* BLOCK 3 (deeply nested) */
                int inner3 = inner2 * 3;
                printf("Deep block: %d\n", inner3);
            }
        }
        
        /* Another sibling block */
        {
            int sibling = 99;
            outer += sibling;
        }
    }
}

/* ===== CONSTRUCTOR ===== */
/* Aggregate initializers */
struct my_struct {
    int x;
    double y;
    char z[10];
};

void constructor_test(void) {
    /* Array constructor */
    int array_constructor[5] = {10, 20, 30, 40, 50};
    
    /* Struct constructor */
    struct my_struct s1 = {.x = 42, .y = 3.14, .z = "hello"};
    
    /* Nested struct constructor */
    struct nested {
        struct my_struct inner;
        int extra;
    } n1 = {{100, 2.718, "world"}, 999};
    
    /* Zero initializer (also a constructor) */
    struct my_struct s2 = {0};
}

/* ===== TREE_VEC ===== */
/* Using GCC statement expressions (GNU extension) */
#ifdef __GNUC__
void tree_vec_test(void) {
    /* Statement expression creates TREE_VEC */
    int result = ({
        int a = 5;
        int b = 10;
        int c = a * b;
        c + 2;
    });
    
    printf("Statement expression result: %d\n", result);
    
    /* Another TREE_VEC example with multiple statements */
    int x = ({
        int temp = 0;
        for (int i = 0; i < 5; i++) {
            temp += i * i;
        }
        temp;
    });
}
#endif

/* ===== OMP_CLAUSE ===== */
#ifdef _OPENMP
void omp_test_function(int size) {
    int i;
    int *array = malloc(size * sizeof(int));
    
    if (!array) return;
    
    /* OpenMP parallel for with clauses */
    #pragma omp parallel for private(i) shared(array) schedule(static)
    for (i = 0; i < size; i++) {
        array[i] = i * i;
    }
    
    /* Another OpenMP construct with reduction clause */
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < size; i++) {
        sum += array[i];
    }
    
    printf("OpenMP sum: %d\n", sum);
    
    free(array);
}
#endif

/* ===== C++ Specific Code for TREE_BINFO ===== */
#ifdef __cplusplus

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

void cpp_binfo_test(void) {
    DerivedClass d;
    AnotherDerived ad;
    BaseClass* ptr1 = &d;
    BaseClass* ptr2 = &ad;
    
    ptr1->base_method();
    ptr2->base_method();
    
    /* Multiple inheritance would create more BINFO nodes */
    class MultiBase1 { public: virtual void m1() {} };
    class MultiBase2 { public: virtual void m2() {} };
    class MultiDerived : public MultiBase1, public MultiBase2 {};
    
    MultiDerived md;
    MultiBase1* mb1 = &md;
    MultiBase2* mb2 = &md;
}

#else
/* C version - TREE_BINFO won't be generated in C mode */
void cpp_binfo_test(void) {
    printf("C++ features not available in C mode\n");
}
#endif

/* ===== Main function to trigger all tests ===== */
int main(int argc, char **argv) {
    printf("Testing GCC tree node coverage...\n");
    
    /* Trigger IDENTIFIER_NODE creation */
    function_with_identifier();
    
    /* Trigger SSA_NAME creation */
    int fib_result = ssa_test_function(20);
    printf("SSA test result (Fibonacci-ish): %d\n", fib_result);
    
    /* Trigger BLOCK creation */
    block_test_function();
    
    /* Trigger CONSTRUCTOR creation */
    constructor_test();
    
    /* Trigger TREE_VEC creation (if GCC extensions available) */
    #ifdef __GNUC__
    tree_vec_test();
    #endif
    
    /* Trigger OMP_CLAUSE creation (if OpenMP available) */
    #ifdef _OPENMP
    omp_test_function(100);
    #else
    printf("OpenMP not enabled\n");
    #endif
    
    /* Trigger TREE_BINFO creation (in C++ mode) */
    cpp_binfo_test();
    
    /* Complex expression to ensure middle-end processing */
    int final_result = 0;
    for (int i = 0; i < 1000; i++) {
        if (i % 3 == 0) {
            final_result += i * 2;
        } else if (i % 3 == 1) {
            final_result -= i;
        } else {
            final_result *= (i % 10) + 1;
        }
    }
    
    printf("Final computation: %d\n", final_result);
    
    return 0;
}
