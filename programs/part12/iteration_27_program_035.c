/* test_tree_kind_coverage.c - Comprehensive test to trigger all tree_kind cases */
#include <stdio.h>
#include <stdlib.h>

/* Enable OpenMP support */
#ifdef _OPENMP
#include <omp.h>
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Using various identifiers in declarations */
void test_identifier_node() {
    int some_unique_identifier = 42;
    int another_identifier = some_unique_identifier * 2;
    volatile int force_usage = another_identifier; /* Prevent optimization */
    (void)force_usage;
}

/* ==================== SSA_NAME ==================== */
/* Create code that forces SSA form generation */
int test_ssa_name(int n) {
    int a = 0;
    int b = 1;
    
    /* Complex loop to generate SSA names */
    for (int i = 0; i < n; ++i) {
        a = a + b;      /* Generates phi nodes in SSA */
        b = b * 2;      /* More SSA transformations */
        if (i % 3 == 0) {
            a = a - 1;  /* Conditional assignment for SSA */
        }
    }
    
    /* Another loop with induction variable */
    int sum = 0;
    for (int j = 0; j < n; ++j) {
        sum += j * j;   /* SSA for induction variable */
    }
    
    return a + b + sum;
}

/* ==================== BLOCK ==================== */
/* Nested blocks with local variables */
void test_block() {
    int outer = 10;
    
    { /* Block 1 */
        int inner1 = outer + 5;
        
        { /* Block 2 */
            int inner2 = inner1 * 2;
            
            { /* Block 3 */
                int inner3 = inner2 / 3;
                volatile int block_var = inner3;
                (void)block_var;
            }
        }
    }
    
    /* Switch statement creates blocks */
    switch (outer) {
        case 10: {
            int case_var = 100;
            volatile int v = case_var;
            (void)v;
            break;
        }
        default: {
            int default_var = 200;
            volatile int v = default_var;
            (void)v;
            break;
        }
    }
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers */
void test_constructor() {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Point p1 = {10, 20, 30};
    struct Point p2 = {.x = 5, .y = 15, .z = 25};
    
    /* Nested struct with constructor */
    struct Line {
        struct Point start;
        struct Point end;
    };
    
    struct Line line = {{0, 0, 0}, {10, 10, 10}};
    
    volatile int use = arr[0] + p1.x + p2.y + line.start.z;
    (void)use;
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas */
void test_omp_clause(int n) {
    int i;
    int sum = 0;
    
    #ifdef _OPENMP
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static)
    for (i = 0; i < n; i++) {
        sum += i * i;
    }
    
    /* Another OpenMP region with different clauses */
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (i = 0; i < n; i++) {
            /* Do some work */
        }
        
        #pragma omp barrier
        
        #pragma omp single
        {
            int single_var = 42;
            volatile int v = single_var;
            (void)v;
        }
    }
    #endif
    
    volatile int result = sum;
    (void)result;
}

/* ==================== TREE_VEC (GCC extension) ==================== */
/* Using statement expressions - GCC extension */
void test_tree_vec() {
    /* Statement expression creates TREE_VEC */
    int result = ({
        int a = 10;
        int b = 20;
        int c = 30;
        a + b + c;  /* Last expression is result */
    });
    
    /* Another example with type in statement expression */
    typeof(result) x = ({
        typeof(result) temp = result * 2;
        temp + 5;
    });
    
    volatile int use = result + x;
    (void)use;
}

/* ==================== C++ Specific Code for TREE_BINFO ==================== */
#ifdef __cplusplus

/* Base class for BINFO generation */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual void base_method() { 
        int x = 42;
        volatile int v = x;
        (void)v;
    }
    int base_data;
};

/* Derived class with virtual inheritance */
class DerivedClass : public virtual BaseClass {
public:
    virtual void derived_method() {
        base_method();  /* Call base method */
        int y = 84;
        volatile int v = y;
        (void)v;
    }
    int derived_data;
};

/* Multiple inheritance for more complex BINFO */
class AnotherBase {
public:
    virtual ~AnotherBase() {}
    virtual void another_method() {}
    int another_data;
};

class MultipleDerived : public DerivedClass, public AnotherBase {
public:
    void multiple_method() {
        base_method();
        derived_method();
        another_method();
    }
};

void test_tree_binfo() {
    DerivedClass* obj = new DerivedClass();
    obj->base_method();
    obj->derived_method();
    
    MultipleDerived* multi = new MultipleDerived();
    multi->multiple_method();
    
    delete obj;
    delete multi;
}

#else
/* C version - dummy function */
void test_tree_binfo() {
    /* Not applicable in C */
}
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char** argv) {
    int n = 100;
    
    /* Execute all tests */
    test_identifier_node();
    
    int ssa_result = test_ssa_name(n);
    printf("SSA test result: %d\n", ssa_result);
    
    test_block();
    test_constructor();
    test_omp_clause(n);
    test_tree_vec();
    test_tree_binfo();
    
    /* Complex control flow to ensure optimization passes run */
    if (argc > 1) {
        for (int i = 0; i < argc; i++) {
            volatile int arg_len = 0;
            while (argv[i][arg_len] != '\0') {
                arg_len++;
            }
            printf("Arg %d length: %d\n", i, arg_len);
        }
    }
    
    return 0;
}
