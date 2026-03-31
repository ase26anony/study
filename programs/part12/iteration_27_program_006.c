/* test_tree_kind.c - Comprehensive test for GCC tree kind coverage */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 test_tree_kind.c -o test_tree_kind */
/* For C++ features: g++ -O2 -fopenmp -std=gnu++11 test_tree_kind.c -o test_tree_kind_cpp */

#include <stdio.h>
#include <stdlib.h>

/* ==================== IDENTIFIER_NODE ==================== */
/* Variable and function names create IDENTIFIER_NODE */
int global_identifier = 42;
void function_identifier(void) {
    int local_identifier = global_identifier;
    printf("Identifier test: %d\n", local_identifier);
}

/* ==================== TREE_VEC ==================== */
/* Using GCC statement expressions to create TREE_VEC nodes */
#ifdef __GNUC__
#define CREATE_VEC() ({ \
    int a = 1, b = 2, c = 3; \
    (typeof(a))((a + b) * c); \
})
#else
#define CREATE_VEC() (0)
#endif

int test_tree_vec(void) {
    /* This should generate TREE_VEC in GCC's internal representation */
    int result = CREATE_VEC();
    return result;
}

/* ==================== SSA_NAME ==================== */
/* Complex arithmetic to force SSA form */
int test_ssa_name(int n) {
    int x = 0;
    int y = 1;
    
    /* Loop with arithmetic to create SSA_NAME nodes */
    for (int i = 0; i < n; ++i) {
        x = x + y * i;      /* Creates phi nodes in SSA */
        y = y + x;          /* More SSA complexity */
        if (i % 2 == 0) {
            x = x - 1;      /* Conditional assignment creates more SSA */
        }
    }
    
    /* Additional SSA complexity */
    int z = x;
    for (int j = 0; j < 10; ++j) {
        z = z * 2 + j;
    }
    
    return x + y + z;
}

/* ==================== BLOCK ==================== */
/* Nested blocks create BLOCK nodes */
int test_block(int depth) {
    int result = 0;
    
    /* Outer block */
    {
        int a = depth;
        
        /* First nested block */
        {
            int b = a * 2;
            
            /* Second nested block with its own scope */
            {
                int c = b + 1;
                result = c;
                
                /* Third nested block */
                {
                    int d = c * 3;
                    result += d;
                }
            }
        }
        
        /* Another block at same level */
        {
            int e = 100;
            result -= e;
        }
    }
    
    return result;
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers create CONSTRUCTOR nodes */
struct test_struct {
    int a;
    float b;
    char c[10];
};

int test_constructor(void) {
    /* Array constructor */
    int array[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor */
    struct test_struct s1 = {10, 3.14f, "hello"};
    
    /* Designated initializer */
    struct test_struct s2 = {.a = 20, .b = 2.71f, .c = "world"};
    
    /* Nested initializer */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    
    return array[0] + s1.a + s2.a + matrix[0][0];
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
#include <omp.h>

int test_omp_clause(int size) {
    int sum = 0;
    int i;
    
    /* Multiple OpenMP clauses to generate various OMP_CLAUSE nodes */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static) num_threads(4)
    for (i = 0; i < size; i++) {
        sum += i * i;
    }
    
    /* Another OpenMP construct with different clauses */
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
    
    return sum;
}
#else
int test_omp_clause(int size) {
    /* Fallback without OpenMP */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += i * i;
    }
    return sum;
}
#endif

/* ==================== C++ Specific: TREE_BINFO ==================== */
#ifdef __cplusplus

/* Class hierarchy to generate TREE_BINFO (base info) nodes */
class BaseClass1 {
public:
    virtual void method1() { printf("Base1\n"); }
    virtual ~BaseClass1() {}
    int base_data1;
};

class BaseClass2 {
public:
    virtual void method2() { printf("Base2\n"); }
    virtual ~BaseClass2() {}
    int base_data2;
};

class DerivedClass : public BaseClass1, public BaseClass2 {
public:
    virtual void method1() override { printf("Derived::method1\n"); }
    virtual void method2() override { printf("Derived::method2\n"); }
    int derived_data;
};

void test_tree_binfo(void) {
    DerivedClass obj;
    BaseClass1* ptr1 = &obj;
    BaseClass2* ptr2 = &obj;
    
    ptr1->method1();  /* Virtual call through base pointer */
    ptr2->method2();  /* Virtual call through second base pointer */
    
    /* Multiple inheritance creates BINFO nodes for base class info */
    DerivedClass* dptr = dynamic_cast<DerivedClass*>(ptr1);
    if (dptr) {
        dptr->derived_data = 42;
    }
}

#else
/* C version - empty stub */
void test_tree_binfo(void) {
    printf("C++ features not available in C mode\n");
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char** argv) {
    printf("Testing GCC tree kind coverage...\n");
    
    /* Trigger IDENTIFIER_NODE */
    function_identifier();
    
    /* Trigger TREE_VEC */
    int vec_result = test_tree_vec();
    printf("TREE_VEC test result: %d\n", vec_result);
    
    /* Trigger SSA_NAME */
    int ssa_result = test_ssa_name(100);
    printf("SSA_NAME test result: %d\n", ssa_result);
    
    /* Trigger BLOCK */
    int block_result = test_block(5);
    printf("BLOCK test result: %d\n", block_result);
    
    /* Trigger CONSTRUCTOR */
    int constr_result = test_constructor();
    printf("CONSTRUCTOR test result: %d\n", constr_result);
    
    /* Trigger OMP_CLAUSE */
    int omp_result = test_omp_clause(1000);
    printf("OMP_CLAUSE test result: %d\n", omp_result);
    
    /* Trigger TREE_BINFO (C++ only) */
    test_tree_binfo();
    
    /* Complex expression to ensure all constructs are used */
    int final_result = vec_result + ssa_result + block_result + 
                      constr_result + omp_result;
    
    printf("Final result: %d\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
