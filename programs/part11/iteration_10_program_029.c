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
/* C++ specific code for TREE_BINFO */
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

void use_inheritance() {
    DeepDerived dd;
    dd.a = 1;      /* Accesses through inheritance chain */
    dd.b = 2;
    dd.c = 3;
    Base* bp = &dd;
    bp->vfunc();   /* Virtual call */
}
#endif

/* Function to create SSA_NAME nodes with complex control flow */
int create_ssa_names(int n) {
    int i, j, k, sum = 0;
    
    /* Complex loop with multiple branches */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            sum += i * 2;
            for (j = 0; j < i; j++) {
                if (j % 3 == 0) {
                    sum -= j;
                } else if (j % 3 == 1) {
                    sum *= 2;
                } else {
                    sum = sum / 2 + 1;
                }
            }
        } else {
            sum = sum * 3 - 1;
            k = i;
            while (k > 0) {
                sum += k;
                k /= 2;
            }
        }
        
        /* Switch to add more complexity */
        switch (i % 4) {
            case 0: sum += 100; break;
            case 1: sum -= 50; break;
            case 2: sum *= 2; break;
            case 3: sum = sum / 3; break;
        }
    }
    
    return sum;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
void test_identifier_nodes(void) {
    int x = 1;
    volatile int sink = x;
    
    /* Level 1 */
    {
        int x = 2;  /* Same name, different scope */
        sink += x;
        
        /* Level 2 */
        {
            extern int x;  /* External declaration */
            sink += 3;
            
            /* Level 3 */
            {
                volatile int x = 4;
                sink += x;
                
                /* Level 4 - in a loop */
                for (int x = 0; x < 3; x++) {
                    sink += x;
                    
                    /* Level 5 - in a conditional */
                    if (x == 1) {
                        static int x = 5;
                        sink += x;
                    }
                }
            }
        }
    }
    
    /* Function parameter shadowing */
    auto shadow_func = [](int x) -> int {
        {
            long x = 6L;
            return x + 7;
        }
    };
    
    sink += shadow_func(0);
    global_sink += sink;
}

/* Function for TREE_VEC nodes using vector extensions */
void test_tree_vec(void) {
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c - d;
    
    v4sf f = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf g = f * 2.0f;
    
    v8hi h = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi i = h << 1;
    
    /* Array compound literals */
    int* arr1 = (int[]){1, 2, 3, 4, 5};
    int* arr2 = (int[3]){10, 20, 30};
    struct Point { int x; int y; };
    struct Point* points = (struct Point[]){{1,2}, {3,4}, {5,6}};
    
    /* Multi-dimensional array literal */
    int (*md_arr)[3] = (int[2][3]){{1,2,3}, {4,5,6}};
    
    /* Use vectors to prevent dead code elimination */
    volatile v4si sink_vec = e;
    global_sink += sink_vec[0] + sink_vec[1];
}

/* Function with complex BLOCK structure */
void test_block_nodes(void) {
    int a = 0;
    
    /* Block 1 */
    {
        int b = 1;
    block1_label:
        b += a;
        
        /* Block 2 */
        {
            int c = 2;
            goto block3_label;  /* Jump across blocks */
        }
    }
    
    /* Block 3 */
    {
        int d = 3;
    block3_label:
        d += 10;
        goto block4_label;
    }
    
    /* Block 4 */
    {
        int e = 4;
    block4_label:
        e++;
        
        /* Nested block with label */
        {
            int f = 5;
        inner_label:
            f *= 2;
            if (f < 20) goto inner_label;
        }
        
        goto block1_label;  /* Backward jump */
    }
    
    /* Switch with compound statements */
    switch (a) {
        case 0: {
            int x = 100;
            a += x;
            break;
        }
        case 1: {
            int y = 200;
            a += y;
            break;
        }
        default: {
            int z = 300;
            a += z;
        }
    }
    
    global_sink += a;
}

/* Function for CONSTRUCTOR nodes */
void test_constructor_nodes(void) {
    /* Struct with designated initializers */
    struct Complex {
        int a;
        int b[4];
        struct {
            float x;
            float y;
        } point;
        union {
            int i;
            float f;
        } data;
    };
    
    /* Full initialization */
    struct Complex c1 = {
        .a = 1,
        .b = {10, 20, 30, 40},
        .point = {3.14f, 2.71f},
        .data = {.f = 1.618f}
    };
    
    /* Partial initialization */
    struct Complex c2 = {
        .a = 2,
        .b = {[1] = 50, [3] = 60},
        .point.x = 1.0f
        /* .point.y and .data are zero-initialized */
    };
    
    /* Nested designated initializers */
    struct Outer {
        struct Inner {
            int x;
            int y;
        } inner;
        int arr[2][3];
    };
    
    struct Outer o = {
        .inner = {.x = 1, .y = 2},
        .arr = {{[1] = 10}, {[0] = 20, [2] = 30}}
    };
    
    /* Union initialization */
    union U {
        int i;
        float f;
        double d;
    };
    
    union U u1 = {.i = 42};
    union U u2 = {.f = 3.14f};
    union U u3 = {.d = 2.71828};
    
    /* Array with designated range (GCC extension) */
    int arr[10] = {[0 ... 4] = 1, [5 ... 9] = 2};
    
    /* Use variables to prevent optimization */
    global_sink += c1.a + c2.a + o.inner.x + u1.i + arr[3];
}

