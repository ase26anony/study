/* test_tree.c - Comprehensive tree node coverage test */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func(int);
extern volatile int external_var;

/* Sink function to prevent optimization */
static void sink(volatile void *ptr) {
    (void)ptr;
}

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

void use_hierarchy() {
    DeepDerived dd;
    dd.a = 1;      /* Accesses through inheritance chain */
    dd.b = 2;
    dd.c = 3;
    Base *bp = &dd;
    bp->vfunc();   /* Virtual call */
}
#endif

/* Function to generate SSA_NAME nodes */
int complex_control_flow(int n) {
    int i, s = 0, t = 1;
    
    /* Complex loop with multiple assignments */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t += 2;
        } else {
            s -= i / (t + 1);
            t *= 3;
        }
        
        /* Nested condition */
        switch (i % 4) {
            case 0: s += 100; break;
            case 1: s -= 50; break;
            case 2: s *= 2; break;
            case 3: s /= 2; break;
        }
    }
    
    /* Another loop with phi nodes */
    int j = 0, k = 0;
    while (j < n) {
        if (j % 3 == 0) {
            k = j * 2;
        } else if (j % 3 == 1) {
            k = j + 5;
        } else {
            k = j - 3;
        }
        s += k;
        j++;
    }
    
    return s;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
int nested_scopes_test(void) {
    int checksum = 0;
    
    /* Level 1 */
    {
        volatile int x = 1;
        checksum += x;
        
        /* Level 2 - shadowing x */
        {
            extern int x;  /* Different x */
            volatile int y = x + 2;
            checksum += y;
            
            /* Level 3 - another x */
            {
                static int x = 3;
                volatile int z = x * 2;
                checksum += z;
                
                /* Level 4 - yet another x */
                {
                    auto int x = 4;
                    volatile int w = x / 2;
                    checksum += w;
                }
            }
        }
        
        /* Another branch of nesting */
        {
            register int x = 5;
            volatile int y = x << 1;
            checksum += y;
        }
    }
    
    /* Function scope with parameter shadowing */
    {
        auto int test_shadow(int x) {
            {
                long x = (long)x * 2;  /* Shadows parameter */
                return (int)x;
            }
        }
        checksum += test_shadow(10);
    }
    
    return checksum;
}

/* Function to generate TREE_VEC nodes */
int vector_operations(void) {
    int checksum = 0;
    
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c - d;
    
    /* Store to volatile to prevent optimization */
    volatile v4si sink_vec = e;
    checksum += sink_vec[0] + sink_vec[1] + sink_vec[2] + sink_vec[3];
    
    /* Float vectors */
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf f3 = f1 * f2;
    volatile v4sf sink_fvec = f3;
    
    /* Array compound literals (also generate TREE_VEC) */
    int *arr1 = (int[]){1, 2, 3, 4, 5};
    int *arr2 = (int[3]){10, 20, 30};
    struct Point { int x; int y; };
    struct Point *points = (struct Point[]){{1,2}, {3,4}, {5,6}};
    
    checksum += arr1[0] + arr2[1] + points[2].x;
    
    /* Multi-dimensional array literal */
    int (*mdarr)[3] = (int[2][3]){{1,2,3}, {4,5,6}};
    checksum += mdarr[1][2];
    
    return checksum;
}

/* Function with BLOCK nodes and labels */
int block_and_label_test(void) {
    int checksum = 0;
    volatile int a = 0;
    
    /* Block 1 */
    {
        int x = 1;
    lab1:
        x++;
        checksum += x;
        
        /* Nested block */
        {
            int y = 2;
            if (x > 0)
                goto lab2;  /* Jump to outer block */
            y++;
        }
        
        x += 3;
    }
    
    /* Block 2 */
    {
        int z = 5;
    lab2:
        z *= 2;
        checksum += z;
        
        /* Deep nesting with multiple labels */
        {
            int w = 10;
        lab3:
            w--;
            if (w > 0)
                goto lab3;  /* Local loop */
            
            /* Switch with case labels as blocks */
            switch (z) {
                case 10: {
                    int inner = 100;
                    checksum += inner;
                    break;
                }
                default: {
                    int inner = 200;
                    checksum += inner;
                    goto lab4;
                }
            }
        }
    }
    
lab4:
    a = checksum;
    
    /* Computed goto (GCC extension) */
    {
        static void *labels[] = { &&l1, &&l2, &&l3 };
        int idx = checksum % 3;
        
        goto *labels[idx];
        
    l1:
        checksum += 1000;
        goto end;
    l2:
        checksum += 2000;
        goto end;
    l3:
        checksum += 3000;
        goto end;
    }
    
end:
    return checksum + a;
}

