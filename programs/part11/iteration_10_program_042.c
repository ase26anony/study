/* test_tree.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func(int);
extern volatile int external_var;

#ifdef __cplusplus
/* C++ specific code for BINFO nodes */
struct Base {
    int a;
    virtual void vfunc() {}
};

struct Derived : Base {
    int b;
    void vfunc() override {}
};

struct DeepDerived : Derived {
    int c;
};

void use_inheritance() {
    Derived d;
    d.a = 1;  // Access base member
    d.b = 2;
    
    DeepDerived dd;
    dd.a = 3;  // Access through multiple inheritance
    dd.b = 4;
    dd.c = 5;
    
    Base* bp = &d;
    bp->vfunc();  // Virtual call
}
#endif

/* Force IDENTIFIER_NODE creation with shadowing */
void test_identifiers() {
    volatile int result = 0;
    
    /* Multiple scopes with same variable name */
    {
        int x = 1;
        result += x;
        
        {
            /* Different type, same name */
            float x = 2.0f;
            result += (int)x;
            
            {
                /* Pointer with same name */
                int* x = (int[]){1, 2, 3};
                result += x[0];
                
                {
                    /* Reference to external with same name */
                    extern int x;  /* Unresolved identifier */
                    volatile int y = external_var;  /* Use external */
                    result += y;
                }
            }
        }
    }
    
    /* Function parameter shadowing */
    {
        auto func = [](int x) -> int {
            {
                long x = x * 2;  /* Shadow parameter */
                return (int)x;
            }
        };
        result += func(5);
    }
    
    printf("Identifier test result: %d\n", result);
}

/* Create TREE_VEC nodes */
void test_tree_vec() {
    volatile int result = 0;
    
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    /* Extract elements */
    for (int i = 0; i < 4; i++) {
        result += c[i];
        result += d[i];
    }
    
    /* Float vectors */
    v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fb = fa * 2.0f;
    
    /* Array compound literals */
    int* arr1 = (int[]){1, 2, 3, 4, 5};
    int* arr2 = (int[]){[0] = 10, [2] = 20, [4] = 30};
    
    for (int i = 0; i < 5; i++) {
        result += arr1[i];
        result += arr2[i];
    }
    
    /* Nested compound literals */
    struct Point { int x; int y; };
    struct Point* points = (struct Point[]){{1, 2}, {3, 4}, {5, 6}};
    result += points[1].x;
    
    printf("Tree_vec test result: %d\n", result);
}

/* Force SSA_NAME creation */
int test_ssa_names(int n) {
    volatile int result = 0;
    
    /* Complex control flow with many assignments */
    int i, s = 0, t = 1, u = 2;
    
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t = u + 1;
        } else {
            s *= 2 + u;
            u = t - 1;
        }
        
        /* Nested condition */
        switch (i % 3) {
            case 0: s += 1; break;
            case 1: s += t; break;
            case 2: s += u; break;
        }
    }
    
    /* Another loop with phi nodes */
    int x = 0, y = 1;
    for (i = 0; i < 10; i++) {
        if (x > y) {
            x = y + i;
        } else {
            y = x + i * 2;
        }
        result += x + y;
    }
    
    result += s;
    return result;
}

/* Generate BLOCK nodes */
int test_blocks() {
    volatile int result = 0;
    
    /* Deeply nested blocks with labels */
    {
        int a = 1;
    label1:
        a++;
        {
            int b = 2;
            if (a < 10) {
                goto label2;
            }
        label3:
            b = 3;
            {
                int c = 4;
            label4:
                c += a + b;
                result += c;
                if (c < 100) {
                    goto label1;
                }
            }
        }
    label2:
        {
            int d = 5;
            goto label3;
        }
    }
    
    /* Switch with blocks */
    int val = 5;
    switch (val) {
        case 1: {
            int block_var = 10;
            result += block_var;
            break;
        }
        case 5: {
            int block_var = 20;  /* Same name, different scope */
            {
                int block_var = 30;  /* Shadowing */
                result += block_var;
            }
            result += block_var;
            break;
        }
        default: {
            int block_var = 40;
            result += block_var;
        }
    }
    
    printf("Blocks test result: %d\n", result);
    return result;
}

