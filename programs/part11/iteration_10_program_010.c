/* test_tree.c - Comprehensive tree node coverage test */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func(int);
extern volatile int global_var;

/* Helper to prevent optimization */
static volatile int sink;

#ifdef __cplusplus
/* C++ specific code for BINFO nodes */
struct Base {
    int a;
    virtual void virt() {}
};

struct Derived : Base {
    int b;
    void virt() override {}
};

struct MultiDerived : Derived {
    int c;
};

void use_inheritance() {
    MultiDerived md;
    md.a = 1;  /* Accesses through inheritance hierarchy */
    md.b = 2;
    md.c = 3;
    Base* bp = &md;
    bp->virt();
}
#endif

/* Function to force SSA_NAME creation with complex control flow */
int ssa_test(int n) {
    int i, s = 0, t = 1, u = 0;
    
    /* Complex loop with multiple branches */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t ^= s;
        } else {
            s *= 2;
            u += s;
        }
        
        switch (i % 4) {
            case 0: s += u; break;
            case 1: s -= t; break;
            case 2: s ^= u; break;
            case 3: s |= t; break;
        }
    }
    
    /* Nested loops for more SSA complexity */
    for (int j = 0; j < 5; j++) {
        for (int k = 0; k < j; k++) {
            s += j * k;
            if (k % 2) s--;
            else s++;
        }
    }
    
    return s;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
void identifier_test() {
    /* Level 1 - outer scope 'x' */
    int x = 1;
    sink = x;
    
    {
        /* Level 2 - shadowing 'x' */
        volatile int x = 2;
        sink = x;
        
        {
            /* Level 3 - another 'x' */
            extern int x;  /* Declaration only */
            volatile int y = x;  /* Uses external x */
            sink = y;
            
            {
                /* Level 4 - yet another 'x' */
                static int x = 3;
                sink = x;
                
                {
                    /* Level 5 - pointer to x */
                    int* x = &((int){4});
                    sink = *x;
                }
            }
        }
    }
    
    /* More shadowing in loops */
    for (int i = 0; i < 3; i++) {
        int x = i * 10;  /* New 'x' in loop scope */
        sink = x;
        
        for (int j = 0; j < 2; j++) {
            float x = j * 1.5f;  /* Different type 'x' */
            sink = (int)x;
        }
    }
}

/* Function for TREE_VEC nodes using vector extensions */
void vector_test() {
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c - d;
    
    /* Store to volatile to prevent optimization */
    volatile v4si* vsink = &e;
    sink = (*vsink)[0];
    
    /* Float vectors */
    v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fb = fa * 2.0f;
    volatile v4sf* fvsink = &fb;
    sink = (int)(*fvsink)[0];
    
    /* Array compound literals */
    int* p1 = (int[]){1, 2, 3, 4};
    int* p2 = (int[]){[0]=5, [2]=6, [3]=7};
    sink = p1[0] + p2[0];
    
    /* Nested array literals */
    int* p3 = (int[]){1, (int[]){2, 3}[0], 4};
    sink = p3[1];
}

/* Function with complex blocks for BLOCK nodes */
void block_test() {
    int a = 0;
    
    /* Block 1 with label */
    block1: {
        int b = 1;
        a += b;
        
        /* Nested block */
        {
            int c = 2;
            a += c;
            goto block3;  /* Jump forward */
        }
        
        /* Unreachable but creates BLOCK structure */
        int d = 3;
        a += d;
    }
    
    /* Block 2 (skipped by goto) */
    block2: {
        int e = 4;
        a += e;
        goto block4;
    }
    
    /* Block 3 */
    block3: {
        int f = 5;
        a += f;
        goto block2;  /* Jump back */
    }
    
    /* Block 4 */
    block4: {
        int g = 6;
        a += g;
    }
    
    /* Switch with blocks */
    switch (a % 3) {
        case 0: {
            int h = 7;
            a += h;
            break;
        }
        case 1: {
            int i = 8;
            a += i;
            break;
        }
        default: {
            int j = 9;
            a += j;
            break;
        }
    }
    
    sink = a;
}

