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
    DeepDerived dd;
    dd.a = 1;      /* Accesses through inheritance hierarchy */
    dd.b = 2;
    dd.c = 3;
    Base* bp = &dd;
    bp->vfunc();   /* Virtual call */
}
#endif

/* Function to create SSA_NAME nodes with complex control flow */
int create_ssa_names(int n) {
    int i, s = 0, t = 1, u = 2;
    
    /* Complex loop with multiple branches */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t = s - u;
        } else {
            s *= 2 + u;
            u = t ^ s;
        }
        
        switch (i % 4) {
            case 0: s += 1; break;
            case 1: t += s; break;
            case 2: u += t; break;
            case 3: s = u * t; break;
        }
    }
    
    /* Nested loops for more SSA complexity */
    for (i = 0; i < 10; i++) {
        for (int j = 0; j < i; j++) {
            s += j;
            if (j % 2 == 0) {
                t += s;
            } else {
                u += t;
            }
        }
    }
    
    return s + t + u;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE creation */
int nested_scopes_test() {
    volatile int result = 0;
    
    /* Level 1 */
    {
        int x = 1;
        result += x;
        
        /* Level 2 */
        {
            /* Shadowing variable */
            int x = 2;
            result += x;
            
            /* Level 3 */
            {
                /* Another shadow */
                extern int x;  /* External declaration */
                volatile int y = x;  /* Use external */
                result += y;
                
                /* Level 4 - in loop */
                for (int i = 0; i < 3; i++) {
                    int x = i + 10;  /* Another shadow in loop */
                    result += x;
                    
                    /* Level 5 - in nested block */
                    {
                        volatile int x = i * 2;  /* Yet another */
                        result += x;
                    }
                }
            }
        }
        
        /* Back to outer scope */
        {
            int x = 99;  /* New scope, new variable */
            result += x;
        }
    }
    
    /* Function scope variables */
    {
        static int x = 100;  /* Static storage */
        volatile int y = x;
        result += y;
    }
    
    return result;
}

/* Function to create TREE_VEC nodes using vector extensions */
#ifdef __VECTOR_EXTENSION__
void vector_operations() {
    /* Vector types using GCC extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c - d;
    
    /* Vector comparisons */
    v4si mask = a > b;
    
    /* Float vectors */
    v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fb = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fc = fa * fb;
    
    /* Array compound literals - also creates TREE_VEC */
    int *p = (int[]){1, 2, 3, 4, 5};
    int *q = (int[]){[0] = 10, [2] = 20, [4] = 30};
    
    /* Multi-dimensional array literal */
    int (*arr)[3] = (int[][3]){{1, 2, 3}, {4, 5, 6}};
    
    /* Prevent dead code elimination */
    volatile v4si sink1 = c;
    volatile v4si sink2 = e;
    volatile v4sf sink3 = fc;
    (void)sink1; (void)sink2; (void)sink3;
    (void)p; (void)q; (void)arr;
}
#endif

/* Function with complex blocks and labels for BLOCK nodes */
int block_and_labels_test() {
    volatile int a = 0;
    
    /* Outer block with label */
    outer_block: {
        int x = 1;
        a += x;
        
        /* Inner block 1 */
        {
            int y = 2;
            a += y;
            goto middle_block;  /* Jump forward */
        }
        
        /* Unreachable code - creates separate block */
        {
            int z = 3;
            a += z;
        }
    }
    
    middle_block: {
        int w = 4;
        a += w;
        
        /* Nested block with its own label */
        inner_label: {
            int v = 5;
            a += v;
            
            /* Conditional goto */
            if (a < 100) {
                goto outer_block;  /* Jump back */
            } else {
                goto final_block;
            }
        }
    }
    
    final_block: {
        int u = 6;
        a += u;
        
        /* Block in loop */
        for (int i = 0; i < 3; i++) {
            loop_block: {
                int loop_var = i * 10;
                a += loop_var;
                
                if (i == 1) {
                    goto inner_label;  /* Jump to different scope */
                }
            }
        }
    }
    
    return a;
}

