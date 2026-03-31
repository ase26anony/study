/* test_tree.c - Comprehensive tree node coverage test */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(int);
extern void external_func2(void);
extern volatile int external_var;

/* Helper to prevent optimization */
static volatile int sink;

/* Function to accumulate checksum */
static int checksum = 0;

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
};

void use_inheritance() {
    DeepDerived dd;
    dd.a = 1;      /* Accesses through inheritance hierarchy */
    dd.b = 2;
    dd.c = 3;
    sink = dd.a + dd.b + dd.c;
}
#endif

/* Function with complex control flow for SSA_NAME */
int ssa_test(int n) {
    int i, s = 0, t = 1;
    
    /* Complex loop with multiple assignments */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t = s % 7;
        } else {
            s *= 2;
            t += i;
        }
        
        /* Nested condition */
        if (s > 100) {
            s = s / 2;
            t = t * 3;
        }
    }
    
    /* Multiple paths to return */
    if (n > 0) {
        return s + t;
    } else {
        return s - t;
    }
}

/* Function with many blocks and labels for BLOCK nodes */
void block_test(void) {
    int x = 0;
    
    /* Outer block with label */
    outer_block: {
        int y = 1;
        
        /* Inner block 1 */
        {
            int z = 2;
            if (x < 10) {
                goto middle_block;
            }
            z = x + y;
        }
        
        /* This should be skipped by goto */
        x = 99;
        
        middle_block: {
            int a = 3;
            
            /* Deeply nested block */
            {
                int b = 4;
                if (x == 0) {
                    goto inner_label;
                }
                b = a * 2;
            }
            
            inner_label:
            a = x + 1;
            
            /* Jump to outer block */
            if (a < 5) {
                goto outer_block;
            }
        }
        
        /* Another block with switch */
        {
            int val = x % 3;
            switch (val) {
                case 0: goto case_label0;
                case 1: goto case_label1;
                default: goto case_label2;
            }
            
            case_label0:
            y = 10;
            break;
            
            case_label1:
            y = 20;
            break;
            
            case_label2:
            y = 30;
            break;
        }
    }
    
    sink = x;
}

/* Function with designated initializers for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    int b[4];
    struct {
        int x;
        int y;
    } nested;
    union {
        int u1;
        float u2;
    } u;
};

struct ComplexArray {
    int data[3][2];
};

void constructor_test(void) {
    /* Struct with designated initializers */
    struct ComplexStruct cs1 = {
        .a = 1,
        .b = {[0] = 10, [2] = 30, [3] = 40},  /* Partial array init */
        .nested = {.x = 100, .y = 200},
        .u = {.u2 = 3.14f}
    };
    
    /* Another with different union member */
    struct ComplexStruct cs2 = {
        .a = 2,
        .b = {[1] = 20},  /* Only middle element */
        .nested.x = 300,   /* Direct nested member */
        .u.u1 = 42         /* Different union member */
    };
    
    /* Array with designated initializers */
    struct ComplexArray ca = {
        .data = {
            [0] = {1, 2},
            [2] = {5, 6}   /* Skip middle row */
        }
    };
    
    /* Compound literal with designators */
    int *p = (int[]){[0] = 1, [9] = 10, [4] = 5};
    
    sink = cs1.a + cs2.a + ca.data[0][0] + p[0];
}

/* OpenMP function for OMP_CLAUSE nodes */
void omp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    static int shared_var = 0;
    
    /* Multiple OpenMP pragmas with various clauses */
    #pragma omp parallel for private(i) shared(shared_var) reduction(+:sum) \
            schedule(dynamic, 2) if(n > 1000)
    for (i = 0; i < n; i++) {
        sum += i;
        shared_var++;
    }
    
    /* Another with different clauses */
    #pragma omp parallel private(private_var) firstprivate(n) \
            copyin(shared_var) num_threads(4)
    {
        private_var = omp_get_thread_num();
        #pragma omp critical
        {
            shared_var += private_var;
        }
    }
    
    /* Parallel sections */
    #pragma omp parallel sections private(i) \
            lastprivate(private_var) nowait
    {
        #pragma omp section
        {
            for (i = 0; i < 10; i++) {
                private_var = i;
            }
        }
        
        #pragma omp section
        {
            for (i = 10; i < 20; i++) {
                private_var = i * 2;
            }
        }
    }
    
    sink = sum + shared_var + private_var;
}

/* Vector operations for TREE_VEC */
void vector_test(void) {
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c > d;
    
    /* Mixed operations */
    v4sf f = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf g = f * 2.0f;
    
    /* Array compound literals */
    int *arr1 = (int[]){1, 2, 3, 4, 5};
    int *arr2 = (int[3]){10, 20, 30};
    
    /* Multi-dimensional compound literal */
    int (*arr3)[2] = (int[][2]){{1, 2}, {3, 4}, {5, 6}};
    
    sink = c[0] + d[1] + (int)e[2] + (int)g[0] + arr1[0] + arr2[1] + arr3[1][0];
}

/* Main test function with all cases */
int main(void) {
    int result = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* 1. IDENTIFIER_NODE - Deeply nested scopes with same variable names */
    {
        int x = 1;
        checksum += x;
        
        {
            /* Shadow outer x */
            int x = 2;
            volatile int y = x;
            checksum += y;
            
            {
                /* Another shadow */
                extern int x;  /* Unexterned identifier */
                volatile int z = x;  /* Should use external */
                checksum += z;
                
                {
                    /* And another */
                    float x = 3.14f;
                    volatile float w = x;
                    checksum += (int)w;
                }
            }
        }
        
        /* Function scope test */
        {
            auto int x = 4;  /* C++ or C11 */
            volatile int v = x;
            checksum += v;
        }
    }
    
    /* 2. TREE_VEC nodes */
    vector_test();
    checksum += sink;
    
    /* 3. TREE_BINFO nodes (C++ only) */
#ifdef __cplusplus
    use_inheritance();
    checksum += sink;
#endif
    
    /* 4. SSA_NAME nodes */
    result = ssa_test(50);
    checksum += result;
    
    /* 5. BLOCK nodes */
    block_test();
    checksum += sink;
    
    /* 6. CONSTRUCTOR nodes */
    constructor_test();
    checksum += sink;
    
    /* 7. OMP_CLAUSE nodes */
#ifdef _OPENMP
    omp_test(100);
    checksum += sink;
#endif
    
    /* Additional identifier stress test */
    {
        /* Multiple extern declarations */
        extern int external_func1(int);
        extern double external_func3(double);
        extern char external_func4(void);
        
        /* Use in complex expression */
        volatile int tmp = external_func1(42);
        checksum += tmp;
        
        /* Call through pointer */
        void (*func_ptr)(void) = external_func2;
        if (func_ptr) {
            /* Generate call expr */
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum == 0 ? 0 : 1;
}

/* Additional functions to create more tree nodes */
static void extra_functions(void) {
    /* Function with try-catch for C++ (exception handling nodes) */
    #ifdef __cplusplus
    try {
        throw 42;
    } catch (int e) {
        sink = e;
    }
    #endif
    
    /* Function with computed goto (GCC extension) */
    {
        static void *labels[] = {&&label1, &&label2, &&label3};
        int i = 0;
        
        goto *labels[i];
        
        label1:
        i = 1;
        goto end;
        
        label2:
        i = 2;
        goto end;
        
        label3:
        i = 3;
        goto end;
        
        end:
        sink = i;
    }
}
