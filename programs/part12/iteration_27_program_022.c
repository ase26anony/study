/* test_tree_kind_coverage.c - Comprehensive test for GCC tree kind coverage */

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Variable and function names create IDENTIFIER_NODE */
int global_identifier_node_test;
static int static_identifier_node_test;

void function_with_identifiers() {
    int local_identifier = 42;
    global_identifier_node_test = local_identifier;
}

/* ==================== SSA_NAME ==================== */
/* Complex arithmetic and loops force SSA form */
int ssa_name_test(int n) {
    int result = 0;
    for (int i = 0; i < n; ++i) {
        result = result + i * 2;  /* This creates SSA_NAME nodes */
        if (result > 100) {
            result = result / 2;
        }
    }
    
    /* More SSA opportunities */
    int a = 1, b = 2, c = 3;
    for (int j = 0; j < 10; ++j) {
        a = b + c;
        b = c * 2;
        c = a - b;
    }
    
    return result + a + b + c;
}

/* ==================== BLOCK ==================== */
/* Nested blocks create BLOCK nodes */
void block_test() {
    /* Outer block */
    int x = 10;
    
    {
        /* Inner block 1 */
        int y = 20;
        x = x + y;
        
        {
            /* Inner block 2 */
            int z = 30;
            x = x + z;
            
            {
                /* Deeply nested block */
                int w = 40;
                x = x + w;
            }
        }
    }
    
    /* Another block with control flow */
    if (x > 0) {
        int temp = x * 2;
        x = temp;
    }
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers create CONSTRUCTOR nodes */
struct point {
    int x;
    int y;
    int z;
};

struct rectangle {
    struct point top_left;
    struct point bottom_right;
};

void constructor_test() {
    /* Array constructor */
    int array_constructor[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor (C99 designated initializer) */
    struct point p1 = {.x = 10, .y = 20, .z = 30};
    
    /* Nested struct constructor */
    struct rectangle rect = {
        .top_left = {1, 2, 3},
        .bottom_right = {4, 5, 6}
    };
    
    /* More complex constructor with expressions */
    int expr_constructor[3] = {1 + 2, 3 * 4, 5 - 6};
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_clause_test(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    
    #pragma omp parallel for private(i, private_var) reduction(+:sum) schedule(static)
    for (i = 0; i < n; i++) {
        private_var = i * 2;
        sum += private_var;
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
    
    #pragma omp task shared(sum) if(n > 100)
    {
        sum = sum * 2;
    }
}
#endif

/* ==================== TREE_VEC ==================== */
/* GCC extensions and statement expressions create TREE_VEC nodes */
#ifdef __GNUC__
void tree_vec_test() {
    /* Using statement expression (GCC extension) */
    int a = ({ 
        int b = 5; 
        int c = 10; 
        b + c; 
    });
    
    /* Another TREE_VEC example with multiple values */
    int result = ({
        int x = 1, y = 2, z = 3;
        int temp = x + y;
        temp * z;
    });
    
    /* Complex statement expression with control flow */
    int complex_vec = ({
        int val = 0;
        for (int i = 0; i < 5; i++) {
            val += i * i;
        }
        if (val > 10) {
            val = val / 2;
        }
        val;
    });
}
#endif

#ifdef __cplusplus
/* ==================== TREE_BINFO (C++ only) ==================== */
/* Class hierarchies create TREE_BINFO nodes */

class BaseClass1 {
public:
    virtual void base1_method() {}
    int base1_data;
};

class BaseClass2 {
public:
    virtual void base2_method() {}
    int base2_data;
};

class DerivedClass : public BaseClass1, public BaseClass2 {
public:
    virtual void base1_method() override {}
    virtual void base2_method() override {}
    int derived_data;
};

class DeepDerived : public DerivedClass {
public:
    virtual void base1_method() override {}
    int deep_data;
};

void tree_binfo_test() {
    DerivedClass derived;
    BaseClass1* base1_ptr = &derived;
    BaseClass2* base2_ptr = &derived;
    
    DeepDerived deep;
    DerivedClass* derived_ptr = &deep;
    
    /* Virtual calls to ensure vtable usage */
    base1_ptr->base1_method();
    base2_ptr->base2_method();
    derived_ptr->base1_method();
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Exercise IDENTIFIER_NODE */
    function_with_identifiers();
    
    /* Exercise SSA_NAME */
    result += ssa_name_test(100);
    
    /* Exercise BLOCK */
    block_test();
    
    /* Exercise CONSTRUCTOR */
    constructor_test();
    
    /* Exercise TREE_VEC (if GCC) */
    #ifdef __GNUC__
    tree_vec_test();
    #endif
    
    /* Exercise OMP_CLAUSE (if OpenMP supported) */
    #ifdef _OPENMP
    omp_clause_test(1000);
    #endif
    
    #ifdef __cplusplus
    /* Exercise TREE_BINFO (C++ only) */
    tree_binfo_test();
    
    cout << "C++ test completed. Result: " << result << endl;
    #else
    printf("C test completed. Result: %d\n", result);
    #endif
    
    return result == 0 ? 0 : 1;
}
