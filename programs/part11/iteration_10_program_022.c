/* test_tree.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func(int);
extern volatile int external_var;

/* Sink function to prevent optimization */
static void sink(void *p) {
    volatile static int sink_var = 0;
    sink_var += (int)(long)p;
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

struct MultiDerived : Derived {
    int c;
};

void use_inheritance() {
    MultiDerived md;
    md.a = 1;      /* Accesses through inheritance hierarchy */
    md.b = 2;
    md.c = 3;
    Base *bp = &md;
    bp->vfunc();   /* Virtual call */
    sink(&md);
}
#endif

/* Function to generate SSA_NAME nodes with complex control flow */
int generate_ssa(int n) {
    int i, s = 0, t = 1, u = 2;
    
    /* Complex loop with multiple branches */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t ^= u;
        } else {
            s *= 2 + u;
            u += t;
        }
        
        switch (i % 4) {
            case 0: s += 1; break;
            case 1: s -= t; break;
            case 2: s *= u; break;
            case 3: s /= 2; break;
        }
    }
    
    /* Nested loops for more SSA complexity */
    for (i = 0; i < 10; i++) {
        for (int j = 0; j < i; j++) {
            s += (i * j) % 7;
        }
    }
    
    return s;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
void nested_scopes() {
    /* Level 1 */
    volatile int x = 1;
    sink((void*)(long)x);
    
    {
        /* Level 2 - shadow x */
        volatile int x = 2;
        {
            /* Level 3 - shadow again */
            volatile int x = 3;
            sink((void*)(long)x);
        }
        
        /* Reference outer x */
        extern int x;  /* Unresolved identifier */
        volatile int y = x;  /* Should reference external x */
        sink((void*)(long)y);
    }
    
    /* Function scope shadowing */
    for (int i = 0; i < 3; i++) {
        volatile int x = i * 10;  /* Different x in loop */
        sink((void*)(long)x);
    }
    
    /* Block with same variable name */
    {
        volatile long x = 99L;  /* Different type, same name */
        sink((void*)x);
    }
}

/* Function for TREE_VEC nodes using GCC vector extensions */
#ifdef __GNUC__
void vector_operations() {
    /* Vector types with different sizes */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v8f __attribute__((vector_size(32)));
    typedef short v16hi __attribute__((vector_size(32)));
    
    /* Vector initializations */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b - c;
    
    /* Compound literal in array initializer */
    int *arr = (int[]){1, 2, 3, 4, 5};
    sink(arr);
    
    /* More complex vector operations */
    v8f f1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8f f2 = f1 * 2.0f;
    v8f f3 = f1 + f2;
    
    /* Vector comparisons */
    v4si mask = a > b;
    v4si result = (a & mask) | (b & ~mask);
    
    sink(&c);
    sink(&d);
    sink(&f3);
    sink(&result);
}
#endif

/* Function with complex blocks and labels for BLOCK nodes */
void block_nodes() {
    int a = 0;
    
    /* First block with label */
    block1: {
        int b = 1;
        a += b;
        goto block3;  /* Jump forward */
    }
    
    /* Second block (unreachable directly) */
    block2: {
        int c = 2;
        a *= c;
        goto block4;
    }
    
    /* Third block */
    block3: {
        int d = 3;
        a -= d;
        goto block2;  /* Jump back */
    }
    
    /* Fourth block */
    block4: {
        int e = 4;
        a /= (e != 0 ? e : 1);
    }
    
    /* Nested blocks with variables */
    {
        int x = 5;
        {
            int y = 6;
            {
                int z = 7;
                a += x + y + z;
            }
            /* y still in scope */
            a += y * 2;
        }
        /* x still in scope */
        a += x * 3;
    }
    
    sink((void*)(long)a);
}