/* Construct CONSTRUCTOR nodes */
void test_constructors() {
    volatile int result = 0;
    
    /* Struct with designated initializers */
    struct S {
        int a;
        int b[3];
        struct {
            int x;
            int y;
        } nested;
    };
    
    struct S s1 = { .a = 1, .b = {[0] = 2, [2] = 4}, .nested = {.x = 5, .y = 6} };
    struct S s2 = { .a = 10, .b = {[1] = 20}, .nested.x = 30 };  /* Partial */
    
    result += s1.a + s1.b[0] + s1.b[2] + s1.nested.x;
    result += s2.a + s2.b[1] + s2.nested.x;
    
    /* Array with designated initializers */
    int arr[10] = {[0] = 1, [5] = 2, [9] = 3, [2 ... 4] = 4};
    for (int i = 0; i < 10; i++) {
        result += arr[i];
    }
    
    /* Union initializers */
    union U {
        int i;
        float f;
        struct { short a; short b; } s;
    };
    
    union U u1 = { .i = 42 };
    union U u2 = { .f = 3.14f };
    union U u3 = { .s = {.a = 1, .b = 2} };
    
    result += u1.i + (int)u2.f + u3.s.a + u3.s.b;
    
    /* Nested constructors */
    struct Complex {
        struct S s;
        union U u;
        int arr[2][3];
    };
    
    struct Complex c = {
        .s = { .a = 100, .b = {[0] = 200} },
        .u = { .s = {.a = 300, .b = 400} },
        .arr = { {1, 2}, {[2] = 3} }
    };
    
    result += c.s.a + c.s.b[0] + c.u.s.a + c.arr[0][1];
    
    printf("Constructors test result: %d\n", result);
}

/* Generate OMP_CLAUSE nodes */
void test_omp_clauses() {
    volatile int result = 0;
    int i;
    
    /* Multiple OpenMP pragmas with various clauses */
    
    /* Parallel region with many clauses */
    #pragma omp parallel private(i) shared(result) num_threads(4) if(1)
    {
        #pragma omp critical
        {
            result++;
        }
    }
    
    /* Parallel for with reduction and schedule */
    #pragma omp parallel for private(i) reduction(+:result) schedule(dynamic, 2)
    for (i = 0; i < 100; i++) {
        result += i % 7;
    }
    
    /* Sections with nowait */
    #pragma omp parallel
    {
        #pragma omp sections private(i) nowait
        {
            #pragma omp section
            {
                for (i = 0; i < 50; i++) {
                    #pragma omp atomic
                    result += 1;
                }
            }
            
            #pragma omp section
            {
                for (i = 0; i < 50; i++) {
                    #pragma omp atomic
                    result += 2;
                }
            }
        }
    }
    
    /* Single with copyprivate */
    int shared_var = 0;
    #pragma omp parallel private(i)
    {
        int local_var = 0;
        
        #pragma omp single copyprivate(local_var)
        {
            local_var = 42;
        }
        
        #pragma omp atomic
        shared_var += local_var;
    }
    result += shared_var;
    
    /* Task with depend clauses */
    int a = 0, b = 0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: a)
            { a = 10; }
            
            #pragma omp task depend(in: a) depend(out: b)
            { b = a * 2; }
            
            #pragma omp task depend(in: b)
            { result += b; }
        }
    }
    
    printf("OMP clauses test result: %d\n", result);
}

/* Main test driver */
int main() {
    volatile int total = 0;
    
    printf("=== Testing GCC Tree Node Coverage ===\n");
    
    /* Test each tree node type */
    test_identifiers();
    total += 1;
    
    test_tree_vec();
    total += 2;
    
    int ssa_result = test_ssa_names(20);
    printf("SSA test result: %d\n", ssa_result);
    total += ssa_result;
    
    int block_result = test_blocks();
    total += block_result;
    
    test_constructors();
    total += 4;
    
    #ifdef _OPENMP
    test_omp_clauses();
    total += 8;
    #endif
    
    #ifdef __cplusplus
    use_inheritance();
    total += 16;
    printf("C++ BINFO nodes tested\n");
    #endif
    
    /* Use external function to create unresolved identifier */
    total += external_func(total);
    
    printf("\nTotal checksum: %d\n", total);
    printf("=== Test Complete ===\n");
    
    return total % 256;
}
