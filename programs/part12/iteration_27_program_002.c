/* test_tree_kind.c - Comprehensive test for GCC tree kind coverage */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 test_tree_kind.c -o test_tree_kind */
/* For C++ mode: g++ -O2 -fopenmp -std=gnu++11 test_tree_kind.c -o test_tree_kind_cpp */

#include <stdio.h>
#include <stdlib.h>

/* ===== IDENTIFIER_NODE ===== */
/* Variable and function names create IDENTIFIER_NODE */
int unique_identifier_1;
void identifier_node_function(void) {
    int local_identifier = 42;
    unique_identifier_1 = local_identifier;
}

/* ===== SSA_NAME ===== */
/* Complex arithmetic with loops forces SSA form */
int ssa_name_test(int n) {
    int a = 0, b = 1, c = 2;
    for (int i = 0; i < n; ++i) {
        a = a + i;      /* Creates phi nodes in SSA */
        b = b * a;
        c = c - b;
        if (c > 100) {
            a = c / 2;  /* Creates additional SSA names */
        }
    }
    return a + b + c;
}

/* ===== BLOCK ===== */
/* Nested blocks with local variables */
void block_test(void) {
    int outer = 10;
    {
        int inner = 20;
        {
            int innermost = 30;
            outer = inner + innermost;
        }
        /* Another block with different scope */
        {
            int temp = 40;
            outer += temp;
        }
    }
}

/* ===== CONSTRUCTOR ===== */
/* Aggregate initializers */
struct constructor_struct {
    int x;
    double y;
    char z[4];
};

void constructor_test(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor with designated initializer */
    struct constructor_struct s1 = {.x = 10, .y = 3.14, .z = "abc"};
    
    /* Nested struct constructor */
    struct nested {
        struct constructor_struct inner;
        int extra;
    } n1 = {{5, 2.71, "def"}, 100};
    
    /* Complex array constructor */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
}

/* ===== OMP_CLAUSE ===== */
/* OpenMP pragmas generate OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_clause_test(int size) {
    int i;
    int sum = 0;
    int private_var = 0;
    
    #pragma omp parallel for private(i) reduction(+:sum) firstprivate(private_var)
    for (i = 0; i < size; i++) {
        sum += i;
    }
    
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            i = 1;
        }
        #pragma omp section
        {
            i = 2;
        }
    }
    
    #pragma omp task shared(sum) if(size > 100)
    {
        sum *= 2;
    }
}
#else
void omp_clause_test(int size) {
    /* Dummy implementation when OpenMP not available */
    printf("OpenMP not enabled\n");
}
#endif

/* ===== TREE_VEC ===== */
/* GCC extensions that create TREE_VEC nodes */
#ifdef __GNUC__
void tree_vec_test(void) {
    /* Using statement expression - may create TREE_VEC */
    int x = ({ 
        int a = 5; 
        int b = 10; 
        a + b; 
    });
    
    /* Vector types extension */
    typedef int v4si __attribute__ ((vector_size (16)));
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
    
    /* Another GCC extension: type attributes */
    struct __attribute__((packed)) packed_struct {
        char a;
        int b;
    } ps;
}
#else
void tree_vec_test(void) {
    /* Non-GCC compilers won't create TREE_VEC */
}
#endif

/* ===== C++ Specific: TREE_BINFO ===== */
#ifdef __cplusplus

/* Base class for BINFO testing */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual void base_method() { printf("Base\n"); }
    int base_data;
};

/* Another base for multiple inheritance */
class AnotherBase {
public:
    virtual ~AnotherBase() {}
    virtual void another_method() { printf("Another\n"); }
    int another_data;
};

/* Derived class with inheritance hierarchy */
class DerivedClass : public BaseClass, public AnotherBase {
public:
    virtual void base_method() override { printf("Derived\n"); }
    virtual void another_method() override { printf("Derived Another\n"); }
    int derived_data;
};

/* More complex hierarchy */
class DeepDerived : public DerivedClass {
public:
    virtual void base_method() override { printf("DeepDerived\n"); }
    int deep_data;
};

void tree_binfo_test(void) {
    DerivedClass d;
    BaseClass* b = &d;
    AnotherBase* a = &d;
    
    b->base_method();    /* Virtual call through base pointer */
    a->another_method(); /* Virtual call through another base */
    
    DeepDerived dd;
    DerivedClass* dc = &dd;
    dc->base_method();   /* Virtual call through intermediate base */
}

#else
/* C version - no BINFO nodes */
void tree_binfo_test(void) {
    printf("C++ required for BINFO nodes\n");
}
#endif

/* ===== Main driver ===== */
int main(int argc, char** argv) {
    /* Ensure all constructs are referenced/used */
    identifier_node_function();
    
    int result = ssa_name_test(100);
    printf("SSA test result: %d\n", result);
    
    block_test();
    constructor_test();
    
    tree_vec_test();
    tree_binfo_test();
    
    omp_clause_test(1000);
    
    /* Complex control flow to engage more compiler passes */
    if (result > 50) {
        for (int i = 0; i < 10; i++) {
            result += ssa_name_test(i * 10);
        }
    } else {
        int temp = 0;
        while (temp < 100) {
            temp += ssa_name_test(temp);
        }
        result = temp;
    }
    
    printf("Final result: %d\n", result);
    return 0;
}
