/* test_tree_kind.c - Coverage test for GCC's get_kind function */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 test_tree_kind.c -o test_tree_kind */
/* For C++ mode: g++ -O2 -fopenmp -std=gnu++11 test_tree_kind.c -o test_tree_kind_cpp */

#include <stdio.h>
#include <stdlib.h>

/* ==================== IDENTIFIER_NODE ==================== */
/* Variable and function names create IDENTIFIER_NODE */
int unique_identifier_1;
void function_with_identifiers(void) {
    int local_identifier = 42;
    unique_identifier_1 = local_identifier;
}

/* ==================== SSA_NAME ==================== */
/* Complex arithmetic with loops forces SSA form */
int ssa_test_function(int n) {
    int a = 0, b = 1, c;
    for (int i = 0; i < n; ++i) {
        c = a + b;      /* This will create SSA_NAME nodes */
        a = b;
        b = c;
        if (c > 1000) {
            a = c % 7;  /* More SSA opportunities */
        }
    }
    /* Multiple basic blocks create phi nodes (SSA) */
    return (n > 0) ? a : b;
}

/* ==================== BLOCK ==================== */
/* Nested blocks with local variables */
void block_test(void) {
    int outer = 10;
    {
        int inner = 20;
        {
            int innermost = 30;
            outer = inner + innermost;
        }
        /* Another block */
        {
            int temp = outer * 2;
            outer = temp;
        }
    }
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers */
struct my_struct {
    int x;
    double y;
    char z[4];
};

void constructor_test(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor */
    struct my_struct s1 = {10, 3.14, "abc"};
    
    /* Designated initializer */
    struct my_struct s2 = {.x = 42, .y = 2.718, .z = "def"};
    
    /* Nested initializer */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas generate OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    
    #pragma omp parallel for private(i) reduction(+:sum) firstprivate(private_var)
    for (i = 0; i < n; i++) {
        sum += i * 2;
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
}
#endif

/* ==================== TREE_VEC ==================== */
/* GCC extensions that create TREE_VEC nodes */
#ifdef __GNUC__
void tree_vec_test(void) {
    /* Using statement expression - may create TREE_VEC */
    int a = ({ 
        int x = 5; 
        int y = 10; 
        x + y; 
    });
    
    /* Typeof with multiple elements */
    typeof(int[3]) arr = {1, 2, 3};
    
    /* Vector extensions (if available) */
    #ifdef __SSE2__
    typedef int v4si __attribute__((vector_size(16)));
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
    #endif
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

class MultiBase : public BaseClass {
public:
    virtual void method() override {
        base_data = 24;
    }
};

class DiamondDerived : public DerivedClass, public MultiBase {
public:
    virtual void method() override {
        DerivedClass::method();
    }
    void test_binfo() {
        BaseClass* bp1 = static_cast<DerivedClass*>(this);
        BaseClass* bp2 = static_cast<MultiBase*>(this);
        bp1->method();
        bp2->method();
    }
};

void cpp_binfo_test() {
    DerivedClass d;
    DiamondDerived dd;
    BaseClass* b1 = &d;
    BaseClass* b2 = static_cast<MultiBase*>(&dd);
    
    b1->method();
    b2->method();
    
    dd.test_binfo();
}

#endif /* __cplusplus */

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char** argv) {
    int n = 100;
    
    /* Ensure all constructs are referenced/used */
    function_with_identifiers();
    
    int ssa_result = ssa_test_function(n);
    printf("SSA test result: %d\n", ssa_result);
    
    block_test();
    
    constructor_test();
    
#ifdef _OPENMP
    omp_test(n);
#endif
    
#ifdef __GNUC__
    tree_vec_test();
#endif
    
#ifdef __cplusplus
    cpp_binfo_test();
#endif
    
    /* Complex control flow to engage middle-end passes */
    int total = 0;
    for (int i = 0; i < 1000; i++) {
        if (i % 3 == 0) {
            total += ssa_test_function(i % 10);
        } else if (i % 7 == 0) {
            total -= i;
        } else {
            total *= (i % 5) + 1;
        }
    }
    
    printf("Final total: %d\n", total);
    return 0;
}
