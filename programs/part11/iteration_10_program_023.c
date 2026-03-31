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
    virtual void virt() {}
};

struct Derived : Base {
    int b;
    void virt() override {}
};

struct DeepDerived : Derived {
    int c;
    void virt() override {}
};

void use_hierarchy() {
    DeepDerived dd;
    dd.a = 1;      /* Accesses through inheritance chain */
    dd.b = 2;
    dd.c = 3;
    Base* bp = &dd;
    bp->virt();    /* Virtual call */
    global_sink = dd.a + dd.b + dd.c;
}
#endif

/* Function to create many IDENTIFIER_NODE instances */
void test_identifiers() {
    /* Deeply nested scopes with same variable names */
    {
        int x = 1;
        volatile int sink1 = x;
        
        {
            /* Different x in inner scope */
            float x = 2.0f;
            volatile float sink2 = x;
            
            {
                /* Another x */
                double x = 3.0;
                volatile double sink3 = x;
                
                {
                    /* Pointer x */
                    int* x = &global_sink;
                    volatile int* sink4 = x;
                    
                    {
                        /* Array x */
                        int x[4] = {1, 2, 3, 4};
                        volatile int sink5 = x[0];
                        
                        /* Reference to outer x via extern declaration */
                        {
                            extern int global_sink;
                            volatile int sink6 = global_sink;
                        }
                    }
                }
            }
        }
    }
    
    /* More identifier variations */
    {
        long counter = 0;
        {
            long counter = 1;  /* Shadows outer counter */
            {
                long counter = 2;
                global_sink += (int)counter;
            }
            global_sink += (int)counter;
        }
    }
}

/* Function to create TREE_VEC nodes */
void test_tree_vec() {
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    /* Store to volatile to prevent optimization */
    volatile v4si sink_vec = c + d;
    
    /* Array compound literals */
    int* p1 = (int[]){1, 2, 3, 4, 5};
    int* p2 = (int[]){6, 7, 8, 9, 10};
    
    /* Nested compound literals */
    struct Point { int x; int y; };
    struct Point* points = (struct Point[]){{1, 2}, {3, 4}, {5, 6}};
    
    global_sink += p1[0] + p2[0] + points[0].x;
    
    /* More vector operations */
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf f3 = f1 * f2;
    
    /* Prevent dead code elimination */
    volatile v4sf sink_float_vec = f3;
}

/* Function to create SSA_NAME nodes (requires optimization) */
int test_ssa_names(int n) {
    int i, s = 0, t = 1, u = 2;
    
    /* Complex control flow with many assignments */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t ^= u;
        } else {
            s *= 2 + u;
            u += t;
        }
        
        /* Nested condition */
        if (s > 100) {
            t = s % 10;
            if (u < 50) {
                u += s;
            } else {
                u -= t;
            }
        } else {
            s += u;
        }
    }
    
    /* Another loop with induction variable */
    int j = n;
    while (j > 0) {
        int k = j;
        while (k > 0) {
            s += k;
            k /= 2;
        }
        j--;
    }
    
    return s;
}

/* Function to create BLOCK nodes */
void test_blocks() {
    int a = 0;
    
    /* Deeply nested blocks with labels */
    {
        int b = 1;
    label1:
        b++;
        {
            int c = 2;
        label2:
            c += a;
            {
                int d = 3;
                if (d > 0) {
                    goto label3;
                }
            label3:
                d = 4;
                goto label4;
            }
        }
    label4:
        a += 2;
        if (a < 10) {
            goto label1;
        }
    }
    
    /* Switch with blocks */
    switch (global_sink) {
        case 0: {
            int x = 5;
            {
                int y = x * 2;
                global_sink += y;
            }
            break;
        }
        case 1: {
            int x = 10;
            goto inner_label;
        inner_label:
            global_sink += x;
            break;
        }
        default: {
            int x = 15;
            global_sink += x;
        }
    }
}