/* Function to generate CONSTRUCTOR nodes */
int constructor_test(void) {
    int checksum = 0;
    
    /* Struct with designated initializers */
    struct S {
        int a;
        int b[3];
        struct {
            int x;
            int y;
        } nested;
    };
    
    struct S s1 = { 
        .a = 1, 
        .b = {[1] = 5, [0] = 3, [2] = 7},
        .nested = { .x = 10, .y = 20 }
    };
    checksum += s1.a + s1.b[1] + s1.nested.x;
    
    /* Partial initialization */
    struct S s2 = { .b = {[2] = 100} };
    checksum += s2.b[2];
    
    /* Array with designated initializers */
    int arr[10] = { [0] = 1, [5] = 2, [9] = 3 };
    checksum += arr[5];
    
    /* Nested struct initialization */
    struct Outer {
        struct Inner {
            int a;
            int b;
        } inner;
        int c;
    };
    
    struct Outer o1 = { .inner = { .a = 1, .b = 2 }, .c = 3 };
    struct Outer o2 = { .inner.a = 4, .c = 5 };  /* Partial nested */
    checksum += o1.inner.a + o2.c;
    
    /* Union initializers */
    union U {
        int i;
        float f;
        struct { short s1; short s2; } ss;
    };
    
    union U u1 = { .i = 42 };
    union U u2 = { .f = 3.14f };
    union U u3 = { .ss = { .s1 = 1, .s2 = 2 } };
    checksum += u1.i + (int)u2.f + u3.ss.s1;
    
    /* Zero initialization */
    struct S s3 = {0};
    checksum += s3.a;
    
    return checksum;
}

/* OpenMP section for OMP_CLAUSE nodes */
#ifdef _OPENMP
#include <omp.h>

int openmp_test(int n) {
    int sum = 0;
    int i;
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel default(none) private(i) shared(n, sum) \
                         num_threads(4) if(n > 1000)
    {
        int tid = omp_get_thread_num();
        
        /* For loop with schedule clause */
        #pragma omp for schedule(dynamic, 2) reduction(+:sum) \
                         collapse(1) nowait
        for (i = 0; i < n; i++) {
            sum += i * (tid + 1);
        }
        
        /* Barrier */
        #pragma omp barrier
        
        /* Single construct */
        #pragma omp single copyprivate(tid) nowait
        {
            sum += 1000;
        }
        
        /* Sections with different clauses */
        #pragma omp sections private(i) lastprivate(sum)
        {
            #pragma omp section
            {
                for (i = 0; i < 10; i++) sum += i;
            }
            
            #pragma omp section
            {
                sum *= 2;
            }
        }
        
        /* Task construct */
        #pragma omp task shared(sum) if(n > 500) untied mergeable
        {
            sum += 500;
        }
        
        /* Taskwait */
        #pragma omp taskwait
        
        /* Atomic operation */
        #pragma omp atomic update
        sum += 1;
        
        /* Critical section */
        #pragma omp critical (my_critical)
        {
            sum += tid;
        }
        
        /* Ordered directive */
        #pragma omp ordered
        {
            sum -= 1;
        }
    }
    
    /* Parallel for with firstprivate/lastprivate */
    int base = 10;
    #pragma omp parallel for firstprivate(base) lastprivate(i) \
                             linear(base:1) ordered
    for (i = 0; i < n; i++) {
        base += i;
        #pragma omp ordered
        sum += base;
    }
    
    return sum;
}
#endif

int main(void) {
    int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* 1. Test IDENTIFIER_NODE generation */
    printf("Testing IDENTIFIER_NODE...\n");
    checksum += nested_scopes_test();
    
    /* 2. Test TREE_VEC generation */
    printf("Testing TREE_VEC...\n");
    checksum += vector_operations();
    
    /* 3. Test BLOCK nodes */
    printf("Testing BLOCK nodes...\n");
    checksum += block_and_label_test();
    
    /* 4. Test CONSTRUCTOR nodes */
    printf("Testing CONSTRUCTOR nodes...\n");
    checksum += constructor_test();
    
    /* 5. Test SSA_NAME generation */
    printf("Testing SSA_NAME...\n");
    checksum += complex_control_flow(50);
    
#ifdef __cplusplus
    /* 6. Test TREE_BINFO generation (C++ only) */
    printf("Testing TREE_BINFO...\n");
    use_hierarchy();
    checksum += 42;  /* Add something to checksum */
#endif
    
#ifdef _OPENMP
    /* 7. Test OMP_CLAUSE nodes */
    printf("Testing OMP_CLAUSE...\n");
    checksum += openmp_test(100);
#endif
    
    /* Use external symbols to create unresolved identifiers */
    checksum += external_var;
    checksum += external_func(checksum);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Prevent dead code elimination */
    sink(&checksum);
    
    return checksum != 0 ? 0 : 1;
}