/* Function for CONSTRUCTOR nodes with various initializers */
void constructor_nodes() {
    /* Struct with designated initializers */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Point p1 = { .x = 1, .y = 2, .z = 3 };
    struct Point p2 = { .y = 5, .z = 6, .x = 4 };  /* Out of order */
    struct Point p3 = { .x = 7 };  /* Partial initialization */
    
    /* Array with designated initializers */
    int arr1[10] = { [0] = 1, [5] = 2, [9] = 3 };
    int arr2[] = { [0 ... 4] = 10, [5 ... 9] = 20 };
    
    /* Nested struct with array */
    struct Data {
        int id;
        struct Point loc;
        int values[4];
    };
    
    struct Data d1 = {
        .id = 100,
        .loc = { .x = 1, .y = 2 },
        .values = { [1] = 10, [3] = 20 }
    };
    
    /* Union initializer */
    union Mixed {
        int i;
        float f;
        char c[4];
    };
    
    union Mixed u1 = { .i = 42 };
    union Mixed u2 = { .f = 3.14f };
    union Mixed u3 = { .c = "ABC" };
    
    /* Zero initialization */
    struct Point p4 = {0};
    int arr3[5] = {0};
    
    sink(&p1);
    sink(&p2);
    sink(&p3);
    sink(arr1);
    sink(arr2);
    sink(&d1);
    sink(&u1);
    sink(&u2);
    sink(&u3);
    sink(&p4);
    sink(arr3);
}

/* OpenMP functions for OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_test(int n) {
    int i, sum = 0;
    int private_var = 0;
    static int shared_var = 0;
    
    /* Test 1: Parallel for with multiple clauses */
    #pragma omp parallel for private(i) shared(sum, shared_var) \
                reduction(+:sum) schedule(dynamic, 2) \
                num_threads(4) if(n > 100)
    for (i = 0; i < n; i++) {
        sum += i;
        shared_var++;
    }
    
    /* Test 2: Parallel region with sections */
    #pragma omp parallel default(none) shared(sum, n) \
                firstprivate(private_var)
    {
        #pragma omp sections nowait
        {
            #pragma omp section
            {
                private_var = 1;
                sum += private_var;
            }
            
            #pragma omp section
            {
                private_var = 2;
                sum += private_var * 2;
            }
        }
        
        #pragma omp barrier
        
        #pragma omp single copyprivate(private_var)
        {
            private_var = sum;
        }
    }
    
    /* Test 3: Task construct */
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (i = 0; i < 10; i++) {
                #pragma omp task depend(in: sum) depend(out: shared_var) \
                            priority(i) untied mergeable
                {
                    shared_var += i;
                }
            }
        }
    }
    
    sink((void*)(long)sum);
    sink((void*)(long)shared_var);
}
#endif

/* Main test driver */
int main() {
    int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* 1. Test IDENTIFIER_NODE generation */
    printf("Testing IDENTIFIER_NODE...\n");
    nested_scopes();
    checksum += 1;
    
    /* 2. Test SSA_NAME generation */
    printf("Testing SSA_NAME...\n");
    checksum += generate_ssa(50);
    
    /* 3. Test BLOCK nodes */
    printf("Testing BLOCK nodes...\n");
    block_nodes();
    checksum += 2;
    
    /* 4. Test CONSTRUCTOR nodes */
    printf("Testing CONSTRUCTOR nodes...\n");
    constructor_nodes();
    checksum += 3;
    
#ifdef __GNUC__
    /* 5. Test TREE_VEC nodes */
    printf("Testing TREE_VEC nodes...\n");
    vector_operations();
    checksum += 4;
#endif
    
#ifdef __cplusplus
    /* 6. Test TREE_BINFO nodes (C++ only) */
    printf("Testing TREE_BINFO nodes...\n");
    use_inheritance();
    checksum += 5;
#endif
    
#ifdef _OPENMP
    /* 7. Test OMP_CLAUSE nodes */
    printf("Testing OMP_CLAUSE nodes...\n");
    omp_test(200);
    checksum += 6;
#endif
    
    /* Use external identifiers */
    checksum += external_var;
    checksum = external_func(checksum);
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