/* Function with OpenMP pragmas for OMP_CLAUSE nodes */
void test_omp_clauses(int n) {
    int i, sum = 0;
    int private_var = 0;
    static int shared_var = 0;
    int reduction_sum = 0;
    int last_iter = 0;
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel private(private_var) shared(shared_var) \
                         reduction(+:reduction_sum) if(n > 1000)
    {
        private_var = omp_get_thread_num();
        shared_var++;
        reduction_sum += private_var;
        
        /* Barrier with memory clause */
        #pragma omp barrier
        
        /* Single construct with copyprivate */
        #pragma omp single copyprivate(last_iter)
        {
            last_iter = private_var;
        }
    }
    
    /* Parallel for with schedule clause */
    #pragma omp parallel for schedule(dynamic, 4) \
                         ordered collapse(2) \
                         linear(i:2) lastprivate(i)
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            #pragma omp ordered
            {
                sum += x * y;
            }
        }
    }
    
    /* Sections with nowait */
    #pragma omp parallel sections num_threads(2)
    {
        #pragma omp section
        {
            sum += 1;
        }
        
        #pragma omp section
        {
            sum += 2;
        }
    }
    
    /* Task with depend clause */
    int a = 0, b = 0;
    #pragma omp task depend(out: a)
    {
        a = 1;
    }
    
    #pragma omp task depend(in: a) depend(out: b)
    {
        b = a + 1;
    }
    
    #pragma omp task depend(in: b)
    {
        sum += b;
    }
    
    /* Taskwait */
    #pragma omp taskwait
    
    /* Atomic with memory order */
    #pragma omp atomic update seq_cst
    shared_var += sum;
    
    /* Critical with hint */
    #pragma omp critical(my_critical) hint(omp_sync_hint_contended)
    {
        sum *= 2;
    }
    
    global_sink += sum + shared_var + reduction_sum + last_iter;
}

/* Main test driver */
int main(void) {
    int checksum = 0;
    
    printf("Testing GCC tree node coverage...\n");
    
    /* Test 1: IDENTIFIER_NODE */
    printf("1. Testing IDENTIFIER_NODE...\n");
    test_identifier_nodes();
    checksum += global_sink;
    
    /* Test 2: TREE_VEC */
    printf("2. Testing TREE_VEC...\n");
    test_tree_vec();
    checksum += global_sink;
    
    /* Test 3: SSA_NAME */
    printf("3. Testing SSA_NAME...\n");
    checksum += create_ssa_names(50);
    
    /* Test 4: BLOCK */
    printf("4. Testing BLOCK...\n");
    test_block_nodes();
    checksum += global_sink;
    
    /* Test 5: CONSTRUCTOR */
    printf("5. Testing CONSTRUCTOR...\n");
    test_constructor_nodes();
    checksum += global_sink;
    
    /* Test 6: OMP_CLAUSE */
    printf("6. Testing OMP_CLAUSE...\n");
    #ifdef _OPENMP
    test_omp_clauses(2000);
    checksum += global_sink;
    #else
    printf("   OpenMP not enabled, skipping...\n");
    #endif
    
    /* Test 7: TREE_BINFO (C++ only) */
    #ifdef __cplusplus
    printf("7. Testing TREE_BINFO...\n");
    use_inheritance();
    checksum += 1;
    #else
    printf("7. TREE_BINFO requires C++, skipping...\n");
    #endif
    
    printf("Final checksum: %d\n", checksum);
    
    /* Call external functions to create unresolved identifiers */
    external_func1();
    external_func2(checksum);
    
    return checksum & 0xFF;
}

/* Additional functions to create more tree nodes */
void extra_identifiers(void) {
    /* Multiple extern declarations */
    extern int ext_var1;
    extern float ext_var2;
    extern double ext_var3;
    extern void ext_func4(int);
    extern char* ext_func5(void);
    
    /* Use in different contexts */
    volatile int v1 = ext_var1;
    volatile float v2 = ext_var2;
    volatile double v3 = ext_var3;
    
    ext_func4(v1);
    char* str = ext_func5();
    (void)str;
}

/* Complex function with try/catch for C++ */
#ifdef __cplusplus
void test_exceptions(void) {
    try {
        throw 42;
    } catch (int e) {
        global_sink += e;
    } catch (...) {
        global_sink += 1;
    }
}
#endif
