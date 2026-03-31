/* test_tree.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func(void);
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

void use_hierarchy(Derived *d, DeepDerived *dd) {
    d->a = 1;        // Accesses through inheritance
    dd->b = 2;       // Multi-level inheritance access
    Base *b = d;     // Upcast
    b->vfunc();      // Virtual call
}
#endif

/* Function with complex control flow for SSA_NAME */
int ssa_test(int n) {
    int i, s = 0, t = 1;
    
    /* Complex loop with multiple updates */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t++;
        } else {
            s *= 2;
            t--;
        }
        
        /* Nested condition */
        if (s > 1000) {
            s /= 2;
            t = t % 10;
        }
    }
    
    /* Another loop with phi nodes */
    int j = 0;
    while (j < n) {
        if (j % 3 == 0) {
            s += j;
        } else if (j % 3 == 1) {
            s -= j;
        } else {
            s *= (j + 1);
        }
        j++;
    }
    
    return s;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
void identifier_test(void) {
    volatile int result = 0;
    
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
                char x = 'A';
                result += x;
                
                /* Level 4 - extern declaration */
                {
                    extern int x;  /* Unresolved identifier */
                    volatile int y = external_var;  /* Use external */
                    result += y;
                }
            }
        }
        
        /* Another sibling scope */
        {
            /* Yet another x */
            double x = 3.14;
            result += (int)x;
        }
    }
    
    /* Function scope variables with same name */
    {
        long x = 100;
        result += x;
    }
    
    sink(&result);
}

/* Function for TREE_VEC nodes */
void tree_vec_test(void) {
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = f1 * 2.0f;
    
    v8hi h1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi h2 = h1 << 1;
    
    /* Array compound literals */
    int *p1 = (int[]){1, 2, 3, 4, 5};
    int *p2 = (int[]){[0]=10, [2]=20, [4]=30};
    
    /* Nested initializers with vectors */
    struct VecHolder {
        v4si vec;
        int scalar;
    };
    
    struct VecHolder vh = {
        .vec = {10, 20, 30, 40},
        .scalar = 99
    };
    
    sink(&c);
    sink(&d);
    sink(&f2);
    sink(&h2);
    sink(p1);
    sink(p2);
    sink(&vh);
}

/* Function with complex blocks for BLOCK nodes */
void block_test(void) {
    int a = 0;
    
    /* Block 1 with label */
    {
        int b = 1;
    block1_label:
        a += b;
        
        /* Nested block */
        {
            int c = 2;
            a += c;
            goto block3_label;  /* Jump forward */
        }
    }
    
    /* Block 2 (skipped by goto) */
    {
        int d = 3;
    block2_label:
        a += d;
    }
    
    /* Block 3 */
    {
        int e = 4;
    block3_label:
        a += e;
        
        /* Jump back */
        if (a < 100) {
            goto block1_label;
        }
    }
    
    /* Switch with blocks */
    switch (a % 3) {
        case 0: {
            int x = 10;
            a += x;
            break;
        }
        case 1: {
            int x = 20;  /* Same name, different scope */
            a += x;
            break;
        }
        case 2: {
            int x = 30;
            a += x;
            break;
        }
    }
    
    sink(&a);
}

/* Function for CONSTRUCTOR nodes */
void constructor_test(void) {
    /* Struct with designated initializers */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Line {
        struct Point start;
        struct Point end;
        int style;
    };
    
    /* Complex nested initializers */
    struct Line l1 = {
        .start = {.x = 1, .y = 2, .z = 3},
        .end = {.x = 4, .y = 5},
        .style = 1
    };
    
    /* Partial array initialization */
    struct ArrayStruct {
        int data[10];
        int count;
    };
    
    struct ArrayStruct as1 = {
        .data = {[0] = 100, [5] = 200, [9] = 300},
        .count = 3
    };
    
    /* Union with initializer */
    union Mixed {
        int i;
        float f;
        char c[4];
    };
    
    union Mixed u1 = {.i = 0x12345678};
    union Mixed u2 = {.f = 3.14f};
    union Mixed u3 = {.c = {'A', 'B', 'C', 'D'}};
    
    /* Nested anonymous struct */
    struct Outer {
        struct {
            int a;
            int b;
        };
        int c;
    };
    
    struct Outer o1 = {
        .a = 1,
        .b = 2,
        .c = 3
    };
    
    sink(&l1);
    sink(&as1);
    sink(&u1);
    sink(&u2);
    sink(&u3);
    sink(&o1);
}

/* OpenMP tests for OMP_CLAUSE nodes */
void omp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    int shared_var = 0;
    int *dynamic_array = (int*)malloc(n * sizeof(int));
    
    if (!dynamic_array) return;
    
    /* Initialize array */
    for (i = 0; i < n; i++) {
        dynamic_array[i] = i;
    }
    
    /* Test 1: Parallel for with multiple clauses */
    #pragma omp parallel for private(i) shared(sum, dynamic_array) \
        reduction(+:sum) schedule(dynamic, 2) num_threads(4) \
        if(n > 100)
    for (i = 0; i < n; i++) {
        sum += dynamic_array[i];
    }
    
    /* Test 2: Parallel sections */
    #pragma omp parallel sections private(private_var) \
        shared(shared_var) firstprivate(sum)
    {
        #pragma omp section
        {
            private_var = 1;
            shared_var += private_var;
        }
        
        #pragma omp section
        {
            private_var = 2;
            shared_var += private_var;
        }
    }
    
    /* Test 3: Task with dependencies */
    int task_result = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: task_result) priority(high)
        {
            task_result = sum;
        }
        
        #pragma omp task depend(in: task_result) priority(low)
        {
            shared_var += task_result;
        }
    }
    
    /* Test 4: Critical with hints */
    #pragma omp critical(my_critical) hint(omp_sync_hint_contended)
    {
        shared_var++;
    }
    
    /* Test 5: Atomic with clauses */
    #pragma omp atomic update seq_cst
    shared_var += 10;
    
    sink(&sum);
    sink(&shared_var);
    sink(&task_result);
    free(dynamic_array);
}

/* Main test driver */
int main(void) {
    volatile int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* Test IDENTIFIER_NODE */
    printf("Testing IDENTIFIER_NODE...\n");
    identifier_test();
    checksum += 1;
    
    /* Test TREE_VEC */
    printf("Testing TREE_VEC...\n");
    tree_vec_test();
    checksum += 2;
    
    /* Test SSA_NAME */
    printf("Testing SSA_NAME...\n");
    int ssa_result = ssa_test(50);
    checksum += ssa_result % 1000;
    
    /* Test BLOCK nodes */
    printf("Testing BLOCK nodes...\n");
    block_test();
    checksum += 4;
    
    /* Test CONSTRUCTOR nodes */
    printf("Testing CONSTRUCTOR nodes...\n");
    constructor_test();
    checksum += 8;
    
    /* Test OMP_CLAUSE nodes */
    printf("Testing OMP_CLAUSE nodes...\n");
    omp_test(1000);
    checksum += 16;
    
#ifdef __cplusplus
    /* Test TREE_BINFO nodes (C++ only) */
    printf("Testing TREE_BINFO nodes...\n");
    Derived d;
    DeepDerived dd;
    use_hierarchy(&d, &dd);
    checksum += 32;
#endif
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum == 0 ? 0 : 1;
}