/* Function to create CONSTRUCTOR nodes */
void test_constructors() {
    /* Struct with designated initializers */
    struct Complex {
        int a;
        float b;
        int c[4];
        struct {
            int x;
            int y;
        } nested;
    };
    
    /* Partial and nested initialization */
    struct Complex c1 = {
        .a = 1,
        .c = {[0] = 10, [2] = 30},
        .nested = {.x = 100, .y = 200}
    };
    
    struct Complex c2 = {
        .b = 2.5f,
        .c = {[1] = 20, [3] = 40},
        .nested.x = 300
    };
    
    /* Array with designated initializers */
    int arr[10] = {[0] = 1, [5] = 2, [9] = 3};
    
    /* Union initializers */
    union Data {
        int i;
        float f;
        char str[4];
    };
    
    union Data u1 = {.i = 42};
    union Data u2 = {.f = 3.14f};
    union Data u3 = {.str = "ABC"};
    
    /* Nested struct initializer */
    struct Outer {
        struct Inner {
            int a;
            int b;
        } inner;
        int c;
    };
    
    struct Outer o = {
        .inner = {.a = 1, .b = 2},
        .c = 3
    };
    
    /* Zero initialization mixed with designated */
    struct Complex c3 = {0};
    c3.a = 5;
    c3.c[1] = 6;
    
    global_sink += c1.a + c2.nested.x + arr[5] + u1.i + o.inner.a + c3.a;
}

/* Function to create OMP_CLAUSE nodes */
void test_omp_clauses(int n) {
    int i, sum = 0;
    int private_var = 0;
    static int shared_var = 0;
    int firstprivate_var = 5;
    int lastprivate_var = 0;
    int reduction_sum = 0;
    
    /* OpenMP with multiple clauses */
    #pragma omp parallel for private(private_var) \
             firstprivate(firstprivate_var) \
             lastprivate(lastprivate_var) \
             shared(shared_var) \
             reduction(+:reduction_sum) \
             schedule(dynamic, 4) \
             num_threads(2) \
             if(n > 1000)
    for (i = 0; i < n; i++) {
        private_var = i;
        firstprivate_var += i;
        reduction_sum += i;
        if (i == n-1) {
            lastprivate_var = i;
        }
    }
    
    /* Another parallel region with different clauses */
    #pragma omp parallel sections private(private_var) \
             nowait
    {
        #pragma omp section
        {
            private_var = 1;
            #pragma omp atomic
            shared_var += private_var;
        }
        
        #pragma omp section
        {
            private_var = 2;
            #pragma omp atomic
            shared_var += private_var;
        }
    }
    
    /* Parallel with collapse clause */
    #pragma omp parallel for collapse(2) \
             ordered \
             reduction(*:reduction_sum)
    for (i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            #pragma omp ordered
            reduction_sum += i * j;
        }
    }
    
    global_sink = reduction_sum + shared_var + lastprivate_var;
}

/* Main test driver */
int main() {
    int checksum = 0;
    
    printf("Testing GCC tree node coverage...\n");
    
    /* Test 1: IDENTIFIER_NODE */
    test_identifiers();
    checksum += global_sink;
    printf("  IDENTIFIER_NODE test complete\n");
    
    /* Test 2: TREE_VEC */
    test_tree_vec();
    checksum += global_sink;
    printf("  TREE_VEC test complete\n");
    
    /* Test 3: SSA_NAME (with optimization) */
    checksum += test_ssa_names(50);
    printf("  SSA_NAME test complete\n");
    
    /* Test 4: BLOCK nodes */
    test_blocks();
    checksum += global_sink;
    printf("  BLOCK test complete\n");
    
    /* Test 5: CONSTRUCTOR nodes */
    test_constructors();
    checksum += global_sink;
    printf("  CONSTRUCTOR test complete\n");
    
    /* Test 6: OMP_CLAUSE nodes */
    test_omp_clauses(100);
    checksum += global_sink;
    printf("  OMP_CLAUSE test complete\n");
    
#ifdef __cplusplus
    /* Test 7: TREE_BINFO (C++ only) */
    use_hierarchy();
    checksum += global_sink;
    printf("  TREE_BINFO test complete\n");
#endif
    
    /* Call external functions to create unresolved identifiers */
    checksum += external_func1();
    external_func2(checksum);
    
    printf("Final checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}
