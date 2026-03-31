/* test_tree_kind.c - Coverage test for GCC's get_kind function */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 test_tree_kind.c -o test_tree_kind */
/* For C++ mode: g++ -O2 -fopenmp -std=gnu++11 test_tree_kind.c -o test_tree_kind_cpp */

#include <stdio.h>
#include <stdlib.h>

/* Helper to ensure code isn't optimized away */
static void use(void *p) {
    volatile void *v = p;
    (void)v;
}

/* ===== IDENTIFIER_NODE ===== */
/* Variable and function names create IDENTIFIER_NODE */
static int some_unique_identifier_1;
static void function_with_identifiers(void) {
    int local_identifier = 42;
    some_unique_identifier_1 = local_identifier;
}

/* ===== TREE_VEC ===== */
/* GCC statement expressions can create TREE_VEC nodes */
#ifdef __GNUC__
static int tree_vec_example(void) {
    /* Using statement expression with multiple elements */
    int result = ({ 
        int a = 5; 
        int b = 10; 
        int c = a + b;
        c * 2;
    });
    return result;
}
#endif

/* ===== SSA_NAME ===== */
/* Complex enough code to trigger SSA form */
static int ssa_name_example(int n) {
    int a = 0, b = 1, c;
    
    /* Loop with arithmetic to force SSA */
    for (int i = 0; i < n; ++i) {
        a = a + i;      /* Creates phi nodes in SSA */
        b = b * 2;
        c = a + b;      /* More SSA names */
        
        /* Conditional to create control flow merge */
        if (i % 2 == 0) {
            a = c - 1;
        } else {
            b = a + 3;
        }
    }
    
    /* Another loop with induction variable */
    int sum = 0;
    for (int j = 0; j < n; ++j) {
        sum += j * j;
    }
    
    return a + b + c + sum;
}

/* ===== BLOCK ===== */
/* Nested blocks with local variables */
static void block_example(void) {
    int outer = 10;
    
    { /* BLOCK 1 */
        int inner1 = outer + 5;
        
        { /* BLOCK 2 - deeper nesting */
            int inner2 = inner1 * 2;
            { /* BLOCK 3 - even deeper */
                int inner3 = inner2 / 3;
                use(&inner3);
            }
        }
        
        /* Another block with different scope */
        {
            float temp = 3.14f;
            int int_temp = (int)temp;
            use(&int_temp);
        }
    }
}

/* ===== CONSTRUCTOR ===== */
/* Aggregate initializers */
static void constructor_example(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor */
    struct point {
        int x;
        int y;
        int z;
    };
    
    struct point p1 = {10, 20, 30};
    struct point p2 = {.x = 5, .y = 15, .z = 25};
    
    /* Nested struct with constructor */
    struct rectangle {
        struct point top_left;
        struct point bottom_right;
    };
    
    struct rectangle rect = {
        {0, 0, 0},
        {100, 100, 0}
    };
    
    use(arr);
    use(&p1);
    use(&p2);
    use(&rect);
}

/* ===== OMP_CLAUSE ===== */
/* OpenMP pragmas */
#ifdef _OPENMP
static void omp_clause_example(void) {
    int i;
    int sum = 0;
    int data[100];
    
    /* Initialize data */
    for (i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    /* Multiple OpenMP clauses */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static)
    for (i = 0; i < 100; i++) {
        sum += data[i];
    }
    
    /* Another with different clauses */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < 50; i++) {
                data[i] *= 2;
            }
        }
        
        #pragma omp section
        {
            for (i = 50; i < 100; i++) {
                data[i] /= 2;
            }
        }
    }
    
    printf("OpenMP sum: %d\n", sum);
}
#endif

/* ===== C++ Specific: TREE_BINFO ===== */
#ifdef __cplusplus

class BaseClass1 {
public:
    virtual void method1() { printf("Base1\n"); }
    int base_data1;
};

class BaseClass2 {
public:
    virtual void method2() { printf("Base2\n"); }
    int base_data2;
};

class DerivedClass : public BaseClass1, public BaseClass2 {
public:
    virtual void method1() override { printf("Derived::method1\n"); }
    virtual void method2() override { printf("Derived::method2\n"); }
    int derived_data;
};

static void tree_binfo_example(void) {
    DerivedClass obj;
    BaseClass1* ptr1 = &obj;
    BaseClass2* ptr2 = &obj;
    
    ptr1->method1();  /* Virtual call through base pointer */
    ptr2->method2();  /* Another virtual call */
    
    /* Multiple inheritance creates BINFO nodes */
    DerivedClass* dptr = dynamic_cast<DerivedClass*>(ptr1);
    if (dptr) {
        dptr->derived_data = 42;
    }
}

#endif /* __cplusplus */

/* ===== Main driver ===== */
int main(int argc, char **argv) {
    int n = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Execute all examples */
    function_with_identifiers();
    
    #ifdef __GNUC__
    int vec_result = tree_vec_example();
    printf("TREE_VEC example result: %d\n", vec_result);
    #endif
    
    int ssa_result = ssa_name_example(n);
    printf("SSA_NAME example result: %d\n", ssa_result);
    
    block_example();
    constructor_example();
    
    #ifdef _OPENMP
    omp_clause_example();
    #endif
    
    #ifdef __cplusplus
    tree_binfo_example();
    #endif
    
    /* Complex control flow to keep optimizer busy */
    int final_result = 0;
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            final_result += i * 2;
        } else if (i % 3 == 1) {
            final_result -= i;
        } else {
            final_result *= (i % 10) + 1;
        }
    }
    
    printf("Final result: %d\n", final_result);
    return 0;
}
