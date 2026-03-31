/* test_tree_kind.c - Coverage test for GCC's get_kind function */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 test_tree_kind.c -o test_tree_kind */
/* For C++ mode: g++ -O2 -fopenmp -std=gnu++11 test_tree_kind.c -o test_tree_kind_cpp */

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
#include <vector>
#endif

/* Helper to ensure code isn't optimized away */
volatile int global_counter = 0;

/* ==================== IDENTIFIER_NODE ==================== */
/* Any variable/function name creates an IDENTIFIER_NODE */
void test_identifier_node(void) {
    int some_unique_identifier = 42;  /* This creates IDENTIFIER_NODE */
    global_counter += some_unique_identifier;
}

/* ==================== SSA_NAME ==================== */
/* Complex enough to trigger SSA optimization passes */
int test_ssa_name(int n) {
    int a = 0, b = 1, c = 0;
    
    /* Loop with arithmetic to force SSA form */
    for (int i = 0; i < n; ++i) {
        a = a + i;      /* Creates SSA_NAME nodes */
        b = b * 2;
        c = a + b;
        
        /* Conditional to add complexity */
        if (c > 100) {
            a = c / 2;
        }
    }
    
    /* Another loop with phi node potential */
    int sum = 0;
    for (int j = 0; j < n; j++) {
        sum += j * j;
        if (sum > 1000) {
            sum = sum % 1000;
        }
    }
    
    return a + b + c + sum;
}

/* ==================== BLOCK ==================== */
/* Nested blocks create BLOCK nodes */
void test_block_node(void) {
    /* Outer block */
    int x = 10;
    {
        /* Inner block 1 - creates BLOCK node */
        int y = x * 2;
        {
            /* Inner block 2 - deeper nesting */
            int z = y + 5;
            global_counter += z;
        }
    }
    
    /* Another block with different scope */
    {
        double temp = 3.14159;
        {
            float f = temp * 2.0f;
            global_counter += (int)f;
        }
    }
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers create CONSTRUCTOR nodes */
void test_constructor_node(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};  /* Creates CONSTRUCTOR node */
    
    /* Struct constructor (in C) */
    struct point {
        int x;
        int y;
        char label[10];
    };
    
    struct point p1 = {10, 20, "origin"};  /* Another CONSTRUCTOR */
    struct point p2 = {.x = 5, .y = 15, .label = "test"};  /* Designated init */
    
    /* Nested struct constructor */
    struct rectangle {
        struct point top_left;
        struct point bottom_right;
    };
    
    struct rectangle rect = {
        {0, 0, "tl"},
        {100, 100, "br"}
    };
    
    global_counter += arr[0] + p1.x + rect.top_left.x;
}

/* ==================== TREE_VEC ==================== */
/* GCC extensions that create TREE_VEC nodes */
void test_tree_vec(void) {
    /* Using statement expression - GCC extension that may create TREE_VEC */
    int result = ({
        int a = 5;
        int b = 10;
        int c = a + b;
        c * 2;  /* Last expression is result */
    });
    
    /* Another potential TREE_VEC source: type conversion */
    typedef int my_array[3];
    my_array ma = {1, 2, 3};
    
    /* Complex expression with multiple elements */
    global_counter += result + ma[0];
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
void test_omp_clause(int n) {
    int i;
    int sum = 0;
    int data[100];
    
    /* Initialize data */
    for (i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    /* OpenMP parallel region with clauses */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static)
    for (i = 0; i < n && i < 100; i++) {
        sum += data[i];
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel sections private(i) shared(sum, data)
    {
        #pragma omp section
        {
            for (i = 0; i < 10; i++) {
                sum += data[i];
            }
        }
        
        #pragma omp section
        {
            for (i = 10; i < 20; i++) {
                sum += data[i];
            }
        }
    }
    
    global_counter += sum;
}

#ifdef __cplusplus
/* ==================== TREE_BINFO (C++ only) ==================== */
/* Class hierarchies create TREE_BINFO nodes */

/* Base class */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual void do_something() {
        global_counter += 1;
    }
    int base_data;
};

/* Derived class with virtual inheritance */
class DerivedClass : public virtual BaseClass {
public:
    virtual void do_something() override {
        global_counter += 2;
    }
    int derived_data;
};

/* Multiple inheritance */
class AnotherBase {
public:
    virtual ~AnotherBase() {}
    virtual void another_method() {
        global_counter += 3;
    }
};

class MultiDerived : public DerivedClass, public AnotherBase {
public:
    virtual void do_something() override {
        global_counter += 4;
    }
    
    virtual void another_method() override {
        global_counter += 5;
    }
};

void test_tree_binfo(void) {
    /* Create objects to instantiate the hierarchy */
    BaseClass* obj1 = new DerivedClass();
    BaseClass* obj2 = new MultiDerived();
    AnotherBase* obj3 = new MultiDerived();
    
    /* Use virtual functions */
    obj1->do_something();
    obj2->do_something();
    obj3->another_method();
    
    /* Clean up */
    delete obj1;
    delete obj2;
    delete obj3;
    
    /* Template with inheritance - may create more BINFO nodes */
    std::vector<BaseClass*> vec;
    vec.push_back(new DerivedClass());
    for (auto* ptr : vec) {
        ptr->do_something();
        delete ptr;
    }
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char** argv) {
    int n = 50;
    
    /* Test all constructs */
    test_identifier_node();
    
    int ssa_result = test_ssa_name(n);
    printf("SSA test result: %d\n", ssa_result);
    
    test_block_node();
    test_constructor_node();
    test_tree_vec();
    
    /* Test OpenMP if available */
    #ifdef _OPENMP
    test_omp_clause(n);
    printf("OpenMP support detected\n");
    #else
    printf("No OpenMP support\n");
    #endif
    
    #ifdef __cplusplus
    test_tree_binfo();
    printf("C++ mode with class hierarchies\n");
    #endif
    
    printf("Global counter: %d\n", global_counter);
    
    /* Additional complexity to ensure middle-end passes run */
    if (argc > 1) {
        char* endptr;
        long val = strtol(argv[1], &endptr, 10);
        if (*endptr == '\0') {
            return test_ssa_name((int)val);
        }
    }
    
    return 0;
}
