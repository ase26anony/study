/* tree_node_coverage.c - Test program to trigger specific GCC tree node types */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifier nodes */
extern int external_func1(void);
extern void external_func2(int);
extern volatile int external_var;

/* Sink function to prevent optimization */
static volatile int sink;

/* Function to accumulate checksum */
static int checksum = 0;

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
    Derived d;
    d.a = 1;  /* This should generate BINFO nodes */
    d.b = 2;
    
    Base* bp = &d;
    bp->vfunc();
    
    DeepDerived dd;
    dd.a = 3;
    dd.c = 4;
}
#endif

/* Function with complex control flow for SSA_NAME generation */
int ssa_test(int n) {
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
        if (s > 100) {
            s = s % 100;
            t = t >> 1;
        } else if (s < -50) {
            s = -s;
            t = t << 1;
        }
    }
    
    /* Switch with variable modifications */
    switch (n % 4) {
        case 0: s = s + t; break;
        case 1: s = s - t; break;
        case 2: s = s * t; break;
        case 3: s = s / (t ? t : 1); break;
    }
    
    return s;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
void identifier_test(void) {
    /* Level 1 */
    {
        int x = 1;
        volatile int y = x;
        checksum += x;
        
        /* Level 2 - shadowing */
        {
            int x = 2;  /* Different identifier node for same name */
            volatile int z = x;
            checksum += x;
            
            /* Level 3 - extern declaration */
            {
                extern int x;  /* Another identifier node */
                volatile int w = 0;
                checksum += w;
                
                /* Level 4 - function scope */
                {
                    static int x = 3;  /* Static variable */
                    volatile int v = x;
                    checksum += x;
                }
            }
        }
        
        /* Another branch with same name */
        {
            long x = 4;  /* Different type */
            volatile long l = x;
            checksum += (int)x;
        }
    }
    
    /* Multiple scopes with same variable names */
    for (int i = 0; i < 3; i++) {
        int counter = i * 10;
        {
            int counter = i * 20;  /* Shadowed */
            checksum += counter;
        }
        checksum += counter;
    }
}

/* Function for TREE_VEC nodes using vector extensions */
void vector_test(void) {
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    /* Compound literal array */
    int *arr1 = (int[]){1, 2, 3, 4, 5};
    int *arr2 = (int[]){[0] = 10, [2] = 20, [4] = 30};
    
    /* Vector operations */
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = f1 * 2.0f;
    
    /* Use results to prevent elimination */
    sink = c[0] + d[1];
    checksum += arr1[0] + arr2[2];
}

/* Function with BLOCK nodes and labels */
void block_test(void) {
    int a = 0;
    
    /* Block 1 */
    {
        int b = 1;
    lab1:
        b = a + 1;
        checksum += b;
        
        /* Nested block */
        {
            int c = 2;
            goto lab3;  /* Jump forward */
        lab2:
            c = 3;
            checksum += c;
        }
    }
    
    /* Block 2 */
    {
        int d = 4;
        goto lab4;
        
    lab3:
        d = 5;
        checksum += d;
        goto lab2;
        
    lab4:
        d = 6;
        checksum += d;
    }
    
    /* Loop with labeled break */
    for (int i = 0; i < 5; i++) {
        if (i == 2) {
            goto end_loop;
        }
        checksum += i;
    }
end_loop:
    a = 10;
}

/* Function for CONSTRUCTOR nodes */
void constructor_test(void) {
    /* Struct with designated initializers */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Point p1 = { .x = 1, .y = 2, .z = 3 };
    struct Point p2 = { .y = 20, .z = 30, .x = 10 };
    struct Point p3 = { .x = 100 };  /* Partial initialization */
    
    /* Nested struct */
    struct Rectangle {
        struct Point top_left;
        struct Point bottom_right;
    };
    
    struct Rectangle rect = {
        .top_left = { .x = 0, .y = 0 },
        .bottom_right = { .x = 100, .y = 50 }
    };
    
    /* Array with designated initializers */
    int arr1[10] = { [0] = 1, [5] = 2, [9] = 3 };
    int arr2[5] = { [1 ... 3] = 42 };
    
    /* Union initializer */
    union Data {
        int i;
        float f;
        char str[20];
    };
    
    union Data data1 = { .i = 100 };
    union Data data2 = { .f = 3.14f };
    
    /* Complex nested initializer */
    struct Complex {
        int a;
        int b[3];
        struct {
            int x;
            int y;
        } nested;
    };
    
    struct Complex comp = {
        .a = 1,
        .b = {[1] = 5, [2] = 6},
        .nested = { .x = 10, .y = 20 }
    };
    
    checksum += p1.x + p2.y + p3.x + rect.top_left.x + arr1[5] + arr2[2] + data1.i + comp.b[1];
}

/* OpenMP test for OMP_CLAUSE nodes */
void omp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    int shared_var = 0;
    
    /* Test various OpenMP clauses */
    #pragma omp parallel for private(i) shared(sum) reduction(+:sum) schedule(dynamic, 2) if(n > 100)
    for (i = 0; i < n; i++) {
        sum += i;
    }
    
    #pragma omp parallel private(private_var) firstprivate(n) shared(shared_var) \
                num_threads(4) default(none)
    {
        private_var = omp_get_thread_num();
        shared_var += private_var;
    }
    
    /* Sections with nowait */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < 10; i++) {
                sum += i;
            }
        }
        
        #pragma omp section
        {
            for (i = 10; i < 20; i++) {
                sum -= i;
            }
        }
    }
    
    /* Single with copyprivate */
    int broadcast_val = 0;
    #pragma omp parallel private(broadcast_val)
    {
        #pragma omp single copyprivate(broadcast_val)
        {
            broadcast_val = 42;
        }
        
        sum += broadcast_val;
    }
    
    checksum += sum;
}

/* Main test driver */
int main(void) {
    printf("Starting tree node coverage test...\n");
    
    /* Test 1: IDENTIFIER_NODE */
    printf("Testing IDENTIFIER_NODE...\n");
    identifier_test();
    
    /* Test 2: TREE_VEC */
    printf("Testing TREE_VEC...\n");
    vector_test();
    
    /* Test 3: SSA_NAME */
    printf("Testing SSA_NAME...\n");
    int ssa_result = ssa_test(50);
    checksum += ssa_result;
    
    /* Test 4: BLOCK */
    printf("Testing BLOCK...\n");
    block_test();
    
    /* Test 5: CONSTRUCTOR */
    printf("Testing CONSTRUCTOR...\n");
    constructor_test();
    
    /* Test 6: OpenMP clauses (OMP_CLAUSE) */
    printf("Testing OMP_CLAUSE...\n");
    #ifdef _OPENMP
    #include <omp.h>
    omp_test(200);
    #else
    printf("OpenMP not enabled, skipping OMP_CLAUSE test\n");
    #endif
    
    /* Test 7: C++ BINFO nodes */
    #ifdef __cplusplus
    printf("Testing BINFO (C++ inheritance)...\n");
    use_inheritance();
    #else
    printf("Not in C++ mode, skipping BINFO test\n");
    #endif
    
    /* Use external identifiers */
    checksum += external_var;
    external_func2(checksum);
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
