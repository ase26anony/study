/* test_tree.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(int);
extern void external_func2(void);
extern volatile int external_var;

/* Helper to prevent optimization */
static volatile int sink;

/* Function to create SSA_NAME nodes with complex control flow */
int create_ssa_names(int n) {
    int i, s = 0, t = 1, u = 2;
    
    /* Complex loop with multiple assignments */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t = u + 1;
        } else {
            s *= 2;
            u = t - 1;
        }
        
        /* Nested condition */
        if (s > 100) {
            t = s / 2;
        } else if (s < 0) {
            u = s * 3;
        } else {
            t = u = s;
        }
    }
    
    /* Another loop with phi nodes */
    int j = 0, k = 0;
    while (j < n) {
        if (j % 3 == 0) {
            k += j;
        } else if (j % 3 == 1) {
            k -= j;
        } else {
            k *= 2;
        }
        j++;
    }
    
    return s + t + u + k;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
int create_identifiers(void) {
    int result = 0;
    
    /* Level 1 */
    {
        int x = 1;
        result += x;
        
        /* Level 2 */
        {
            /* Different x in inner scope */
            float x = 2.5f;
            result += (int)x;
            
            /* Level 3 */
            {
                /* Another x */
                volatile int x = 3;
                result += x;
                
                /* Level 4 - extern declaration */
                {
                    extern int x;  /* Unresolved identifier */
                    volatile int y = x;  /* Forces identifier node */
                    result += y;
                }
            }
        }
        
        /* Another block at level 2 */
        {
            /* Yet another x */
            double x = 4.75;
            result += (int)x;
            
            /* Array with same name */
            {
                char x[] = "test";
                result += x[0];
            }
        }
    }
    
    /* Function scope variables with same name */
    {
        volatile int counter = 0;
        for (int i = 0; i < 5; i++) {
            /* Loop scope variable */
            int counter = i * 2;
            result += counter;
            
            /* Nested loop with same name */
            for (int j = 0; j < 2; j++) {
                int counter = i + j;
                result += counter;
            }
        }
    }
    
    return result;
}

/* Function to create TREE_VEC nodes using vector extensions */
#ifdef __GNUC__
int create_tree_vec(void) {
    int result = 0;
    
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    /* Store to volatile to prevent optimization */
    volatile v4si cv = c;
    volatile v4si dv = d;
    
    /* Access elements */
    for (int i = 0; i < 4; i++) {
        result += c[i] + d[i];
    }
    
    /* Float vectors */
    v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fb = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fc = fa * fb;
    
    volatile v4sf fcv = fc;
    for (int i = 0; i < 4; i++) {
        result += (int)fc[i];
    }
    
    /* Array compound literals */
    int *arr1 = (int[]){1, 2, 3, 4, 5};
    int *arr2 = (int[3]){10, 20, 30};
    
    /* Nested compound literals */
    struct Point {
        int x;
        int y;
    };
    struct Point *points = (struct Point[]){
        {.x = 1, .y = 2},
        {.x = 3, .y = 4},
        {.x = 5, .y = 6}
    };
    
    for (int i = 0; i < 3; i++) {
        result += points[i].x + points[i].y;
    }
    
    /* Multi-dimensional compound literal */
    int (*matrix)[3] = (int[2][3]){
        {1, 2, 3},
        {4, 5, 6}
    };
    
    result += matrix[0][0] + matrix[1][2];
    
    return result;
}
#else
int create_tree_vec(void) {
    /* Fallback without vector extensions */
    int result = 0;
    
    /* Still use compound literals */
    int *arr = (int[]){1, 2, 3};
    result = arr[0] + arr[1] + arr[2];
    
    return result;
}
#endif

/* Function with complex blocks for BLOCK nodes */
int create_blocks(void) {
    int result = 0;
    int a = 0;
    
    /* Block 1 */
    {
        int b = 1;
        result += b;
        
    block1_label:
        b++;
        
        /* Nested block */
        {
            int c = 2;
        block2_label:
            c++;
            
            /* Jump to outer block */
            if (c < 5) goto block1_label;
            
            result += c;
        }
        
        /* Another nested block with goto */
        {
            int d = 3;
            if (b < 10) goto block2_label;
            result += d;
        }
    }
    
    /* Switch with blocks */
    switch (result % 3) {
        case 0: {
            int x = 10;
        case0_label:
            x++;
            result += x;
            break;
        }
        case 1: {
            int y = 20;
            goto case0_label;  /* Cross-block goto */
        }
        case 2: {
            int z = 30;
            result += z;
            break;
        }
    }
    
    /* Loop with labeled blocks */
    for (int i = 0; i < 3; i++) {
        {
            int inner = i * 2;
        loop_label:
            inner++;
            result += inner;
            
            if (inner < 10) {
                goto loop_label;
            }
        }
    }
    
    return result;
}

