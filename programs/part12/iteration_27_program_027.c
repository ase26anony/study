/* test_tree_kind.c - Comprehensive test for GCC tree node kinds */

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* Helper to avoid unused variable warnings */
#define USE(V) ((void)(V))

/* ==================== IDENTIFIER_NODE ==================== */
/* Variable and function names create IDENTIFIER_NODE */
int some_unique_identifier;
static int another_identifier;
void identifier_function(void) {
    int local_identifier = 42;
    USE(local_identifier);
}

/* ==================== TREE_VEC ==================== */
/* Using GCC statement expressions with multiple elements */
#ifdef __GNUC__
#define CREATE_VEC() ({ \
    int a = 1, b = 2, c = 3; \
    (typeof(a)[]){a, b, c}; \
})
#endif

void test_tree_vec(void) {
#ifdef __GNUC__
    /* This should generate TREE_VEC nodes */
    int *vec = CREATE_VEC();
    USE(vec);
#endif
}

/* ==================== SSA_NAME ==================== */
/* Complex enough to trigger SSA formation */
int ssa_test(int n) {
    int a = 0, b = 1, c;
    
    /* Loop with multiple assignments to force SSA */
    for (int i = 0; i < n; ++i) {
        a = a + i;
        b = b * 2;
        c = a + b;
        
        /* Conditional to create phi nodes */
        if (i % 2 == 0) {
            a = c;
        } else {
            b = a;
        }
    }
    
    /* Another loop with induction variable */
    int sum = 0;
    for (int j = 0; j < n; ++j) {
        sum += j * j;
        if (sum > 100) {
            sum = sum / 2;
        }
    }
    
    return a + b + c + sum;
}

/* ==================== BLOCK ==================== */
void test_blocks(void) {
    /* Outer block */
    int x = 10;
    USE(x);
    
    {
        /* Nested block 1 */
        int y = 20;
        USE(y);
        
        {
            /* Nested block 2 */
            int z = x + y;
            USE(z);
            
            {
                /* Deeply nested block */
                int w = z * 2;
                USE(w);
            }
        }
    }
    
    /* Another block with different scope */
    if (x > 5) {
        int temp = x * 3;
        USE(temp);
        
        for (int i = 0; i < 3; i++) {
            /* Loop block */
            int loop_var = temp + i;
            USE(loop_var);
        }
    }
}

/* ==================== CONSTRUCTOR ==================== */
struct Point {
    int x;
    int y;
    int z;
};

union Data {
    int i;
    float f;
    char str[4];
};

void test_constructors(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    USE(arr);
    
    /* Struct constructor */
    struct Point p1 = {10, 20, 30};
    USE(p1);
    
    /* Designated initializer */
    struct Point p2 = {.x = 5, .y = 15, .z = 25};
    USE(p2);
    
    /* Nested initializer */
    struct Point points[2] = {{1, 2, 3}, {4, 5, 6}};
    USE(points);
    
    /* Union constructor */
    union Data d1 = {.i = 42};
    USE(d1);
    
    /* Zero initializer */
    struct Point p3 = {0};
    USE(p3);
    
    /* Partial initializer */
    int partial[10] = {[2] = 100, [5] = 200};
    USE(partial);
}

/* ==================== OMP_CLAUSE ==================== */
#ifdef _OPENMP
void test_omp(void) {
    int i, n = 100;
    int a[100], b[100], c[100];
    
    /* Initialize arrays */
    for (i = 0; i < n; i++) {
        a[i] = i;
        b[i] = n - i;
    }
    
    /* Various OpenMP pragmas to generate OMP_CLAUSE nodes */
    
    /* parallel with private clause */
    #pragma omp parallel private(i)
    {
        int tid = omp_get_thread_num();
        USE(tid);
    }
    
    /* parallel for with reduction clause */
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < n; i++) {
        sum += a[i];
    }
    USE(sum);
    
    /* sections with shared clause */
    #pragma omp parallel sections shared(a, b, c)
    {
        #pragma omp section
        {
            for (i = 0; i < n/2; i++) {
                c[i] = a[i] + b[i];
            }
        }
        
        #pragma omp section
        {
            for (i = n/2; i < n; i++) {
                c[i] = a[i] - b[i];
            }
        }
    }
    
    /* single with copyprivate clause */
    int master_value = 0;
    #pragma omp parallel
    {
        #pragma omp single copyprivate(master_value)
        {
            master_value = 42;
        }
        
        /* All threads now have master_value = 42 */
        USE(master_value);
    }
    
    /* task with firstprivate clause */
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (i = 0; i < n; i++) {
                #pragma omp task firstprivate(i)
                {
                    int result = a[i] * b[i];
                    USE(result);
                }
            }
        }
    }
}
#endif

#ifdef __cplusplus
/* ==================== TREE_BINFO (C++ only) ==================== */
/* Class hierarchy to generate BINFO nodes */

class Base1 {
public:
    virtual ~Base1() {}
    virtual void foo() { cout << "Base1::foo" << endl; }
    int base1_data;
};

class Base2 {
public:
    virtual ~Base2() {}
    virtual void bar() { cout << "Base2::bar" << endl; }
    int base2_data;
};

class Derived : public Base1, public Base2 {
public:
    virtual ~Derived() {}
    virtual void foo() override { cout << "Derived::foo" << endl; }
    virtual void bar() override { cout << "Derived::bar" << endl; }
    virtual void baz() { cout << "Derived::baz" << endl; }
    int derived_data;
};

class DeepDerived : public Derived {
public:
    virtual ~DeepDerived() {}
    virtual void foo() override { cout << "DeepDerived::foo" << endl; }
    int deep_data;
};

void test_cpp_classes(void) {
    /* Create objects with different types to use vtables */
    Base1* b1 = new Derived();
    Base2* b2 = new Derived();
    Derived* d = new Derived();
    DeepDerived* dd = new DeepDerived();
    
    /* Virtual calls to ensure vtable usage */
    b1->foo();
    b2->bar();
    d->baz();
    dd->foo();
    
    /* Dynamic casts (requires RTTI) */
    Derived* d1 = dynamic_cast<Derived*>(b1);
    Derived* d2 = dynamic_cast<Derived*>(b2);
    
    /* Clean up */
    delete b1;
    delete b2;
    delete d;
    delete dd;
    
    USE(d1);
    USE(d2);
}
#endif

/* ==================== MAIN ==================== */
int main(int argc, char** argv) {
    USE(argc);
    USE(argv);
    
    /* Trigger all test functions */
    identifier_function();
    test_tree_vec();
    
    int ssa_result = ssa_test(50);
    printf("SSA test result: %d\n", ssa_result);
    
    test_blocks();
    test_constructors();
    
#ifdef _OPENMP
    test_omp();
    printf("OpenMP tests completed\n");
#endif
    
#ifdef __cplusplus
    test_cpp_classes();
    printf("C++ class tests completed\n");
#endif
    
    /* Use the identifiers */
    some_unique_identifier = 100;
    another_identifier = 200;
    printf("Identifiers: %d, %d\n", some_unique_identifier, another_identifier);
    
    return 0;
}
