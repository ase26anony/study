/* test_tree_kind.c - Coverage test for GCC's get_kind function */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 test_tree_kind.c -o test_tree_kind */
/* For C++ mode: g++ -O2 -fopenmp -std=gnu++11 test_tree_kind.c -o test_tree_kind_cpp */

#include <stdio.h>
#include <stdlib.h>

/* Helper to prevent optimization */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* ========== IDENTIFIER_NODE ========== */
/* Variable and function names create IDENTIFIER_NODE */
int some_unique_identifier_1;
static int another_identifier_2;
void function_with_identifier(void) {
    int local_identifier = 42;
    use(&local_identifier);
}

/* ========== SSA_NAME ========== */
/* Complex arithmetic with loops forces SSA form */
int ssa_test(int n) {
    int a = 0, b = 1, c = 2;
    for (int i = 0; i < n; ++i) {
        a = a + i;      /* Creates phi nodes in SSA */
        b = b * a;
        c = c - b;
        if (c > 100) {
            a = c / 2;  /* Another assignment creating SSA */
        }
    }
    /* Multiple uses to prevent dead code elimination */
    return a + b + c;
}

/* ========== BLOCK ========== */
/* Nested blocks with local variables */
void block_test(void) {
    int outer = 1;
    use(&outer);
    
    { /* BLOCK 1 */
        int inner1 = 2;
        use(&inner1);
        
        { /* BLOCK 2 */
            int inner2 = 3;
            use(&inner2);
            
            { /* BLOCK 3 */
                int inner3 = 4;
                use(&inner3);
            }
        }
    }
}

/* ========== CONSTRUCTOR ========== */
/* Aggregate initializers */
struct my_struct {
    int a;
    double b;
    char c[10];
};

void constructor_test(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor */
    struct my_struct s1 = {10, 3.14, "hello"};
    
    /* Designated initializer */
    struct my_struct s2 = {.a = 20, .b = 2.71, .c = "world"};
    
    /* Nested initializer */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    
    use(arr);
    use(&s1);
    use(&s2);
    use(matrix);
}

/* ========== OMP_CLAUSE ========== */
#ifdef _OPENMP
void omp_test(int n) {
    int i, sum = 0;
    int private_var = 0;
    
    #pragma omp parallel for private(i) reduction(+:sum) firstprivate(private_var)
    for (i = 0; i < n; i++) {
        sum += i;
    }
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("OpenMP thread count: variable access\n");
        }
    }
    
    use(&sum);
}
#else
void omp_test(int n) {
    /* Dummy implementation when OpenMP not available */
    printf("OpenMP not enabled\n");
    use(&n);
}
#endif

/* ========== TREE_VEC ========== */
/* Using GCC statement expressions (GNU extension) */
void tree_vec_test(void) {
    /* Statement expression creates TREE_VEC */
    int result = ({
        int x = 5;
        int y = 10;
        x + y;
    });
    
    /* Another example with multiple statements */
    int vec_result = ({
        int a = 1;
        int b = 2;
        int c = 3;
        a = b + c;
        c = a * b;
        c;
    });
    
    use(&result);
    use(&vec_result);
}

/* ========== C++ Specific: TREE_BINFO ========== */
#ifdef __cplusplus

class Base1 {
public:
    virtual void foo() { }
    int base1_data;
};

class Base2 {
public:
    virtual void bar() { }
    double base2_data;
};

class Derived : public Base1, public Base2 {
public:
    virtual void foo() override { }
    virtual void bar() override { }
    char derived_data;
};

void binfo_test(void) {
    Derived d;
    Base1* b1 = &d;
    Base2* b2 = &d;
    
    /* Virtual calls to ensure binfo is used */
    b1->foo();
    b2->bar();
    
    use(&d);
    use(b1);
    use(b2);
}

#else
/* C version - binfo nodes are C++ only */
void binfo_test(void) {
    printf("BINFO test requires C++ mode\n");
}
#endif

/* ========== Main Driver ========== */
int main(int argc, char **argv) {
    int n = 100;
    
    /* Reference all identifiers to ensure they're used */
    some_unique_identifier_1 = 1;
    another_identifier_2 = 2;
    function_with_identifier();
    
    /* Execute all tests */
    int ssa_result = ssa_test(n);
    printf("SSA test result: %d\n", ssa_result);
    
    block_test();
    constructor_test();
    
    tree_vec_test();
    
    omp_test(n);
    
    binfo_test();
    
    /* Complex control flow to engage more middle-end passes */
    int total = 0;
    for (int i = 0; i < 1000; i++) {
        if (i % 3 == 0) {
            total += i * 2;
        } else if (i % 7 == 0) {
            total -= i / 2;
        } else {
            total += 1;
        }
    }
    
    printf("Final total: %d\n", total);
    
    return 0;
}
