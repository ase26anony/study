/* test_tree_kind.c - Comprehensive test for GCC tree node kinds */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 test_tree_kind.c -o test_tree_kind */
/* For C++ specific parts: g++ -O2 -fopenmp -std=gnu++11 test_tree_kind.c -o test_tree_kind_cpp */

#include <stdio.h>
#include <stdlib.h>

/* Helper to prevent optimization */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* ===== IDENTIFIER_NODE ===== */
/* Variable and function names create IDENTIFIER_NODE */
static int some_unique_identifier_42 = 0;
static void function_with_identifiers(void) {
    int local_identifier = 42;
    some_unique_identifier_42 = local_identifier;
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
    
    /* Loop with multiple assignments to force SSA */
    for (int i = 0; i < n; ++i) {
        a = a + i;
        b = b * 2;
        c = a + b;
        
        /* Conditional to create phi nodes */
        if (c % 2 == 0) {
            a = c / 2;
        } else {
            a = c + 1;
        }
    }
    
    /* Multiple basic blocks for SSA */
    switch (n % 3) {
        case 0: return a;
        case 1: return b;
        default: return c;
    }
}

/* ===== BLOCK ===== */
/* Nested blocks with local variables */
static int block_example(int x) {
    int outer = x * 2;
    
    /* First nested block */
    {
        int inner1 = outer + 5;
        
        /* Second nested block */
        {
            int inner2 = inner1 * 3;
            {
                /* Third nested block */
                int inner3 = inner2 - 10;
                outer = inner3;
            }
        }
    }
    
    /* Another block with different scope */
    if (outer > 0) {
        int positive = 1;
        for (int i = 0; i < 5; i++) {
            int loop_var = i * 2;
            positive += loop_var;
        }
        return positive;
    }
    
    return outer;
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
    
    /* Nested struct with array */
    struct nested {
        int id;
        struct point pos;
        float values[3];
    };
    
    struct nested n1 = {
        100,
        {1, 2, 3},
        {1.0f, 2.0f, 3.0f}
    };
    
    use(&arr[0]);
    use(&p1);
    use(&p2);
    use(&n1);
}

/* ===== OMP_CLAUSE ===== */
/* OpenMP pragmas */
#ifdef _OPENMP
static void omp_clause_example(int size) {
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
    #pragma omp parallel sections private(i) shared(sum)
    {
        #pragma omp section
        {
            for (i = 0; i < 50; i++) {
                #pragma omp atomic
                sum += 1;
            }
        }
        
        #pragma omp section
        {
            for (i = 50; i < 100; i++) {
                #pragma omp atomic
                sum += 2;
            }
        }
    }
    
    printf("OMP sum: %d\n", sum);
}
#endif

/* ===== C++ Specific: TREE_BINFO ===== */
#ifdef __cplusplus

class BaseClass1 {
public:
    virtual void method1() { }
    int base_data1;
};

class BaseClass2 {
public:
    virtual void method2() { }
    int base_data2;
};

class DerivedClass : public BaseClass1, public BaseClass2 {
public:
    virtual void method1() override { }
    virtual void method2() override { }
    int derived_data;
};

static void tree_binfo_example(void) {
    DerivedClass obj;
    BaseClass1* ptr1 = &obj;
    BaseClass2* ptr2 = &obj;
    
    ptr1->method1();
    ptr2->method2();
    
    use(&obj);
    use(ptr1);
    use(ptr2);
}

#endif /* __cplusplus */

/* ===== Main driver ===== */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Trigger all examples */
    function_with_identifiers();
    
    #ifdef __GNUC__
    result += tree_vec_example();
    #endif
    
    result += ssa_name_example(argc > 1 ? atoi(argv[1]) : 10);
    result += block_example(result);
    constructor_example();
    
    #ifdef _OPENMP
    omp_clause_example(100);
    #endif
    
    #ifdef __cplusplus
    tree_binfo_example();
    #endif
    
    printf("Result: %d\n", result);
    return result % 256;
}
