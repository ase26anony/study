/* test_tree.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(void);
extern void external_func2(int);
extern double external_func3(float);

/* Volatile sink to prevent optimization */
volatile int global_sink = 0;

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
    void vfunc() override {}
};

void test_binfo() {
    Derived d;
    d.a = 1;  /* Accesses through inheritance */
    d.b = 2;
    
    DeepDerived dd;
    dd.a = 3;  /* Multiple inheritance levels */
    dd.b = 4;
    dd.c = 5;
    
    Base* bp = &d;
    bp->vfunc();  /* Virtual call */
    
    global_sink += d.a + dd.c;
}
#endif

/* Test for IDENTIFIER_NODE - deeply nested scopes with same names */
int test_identifier_nodes() {
    int result = 0;
    
    /* Level 1 */
    {
        int x = 1;
        volatile int y = x;
        result += y;
        
        /* Level 2 */
        {
            extern int x;  /* Different x - external linkage */
            volatile int y = 2;
            result += y;
            
            /* Level 3 */
            {
                static int x = 3;  /* Static x */
                volatile int y = x;
                result += y;
                
                /* Level 4 - function scope */
                auto int inner_func() {
                    int x = 4;  /* Local x in nested function (GCC extension) */
                    volatile int y = x;
                    return y;
                }
                result += inner_func();
            }
        }
    }
    
    /* More shadowing with loops */
    for (int i = 0; i < 3; i++) {
        int counter = i * 10;
        {
            int counter = i * 20;  /* Shadows outer counter */
            volatile int sink = counter;
            result += sink;
        }
        result += counter;
    }
    
    return result;
}

/* Test for TREE_VEC nodes - vector extensions and compound literals */
int test_tree_vec() {
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
        result += cv[i] + dv[i];
    }
    
    /* More complex vector operations */
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf f3 = f1 * f2;
    v4sf f4 = __builtin_ia32_sqrtps(f3);  /* SSE intrinsic */
    
    volatile v4sf f4v = f4;
    for (int i = 0; i < 4; i++) {
        result += (int)f4v[i];
    }
    
    /* Array compound literals */
    int *arr1 = (int[]){1, 2, 3, 4, 5};
    int *arr2 = (int[3]){10, 20, 30};
    struct { int x; int y; } *p = (struct { int x; int y; }[]){{1,2}, {3,4}};
    
    for (int i = 0; i < 5; i++) result += arr1[i];
    for (int i = 0; i < 3; i++) result += arr2[i];
    result += p[0].x + p[1].y;
    
    /* Nested compound literals */
    int **nested = (int*[]){(int[]){1,2}, (int[]){3,4,5}};
    result += nested[0][1] + nested[1][2];
    
    return result;
}

/* Test for SSA_NAME creation - complex control flow */
int test_ssa_name(int n) {
    int i, s = 0, t = 1, u = 2;
    
    /* Complex loop with multiple updates */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t = u + 1;
        } else {
            s *= 2 + u;
            u = t - 1;
        }
        
        /* Nested condition */
        if (s > 100) {
            t = s / 2;
            if (u < 50) {
                u = u * 2 + t;
            }
        } else if (s < -50) {
            u = s + t;
            t = u * 3;
        }
        
        /* Switch inside loop */
        switch (i % 4) {
            case 0: s += t; break;
            case 1: s -= u; t++; break;
            case 2: s *= 2; u--; break;
            case 3: s /= 2; t = u; break;
        }
    }
    
    /* Another loop with phi nodes */
    int x = 0, y = 1;
    for (i = 0; i < 10; i++) {
        if (x > y) {
            x = y + i;
        } else {
            y = x + i;
        }
        s += x + y;
    }
    
    /* Conditional returns create merge points */
    if (s > 1000) {
        return s + t + u;
    } else if (s > 500) {
        return s * t - u;
    } else {
        return s + t * u;
    }
}

/* Test for BLOCK nodes - nested blocks with labels and gotos */
int test_block_nodes() {
    int result = 0;
    
    /* Block 1 */
    {
        int a = 1;
    label1:
        a = a * 2;
        
        /* Block 2 */
        {
            int b = 3;
            volatile int sink = b;
            result += sink;
            goto label3;  /* Jump forward */
            
        label2:
            b = b + a;
            result += b;
        }
        
        /* Block 3 */
        {
            int c = 5;
        label3:
            c = c + a;
            result += c;
            goto label4;
        }
    }
    
    /* Block 4 */
    {
        int d = 7;
    label4:
        d = d * 3;
        result += d;
        goto label2;  /* Jump backward - requires careful analysis */
    }
    
    /* More complex block structure */
    {
        int x = 0;
        
        /* Loop with nested blocks */
        for (int i = 0; i < 5; i++) {
            /* Block inside loop */
            {
                int temp = i * 2;
                if (temp > 3) {
                    /* Block inside if */
                    {
                        volatile int inner = temp;
                        x += inner;
                        goto loop_end;
                    }
                }
                x += temp;
            }
        loop_end:
            continue;
        }
        result += x;
    }
    
    return result;
}

