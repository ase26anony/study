/* test_tree_kind.c - Coverage test for GCC's tree.cc get_kind function */

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* Helper to prevent optimization from removing code */
#ifdef __GNUC__
#define KEEP_USED __attribute__((used))
#else
#define KEEP_USED
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Any variable/function name creates an IDENTIFIER_NODE */
static int some_unique_identifier KEEP_USED = 42;
static void identifier_func(void) KEEP_USED {
    int another_identifier = some_unique_identifier;
    (void)another_identifier;
}

/* ==================== SSA_NAME ==================== */
/* Complex enough to trigger SSA formation */
void ssa_test(int n) KEEP_USED {
    int a = 0, b = 1, c = 2;
    for (int i = 0; i < n; ++i) {
        /* Multiple assignments to create SSA phi nodes */
        if (i % 2 == 0) {
            a = a + i * b;
            b = b + c;
        } else {
            a = a - i;
            c = c * 2;
        }
        /* Complex expression to force SSA */
        int d = (a * b) + (c << 2) - (i / 3);
        (void)d;
    }
    
    /* Nested loop for more SSA complexity */
    for (int j = 0; j < 10; j++) {
        int x = j;
        while (x > 0) {
            x = x / 2;
            a += x;
        }
    }
}

/* ==================== BLOCK ==================== */
/* Nested blocks create BLOCK nodes */
void block_test(void) KEEP_USED {
    int outer = 0;
    {
        int inner1 = 1;
        {
            int inner2 = 2;
            {
                int inner3 = 3;
                outer = inner1 + inner2 + inner3;
            }
        }
    }
    
    /* Switch with blocks */
    switch (outer) {
        case 1: {
            int case_var = 10;
            outer = case_var;
            break;
        }
        case 2: {
            int case_var = 20;
            outer = case_var;
            break;
        }
        default: {
            int case_var = 30;
            outer = case_var;
        }
    }
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers create CONSTRUCTOR nodes */
struct point {
    int x;
    int y;
    int z;
};

void constructor_test(void) KEEP_USED {
    /* Array constructor */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Struct constructor (C99 designated initializer) */
    struct point p1 = {.x = 1, .y = 2, .z = 3};
    
    /* Nested struct constructor */
    struct triangle {
        struct point a, b, c;
    };
    struct triangle t = {
        .a = {1, 2, 3},
        .b = {4, 5, 6},
        .c = {7, 8, 9}
    };
    
    /* Union constructor */
    union data {
        int i;
        float f;
    };
    union data u = {.i = 42};
    
    (void)arr; (void)p1; (void)t; (void)u;
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
#include <omp.h>
void omp_test(int n) KEEP_USED {
    int i;
    int sum = 0;
    int private_var = 0;
    
    /* Multiple OpenMP pragmas with different clauses */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static)
    for (i = 0; i < n; i++) {
        sum += i;
    }
    
    #pragma omp parallel private(private_var) shared(sum)
    {
        private_var = omp_get_thread_num();
        #pragma omp critical
        {
            sum += private_var;
        }
    }
    
    /* Nested parallel regions */
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            int section_var = 1;
            sum += section_var;
        }
        #pragma omp section
        {
            int section_var = 2;
            sum += section_var;
        }
    }
    
    (void)sum;
}
#else
void omp_test(int n) KEEP_USED {
    (void)n;
}
#endif

/* ==================== TREE_VEC ==================== */
/* GCC extensions that create TREE_VEC nodes */
#ifdef __GNUC__
void tree_vec_test(void) KEEP_USED {
    /* Using statement expression - may create TREE_VEC */
    int x = ({ 
        int y = 5; 
        int z = 10; 
        y + z; 
    });
    
    /* Typeof with multiple elements */
    typeof(int[3]) arr = {1, 2, 3};
    
    /* Vector types (GCC vector extension) */
    typedef int v4si __attribute__((vector_size(16)));
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
    
    (void)x; (void)arr; (void)v3;
}
#else
void tree_vec_test(void) KEEP_USED {}
#endif

#ifdef __cplusplus
/* ==================== TREE_BINFO (C++ only) ==================== */
/* Class hierarchies create TREE_BINFO nodes */
class Base1 {
public:
    virtual void foo() { }
    int base1_data;
};

class Base2 {
public:
    virtual void bar() { }
    int base2_data;
};

class Derived : public Base1, public Base2 {
public:
    virtual void foo() override { }
    virtual void bar() override { }
    int derived_data;
};

void binfo_test() KEEP_USED {
    Derived d;
    Base1* b1 = &d;
    Base2* b2 = &d;
    
    /* Multiple inheritance and virtual calls */
    b1->foo();
    b2->bar();
    
    /* Dynamic cast (requires RTTI, but still creates binfo) */
    Derived* d2 = dynamic_cast<Derived*>(b1);
    (void)d2;
    
    /* Template with inheritance */
    template<typename T>
    class TemplateBase {
    public:
        virtual T get() { return T(); }
    };
    
    class Concrete : public TemplateBase<int> {
    public:
        virtual int get() override { return 42; }
    };
    
    Concrete c;
    TemplateBase<int>* tb = &c;
    int val = tb->get();
    (void)val;
}
#endif

/* ==================== MAIN ==================== */
int main(int argc, char** argv) {
    /* Reference all test functions to ensure they're not optimized away */
    identifier_func();
    ssa_test(argc > 1 ? 100 : 50);
    block_test();
    constructor_test();
    omp_test(100);
    tree_vec_test();
    
#ifdef __cplusplus
    binfo_test();
#endif
    
    /* Compute something to make the program useful */
    int result = some_unique_identifier;
    for (int i = 0; i < 10; i++) {
        result += i;
    }
    
#ifdef __cplusplus
    cout << "Result: " << result << endl;
#else
    printf("Result: %d\n", result);
#endif
    
    return 0;
}
