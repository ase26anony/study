/* test_tree.c - Comprehensive tree node coverage test */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(void);
extern void external_func2(int);
extern volatile int external_var;

/* Sink function to prevent optimization */
static volatile int sink;

/* Force SSA_NAME creation with complex control flow */
int ssa_test(int n) {
    int i, s = 0, t = 1;
    
    /* Complex loop with multiple assignments */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t = s % 7;
        } else {
            s *= 2;
            t = (t + i) & 3;
        }
        
        /* Nested condition */
        if (s > 100) {
            s = s / 2;
            t = t * 3;
        }
    }
    
    /* Another variable with multiple assignments */
    int x = s;
    for (i = 0; i < 5; i++) {
        x = x + (x & 1 ? x * 2 : x / 2);
    }
    
    return s + x + t;
}

/* Generate BLOCK nodes with labels and gotos */
void block_test(void) {
    int a = 0;
    
    /* First block with label */
    block1: {
        int b = 1;
        a += b;
        {
            /* Nested block */
            int c = 2;
            a += c;
            goto block3;
        }
    }
    
    block2: {
        int d = 3;
        a += d;
        goto block4;
    }
    
    block3: {
        int e = 4;
        a += e;
        goto block2;
    }
    
    block4: {
        int f = 5;
        a += f;
    }
    
    sink = a;
}

/* Generate CONSTRUCTOR nodes with various initializers */
void constructor_test(void) {
    /* Struct with designated initializers */
    struct S1 {
        int a;
        int b[3];
        struct {
            int x;
            int y;
        } nested;
    };
    
    struct S1 s1 = {
        .a = 1,
        .b = {[0] = 2, [2] = 4},
        .nested = {.x = 5, .y = 6}
    };
    
    /* Partial array initialization */
    int arr[10] = {[0] = 1, [5] = 2, [9] = 3};
    
    /* Nested struct initialization */
    struct S2 {
        struct S1 inner;
        float f;
    } s2 = {
        .inner = {.a = 7, .b = {8, 9}},
        .f = 3.14f
    };
    
    /* Union with designated initializer */
    union U {
        int i;
        float f;
        char c[4];
    } u = {.c = {'a', 'b', 'c', '\0'}};
    
    sink = s1.a + arr[5] + (int)s2.f + u.c[0];
}

/* Generate TREE_VEC nodes using vector extensions */
#ifdef __GNUC__
void tree_vec_test(void) {
    /* Vector types using GCC extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    /* Mixed operations */
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = f1 * 2.0f;
    
    /* Array compound literal */
    int *p = (int[]){10, 20, 30, 40};
    
    /* Vector shuffle */
    v4si e = __builtin_shuffle(a, b, (v4si){0, 4, 1, 5});
    
    sink = c[0] + d[1] + (int)f2[2] + p[3] + e[2];
}
#endif

/* OpenMP test for OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_test(int n) {
    int i, sum = 0;
    int private_var = 0;
    static int shared_var = 0;
    
    /* Multiple clauses in single pragma */
    #pragma omp parallel for private(i) private(private_var) \
            shared(shared_var) reduction(+:sum) \
            schedule(dynamic, 2) num_threads(4) \
            firstprivate(n) if(n > 100)
    for (i = 0; i < n; i++) {
        private_var = i * 2;
        sum += private_var;
        #pragma omp atomic
        shared_var++;
    }
    
    /* Another pragma with different clauses */
    int arr[100];
    #pragma omp parallel sections private(i) \
            nowait
    {
        #pragma omp section
        {
            for (i = 0; i < 50; i++) {
                arr[i] = i;
            }
        }
        
        #pragma omp section
        {
            for (i = 50; i < 100; i++) {
                arr[i] = i * 2;
            }
        }
    }
    
    sink = sum + shared_var + arr[42];
}
#endif

/* Main test function */
int main(void) {
    int checksum = 0;
    
    /* Test 1: IDENTIFIER_NODE - deeply nested scopes with same variable names */
    {
        volatile int result = 0;
        
        /* Multiple scopes with same variable name */
        {
            int x = 1;
            result += x;
            
            {
                /* Shadowing x */
                int x = 2;
                result += x;
                
                {
                    /* Another shadow */
                    volatile int x = 3;
                    result += x;
                    
                    /* Reference to outer x through pointer */
                    {
                        int *p = &x;
                        result += *p;
                    }
                }
            }
            
            /* Function scope with parameter */
            auto int func(int x) {
                return x * 2;
            }
            result += func(x);
        }
        
        /* More identifier complexity */
        {
            /* Use external identifier */
            extern int external_var;
            volatile int y = external_var;
            result += y;
            
            /* Multiple declarations */
            {
                int z = 5;
                {
                    /* Different type, same name */
                    float z = 3.14f;
                    result += (int)z;
                }
                result += z;
            }
        }
        
        checksum += result;
    }
    
    /* Test 2: SSA_NAME nodes */
    checksum += ssa_test(20);
    
    /* Test 3: BLOCK nodes */
    block_test();
    checksum += sink;
    
    /* Test 4: CONSTRUCTOR nodes */
    constructor_test();
    checksum += sink;
    
    /* Test 5: TREE_VEC nodes */
    #ifdef __GNUC__
    tree_vec_test();
    checksum += sink;
    #endif
    
    /* Test 6: OpenMP clauses */
    #ifdef _OPENMP
    omp_test(200);
    checksum += sink;
    #endif
    
    printf("Tree node test checksum: %d\n", checksum);
    
    return 0;
}

/* C++ specific tests for TREE_BINFO */
#ifdef __cplusplus

/* Base class for BINFO testing */
struct Base {
    int a;
    float b;
    virtual void vfunc() {}
};

/* Derived class with inheritance */
struct Derived : Base {
    int c;
    double d;
    virtual void vfunc() override {}
};

/* More complex hierarchy */
struct Base2 {
    int e;
    virtual void vfunc2() = 0;
};

struct MultiDerived : Base, Base2 {
    int f;
    virtual void vfunc() override {}
    virtual void vfunc2() override {}
};

/* Template class for additional complexity */
template<typename T>
struct TemplateBase {
    T value;
    virtual T get_value() { return value; }
};

struct ConcreteDerived : TemplateBase<int> {
    int extra;
    virtual int get_value() override { return value + extra; }
};

/* Function using inheritance to generate BINFO nodes */
void binfo_test() {
    Derived d;
    d.a = 1;        /* Access base member */
    d.c = 2;
    
    MultiDerived md;
    md.a = 3;
    md.e = 4;
    
    Base* bp = &d;
    bp->vfunc();    /* Virtual call */
    
    Base2* bp2 = &md;
    bp2->vfunc2();
    
    ConcreteDerived cd;
    cd.value = 10;
    cd.extra = 5;
    int val = cd.get_value();
    
    sink = d.a + d.c + md.a + md.e + val;
}

/* C++ main wrapper */
int main_cpp() {
    int checksum = main();
    
    /* Run BINFO test */
    binfo_test();
    checksum += sink;
    
    printf("C++ BINFO test added, total checksum: %d\n", checksum);
    return 0;
}

/* Use C++ main if compiled as C++ */
#ifdef __cplusplus
#ifndef __MAIN_DEFINED
#define __MAIN_DEFINED
int main() {
    return main_cpp();
}
#endif
#endif

#endif /* __cplusplus */