/* Function to create CONSTRUCTOR nodes with various initializers */
void constructor_tests() {
    /* Struct with designated initializers */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Point p1 = { .x = 1, .y = 2, .z = 3 };
    struct Point p2 = { .y = 20, .z = 30, .x = 10 };  /* Out of order */
    struct Point p3 = { .x = 100 };  /* Partial initialization */
    
    /* Nested struct with designated initializers */
    struct Line {
        struct Point start;
        struct Point end;
    };
    
    struct Line l1 = {
        .start = { .x = 1, .y = 2 },
        .end = { .z = 3 }
    };
    
    /* Array with designated initializers */
    int arr1[10] = { [0] = 1, [5] = 2, [9] = 3 };
    int arr2[] = { [0 ... 4] = 10, [5 ... 9] = 20 };
    
    /* Nested array in struct */
    struct Matrix {
        int values[3][3];
    };
    
    struct Matrix m1 = {
        .values = {
            { [0] = 1, [2] = 3 },
            { [1] = 5 },
            { 7, 8, 9 }
        }
    };
    
    /* Union initializers */
    union Data {
        int i;
        float f;
        char str[4];
    };
    
    union Data d1 = { .i = 42 };
    union Data d2 = { .f = 3.14f };
    union Data d3 = { .str = "ABC" };
    
    /* Complex nested initializer */
    struct Complex {
        struct {
            int a;
            int b;
        } inner;
        int arr[2][2];
    };
    
    struct Complex c1 = {
        .inner = { .a = 1, .b = 2 },
        .arr = { { [1] = 3 }, { [0] = 4 } }
    };
    
    /* Prevent dead code elimination */
    volatile struct Point vs1 = p1;
    volatile struct Point vs2 = p2;
    volatile struct Line vl1 = l1;
    (void)vs1; (void)vs2; (void)vl1;
    (void)arr1; (void)arr2; (void)m1;
    (void)d1; (void)d2; (void)d3; (void)c1;
}

/* OpenMP tests for OMP_CLAUSE nodes */
#ifdef _OPENMP
void openmp_tests(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    int shared_var = 0;
    int firstprivate_var = 10;
    int lastprivate_var = 0;
    int reduction_var = 0;
    
    /* Test 1: Parallel region with multiple clauses */
    #pragma omp parallel private(private_var) shared(shared_var) \
                         firstprivate(firstprivate_var) num_threads(2) \
                         default(none)
    {
        private_var = omp_get_thread_num();
        #pragma omp atomic
        shared_var++;
    }
    
    /* Test 2: Parallel for with reduction and schedule */
    #pragma omp parallel for private(i) shared(n) reduction(+:reduction_var) \
                         schedule(dynamic, 2) collapse(1) ordered
    for (i = 0; i < n; i++) {
        reduction_var += i;
        #pragma omp ordered
        {
            /* Ordered block */
        }
    }
    
    /* Test 3: Sections with nowait */
    #pragma omp parallel sections private(i) lastprivate(lastprivate_var) \
                                   nowait
    {
        #pragma omp section
        {
            for (i = 0; i < 5; i++) {
                lastprivate_var = i;
            }
        }
        
        #pragma omp section
        {
            for (i = 5; i < 10; i++) {
                lastprivate_var = i;
            }
        }
    }
    
    /* Test 4: Single with copyprivate */
    #pragma omp parallel private(private_var)
    {
        #pragma omp single copyprivate(private_var)
        {
            private_var = 42;
        }
        /* private_var should be 42 in all threads */
    }
    
    /* Test 5: Task with if clause */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task if(n > 100) untied mergeable final(n > 1000) \
                             priority(1)
            {
                sum += 1;
            }
        }
    }
    
    /* Test 6: Target directives for offloading */
    #pragma omp target map(tofrom: sum) device(0) if(n > 50)
    {
        sum += 10;
    }
    
    /* Test 7: Teams and distribute */
    #pragma omp target teams distribute parallel for simd \
                dist_schedule(static) num_teams(2) thread_limit(4) \
                safelen(8) simdlen(4) aligned(sum: 16)
    for (i = 0; i < n; i++) {
        sum += i;
    }
    
    /* Prevent dead code elimination */
    volatile int sink = sum + shared_var + reduction_var + lastprivate_var;
    (void)sink;
}
#endif

/* Main test driver */
int main() {
    volatile int checksum = 0;
    
    printf("Starting tree node coverage tests...\n");
    
    /* 1. Test IDENTIFIER_NODE creation */
    printf("Testing IDENTIFIER_NODE...\n");
    checksum += nested_scopes_test();
    
    /* 2. Test SSA_NAME creation */
    printf("Testing SSA_NAME...\n");
    checksum += create_ssa_names(20);
    
    /* 3. Test BLOCK nodes */
    printf("Testing BLOCK nodes...\n");
    checksum += block_and_labels_test();
    
    /* 4. Test TREE_VEC nodes */
    #ifdef __VECTOR_EXTENSION__
    printf("Testing TREE_VEC...\n");
    vector_operations();
    checksum += 1;
    #endif
    
    /* 5. Test CONSTRUCTOR nodes */
    printf("Testing CONSTRUCTOR...\n");
    constructor_tests();
    checksum += 2;
    
    /* 6. Test C++ BINFO nodes */
    #ifdef __cplusplus
    printf("Testing TREE_BINFO (C++ inheritance)...\n");
    use_inheritance();
    checksum += 3;
    #endif
    
    /* 7. Test OMP_CLAUSE nodes */
    #ifdef _OPENMP
    printf("Testing OMP_CLAUSE...\n");
    openmp_tests(100);
    checksum += 4;
    #endif
    
    /* Use external identifiers */
    checksum += external_var;
    checksum += external_func(checksum);
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