/* Function to create CONSTRUCTOR nodes */
int create_constructors(void) {
    int result = 0;
    
    /* Struct with designated initializers */
    struct S {
        int a;
        int b;
        int c[3];
        struct {
            int x;
            int y;
        } nested;
    };
    
    struct S s1 = {
        .a = 1,
        .b = 2,
        .c = {[0] = 10, [2] = 30},  /* Partial array initialization */
        .nested = {.x = 100, .y = 200}
    };
    
    result += s1.a + s1.b + s1.c[0] + s1.c[2] + s1.nested.x;
    
    /* Nested designated initializers */
    struct S s2 = {
        .a = 5,
        .c = {[1] = 50},  /* Only middle element initialized */
        .nested.y = 300   /* Only y initialized */
    };
    
    result += s2.a + s2.c[1] + s2.nested.y;
    
    /* Array of structs with designated initializers */
    struct S arr[3] = {
        [0] = {.a = 1, .b = 2},
        [2] = {.a = 3, .c = {[0] = 30}}
    };
    
    result += arr[0].a + arr[2].a + arr[2].c[0];
    
    /* Union initializers */
    union U {
        int i;
        float f;
        char c[4];
    };
    
    union U u1 = {.i = 0x12345678};
    union U u2 = {.f = 3.14f};
    union U u3 = {.c = {'a', 'b', 'c', '\0'}};
    
    result += u1.i + (int)u2.f + u3.c[0];
    
    /* Complex nested initializer */
    struct Complex {
        struct {
            int a[2][2];
        } inner;
        int b;
    };
    
    struct Complex comp = {
        .inner.a = {[0][0] = 1, [1][1] = 4},
        .b = 99
    };
    
    result += comp.inner.a[0][0] + comp.inner.a[1][1] + comp.b;
    
    return result;
}

/* OpenMP section for OMP_CLAUSE nodes */
#ifdef _OPENMP
int create_omp_clauses(void) {
    int result = 0;
    int i;
    
    /* Test 1: Parallel for with multiple clauses */
    #pragma omp parallel for private(i) shared(result) \
        reduction(+:result) schedule(dynamic, 2) \
        num_threads(4) if(1)
    for (i = 0; i < 100; i++) {
        result += i;
    }
    
    /* Test 2: Parallel region with clauses */
    #pragma omp parallel default(none) shared(result) \
        firstprivate(i) copyin(result)
    {
        #pragma omp single copyprivate(result)
        {
            result += 42;
        }
    }
    
    /* Test 3: Sections with nowait */
    #pragma omp parallel
    {
        #pragma omp sections nowait
        {
            #pragma omp section
            {
                result += 1;
            }
            #pragma omp section
            {
                result += 2;
            }
        }
        
        #pragma omp barrier
    }
    
    /* Test 4: Task with dependencies */
    int a = 0, b = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: a)
        {
            a = 10;
        }
        
        #pragma omp task depend(in: a) depend(out: b)
        {
            b = a + 5;
        }
        
        #pragma omp task depend(in: b)
        {
            result += b;
        }
        
        #pragma omp taskwait
    }
    
    /* Test 5: Target directives */
    #pragma omp target map(tofrom: result) device(0)
    {
        result += 1000;
    }
    
    return result;
}
#else
int create_omp_clauses(void) {
    /* Fallback without OpenMP */
    return 42;
}
#endif

#ifdef __cplusplus
/* C++ specific code for TREE_BINFO nodes */
class Base1 {
public:
    int a;
    virtual void func1() { a = 1; }
};

class Base2 {
public:
    int b;
    virtual void func2() { b = 2; }
};

class Derived : public Base1, public Base2 {
public:
    int c;
    virtual void func1() override { a = 10; }
    virtual void func2() override { b = 20; }
    void func3() { c = a + b; }
};

class DeepDerived : public Derived {
public:
    int d;
    virtual void func1() override { a = 100; }
    void func4() { d = a * b; }
};

int create_binfo_nodes(void) {
    Derived d;
    DeepDerived dd;
    
    Base1* b1 = &d;
    Base2* b2 = &d;
    Base1* b3 = &dd;
    
    b1->func1();
    b2->func2();
    b3->func1();
    
    d.func3();
    dd.func4();
    
    return d.a + d.b + d.c + dd.d;
}
#else
int create_binfo_nodes(void) {
    /* Not in C++ mode */
    return 0;
}
#endif

int main(void) {
    int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* Test 1: IDENTIFIER_NODE */
    checksum += create_identifiers();
    printf("IDENTIFIER_NODE test complete\n");
    
    /* Test 2: TREE_VEC */
    checksum += create_tree_vec();
    printf("TREE_VEC test complete\n");
    
    /* Test 3: SSA_NAME */
    checksum += create_ssa_names(20);
    printf("SSA_NAME test complete\n");
    
    /* Test 4: BLOCK */
    checksum += create_blocks();
    printf("BLOCK test complete\n");
    
    /* Test 5: CONSTRUCTOR */
    checksum += create_constructors();
    printf("CONSTRUCTOR test complete\n");
    
    /* Test 6: OMP_CLAUSE */
    checksum += create_omp_clauses();
    printf("OMP_CLAUSE test complete\n");
    
    /* Test 7: TREE_BINFO (C++ only) */
#ifdef __cplusplus
    checksum += create_binfo_nodes();
    printf("TREE_BINFO test complete\n");
#endif
    
    /* Use external identifiers */
    checksum += external_var;
    external_func1(checksum);
    external_func2();
    
    /* Final sink to prevent optimization */
    sink = checksum;
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed successfully!\n");
    
    return checksum == 0 ? 0 : 1;
}