/* Function for CONSTRUCTOR nodes with complex initializers */
void constructor_test() {
    /* Struct with designated initializers */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Rect {
        struct Point p1;
        struct Point p2;
        int id;
    };
    
    /* Complex designated initializer */
    struct Rect r1 = {
        .p1 = {.x = 1, .y = 2, .z = 3},
        .p2 = {.x = 4, .y = 5},
        .id = 100
    };
    sink = r1.p1.x + r1.p2.y;
    
    /* Partial array initialization */
    int arr1[10] = {[0] = 1, [5] = 2, [9] = 3};
    sink = arr1[5];
    
    /* Nested designated initializers */
    struct Nested {
        int a;
        struct {
            int b;
            int c[3];
        } inner;
        int d;
    };
    
    struct Nested n = {
        .a = 1,
        .inner = {
            .b = 2,
            .c = {[1] = 3, [2] = 4}
        },
        .d = 5
    };
    sink = n.inner.c[1];
    
    /* Union initializers */
    union Data {
        int i;
        float f;
        char str[4];
    };
    
    union Data u1 = {.i = 42};
    union Data u2 = {.f = 3.14f};
    union Data u3 = {.str = "ABC"};
    sink = u1.i + (int)u2.f + u3.str[0];
    
    /* Array of structs with mixed initialization */
    struct Point points[3] = {
        {1, 2, 3},
        {.x = 4, .z = 5},
        [2] = {6, 7, 8}
    };
    sink = points[1].x + points[2].z;
}

/* OpenMP function for OMP_CLAUSE nodes */
void omp_test(int n) {
    int i, sum = 0;
    int private_var = 0;
    static int shared_var = 0;
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(i) shared(shared_var) \
            reduction(+:sum) schedule(dynamic, 2) \
            num_threads(4) if(n > 1000)
    for (i = 0; i < n; i++) {
        sum += i;
        #pragma omp atomic
        shared_var++;
    }
    
    sink = sum + shared_var;
    
    /* Another OpenMP construct with different clauses */
    int arr[100];
    #pragma omp parallel sections private(private_var) \
            firstprivate(sum) lastprivate(private_var)
    {
        #pragma omp section
        {
            private_var = 1;
            for (int j = 0; j < 50; j++) {
                arr[j] = j * private_var;
            }
        }
        
        #pragma omp section
        {
            private_var = 2;
            for (int j = 50; j < 100; j++) {
                arr[j] = j * private_var;
            }
        }
    }
    
    sink = arr[25] + arr[75];
    
    /* OMP critical with clause */
    #pragma omp parallel
    {
        #pragma omp critical (my_critical)
        {
            shared_var += 10;
        }
        
        /* OMP barrier */
        #pragma omp barrier
        
        #pragma omp single copyprivate(private_var) nowait
        {
            private_var = 42;
        }
    }
    
    sink = shared_var + private_var;
}

/* Main test driver */
int main() {
    int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* Test 1: IDENTIFIER_NODE */
    printf("Testing IDENTIFIER_NODE...\n");
    identifier_test();
    checksum += sink;
    
    /* Test 2: TREE_VEC */
    printf("Testing TREE_VEC...\n");
    vector_test();
    checksum += sink;
    
    /* Test 3: SSA_NAME */
    printf("Testing SSA_NAME...\n");
    int ssa_result = ssa_test(100);
    checksum += ssa_result;
    sink = ssa_result;
    
    /* Test 4: BLOCK nodes */
    printf("Testing BLOCK nodes...\n");
    block_test();
    checksum += sink;
    
    /* Test 5: CONSTRUCTOR nodes */
    printf("Testing CONSTRUCTOR nodes...\n");
    constructor_test();
    checksum += sink;
    
    /* Test 6: OMP_CLAUSE nodes */
    printf("Testing OMP_CLAUSE nodes...\n");
    omp_test(500);
    checksum += sink;
    
#ifdef __cplusplus
    /* Test 7: TREE_BINFO (C++ only) */
    printf("Testing TREE_BINFO (C++ only)...\n");
    use_inheritance();
    checksum += 1;  /* Dummy checksum increment */
#endif
    
    /* Use external symbols to create more IDENTIFIER_NODEs */
    checksum += external_func(checksum);
    checksum += global_var;
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum == 0 ? 0 : 1;
}