/* Test for CONSTRUCTOR nodes - designated initializers */
int test_constructor_nodes() {
    int result = 0;
    
    /* Struct with designated initializers */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Line {
        struct Point start;
        struct Point end;
        int id;
    };
    
    /* Partial initialization */
    struct Point p1 = { .x = 1, .z = 3 };  /* y will be 0 */
    struct Point p2 = { .y = 2 };  /* x and z will be 0 */
    
    result += p1.x + p1.z + p2.y;
    
    /* Nested designated initializers */
    struct Line l1 = {
        .start = { .x = 1, .y = 2 },
        .end = { .z = 3 },
        .id = 100
    };
    
    result += l1.start.x + l1.end.z + l1.id;
    
    /* Array with designated initializers */
    int arr1[10] = { [0] = 1, [5] = 2, [9] = 3 };
    int arr2[] = { [0 ... 4] = 10, [5 ... 9] = 20 };
    
    for (int i = 0; i < 10; i++) {
        result += arr1[i] + arr2[i];
    }
    
    /* Complex nested initialization */
    struct Node {
        int value;
        struct Node* children[3];
        struct {
            int id;
            char tag;
        } meta;
    };
    
    struct Node n1 = {
        .value = 42,
        .children = { NULL, NULL, NULL },
        .meta = { .id = 1, .tag = 'A' }
    };
    
    result += n1.value + n1.meta.id;
    
    /* Union initializers */
    union Data {
        int i;
        float f;
        char str[4];
    };
    
    union Data d1 = { .i = 255 };
    union Data d2 = { .f = 3.14f };
    union Data d3 = { .str = "ABC" };
    
    result += d1.i + (int)d2.f + d3.str[0];
    
    /* Zero initialization with designators */
    struct BigStruct {
        int a[100];
        struct {
            int x;
            int y;
        } points[50];
    };
    
    struct BigStruct bs = { .a[10] = 1, .points[25].x = 2 };
    result += bs.a[10] + bs.points[25].x;
    
    return result;
}

/* Test for OMP_CLAUSE nodes - OpenMP pragmas */
int test_omp_clause(int n) {
    int sum = 0;
    int i;
    
    /* Test 1: Parallel for with multiple clauses */
    #pragma omp parallel for private(i) shared(sum) reduction(+:sum) schedule(dynamic, 2) if(n > 1000)
    for (i = 0; i < n; i++) {
        sum += i * 2;
    }
    
    /* Test 2: Parallel region with data sharing clauses */
    #pragma omp parallel num_threads(4) default(none) shared(sum, n) private(i)
    {
        int local_sum = 0;
        #pragma omp for nowait
        for (i = 0; i < n; i++) {
            local_sum += i;
        }
        #pragma omp atomic
        sum += local_sum;
    }
    
    /* Test 3: Sections with different clauses */
    #pragma omp parallel sections private(i) reduction(+:sum)
    {
        #pragma omp section
        {
            for (i = 0; i < n/4; i++) {
                sum += i * 3;
            }
        }
        
        #pragma omp section
        {
            int j;
            for (j = n/4; j < n/2; j++) {
                sum += j * 2;
            }
        }
    }
    
    /* Test 4: Task construct with dependencies */
    int a = 0, b = 0, c = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: a) firstprivate(n)
        { a = n * 2; }
        
        #pragma omp task depend(out: b)
        { b = n * 3; }
        
        #pragma omp task depend(in: a, b) depend(out: c)
        { c = a + b; }
        
        #pragma omp task depend(in: c)
        { sum += c; }
        
        #pragma omp taskwait
    }
    
    /* Test 5: SIMD directive with clauses */
    int arr[1000];
    for (i = 0; i < 1000; i++) arr[i] = i;
    
    #pragma omp simd aligned(arr: 16) linear(i:1) reduction(+:sum) safelen(8)
    for (i = 0; i < 1000; i++) {
        sum += arr[i];
    }
    
    /* Test 6: Target directives for offloading */
    #pragma omp target map(tofrom: sum) if(target: n > 5000) device(0)
    {
        #pragma omp teams distribute parallel for reduction(+:sum) collapse(2)
        for (int x = 0; x < 10; x++) {
            for (int y = 0; y < 10; y++) {
                sum += x * y;
            }
        }
    }
    
    return sum;
}

/* Main test driver */
int main() {
    int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* Test IDENTIFIER_NODE */
    checksum += test_identifier_nodes();
    printf("IDENTIFIER_NODE test complete\n");
    
    /* Test TREE_VEC */
    checksum += test_tree_vec();
    printf("TREE_VEC test complete\n");
    
    /* Test SSA_NAME */
    checksum += test_ssa_name(50);
    printf("SSA_NAME test complete\n");
    
    /* Test BLOCK */
    checksum += test_block_nodes();
    printf("BLOCK test complete\n");
    
    /* Test CONSTRUCTOR */
    checksum += test_constructor_nodes();
    printf("CONSTRUCTOR test complete\n");
    
    /* Test OMP_CLAUSE */
    checksum += test_omp_clause(100);
    printf("OMP_CLAUSE test complete\n");
    
#ifdef __cplusplus
    /* Test BINFO (C++ only) */
    test_binfo();
    printf("BINFO test complete\n");
#endif
    
    /* Call external functions to create unresolved identifiers */
    checksum += external_func1();
    external_func2(checksum);
    checksum += (int)external_func3(checksum);
    
    /* Final sink */
    global_sink = checksum;
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed successfully!\n");
    
    return 0;
}
