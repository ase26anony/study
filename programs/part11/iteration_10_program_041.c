/* test_tree.c - Comprehensive tree node coverage test */
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

void use_hierarchy() {
    Derived d;
    d.a = 1;  /* Accesses base member - creates BINFO */
    d.b = 2;
    
    DeepDerived dd;
    dd.a = 3;  /* Accesses through multiple inheritance */
    dd.c = 4;
    
    Base* bp = &d;
    bp->vfunc();  /* Virtual call */
}
#endif

/* Function to create SSA_NAME nodes with complex control flow */
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
    
    /* Another loop with phi nodes */
    int j = 0, k = 0;
    while (j < n) {
        k = (j % 3 == 0) ? k + j : k - j;
        j++;
    }
    
    return s + k + t;
}

/* Function with deeply nested blocks and labels */
void block_test(void) {
    int outer = 0;
    
    /* Level 1 block */
    {
        int a = 1;
    lab1:
        a = a * 2;
        
        /* Level 2 block */
        {
            int b = 3;
            if (a > b) {
                goto lab2;  /* Jump to outer block */
            }
            b = b + a;
        }
        
        /* Another level 2 block */
        {
            int c = 5;
        lab3:
            c = c - 1;
            if (c > 0) goto lab3;
        }
        
        a++;
    }
    
lab2:
    outer = 10;
    
    /* Deep nesting */
    {
        {
            {
                int deepest = 99;
            deepest_label:
                deepest--;
                if (deepest > 0) goto deepest_label;
            }
        }
    }
}

/* Function to create CONSTRUCTOR nodes */
void constructor_test(void) {
    /* Struct with designated initializers */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Point p1 = { .x = 1, .y = 2, .z = 3 };
    struct Point p2 = { .y = 5, .z = 6 };  /* Partial initialization */
    
    /* Nested struct with array */
    struct Container {
        struct Point pt;
        int values[5];
        char tag;
    };
    
    struct Container c1 = {
        .pt = { .x = 10, .y = 20 },
        .values = { [0] = 100, [2] = 200, [4] = 300 },
        .tag = 'A'
    };
    
    /* Union initialization */
    union Data {
        int i;
        float f;
        char str[4];
    };
    
    union Data d1 = { .i = 42 };
    union Data d2 = { .f = 3.14f };
    union Data d3 = { .str = "ABC" };
    
    /* Array with designated initializer */
    int arr[10] = { [0] = 1, [5] = 2, [9] = 3 };
    
    /* Compound literal */
    int* ptr = (int[]){1, 2, 3, 4, 5};
    
    global_sink = p1.x + c1.values[0] + d1.i + arr[5] + ptr[2];
}

/* Function using GCC vector extensions for TREE_VEC */
void vector_test(void) {
    /* Different vector types */
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
    v8hi h2 = h1 >> 1;
    
    /* Vector operations */
    v4si mask = a > b;
    v4si e = c + (d & mask);
    
    /* Store results to volatile to prevent optimization */
    volatile v4si sink_vec = e;
    (void)sink_vec;
}

/* OpenMP section for OMP_CLAUSE nodes */
void omp_test(int n) {
    int i, sum = 0;
    int private_var = 0;
    int shared_array[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        shared_array[i] = i;
    }
    
    /* Multiple OpenMP pragmas with various clauses */
    #pragma omp parallel for private(i) shared(shared_array) reduction(+:sum) schedule(dynamic, 4) num_threads(2)
    for (i = 0; i < n; i++) {
        sum += shared_array[i % 100];
    }
    
    /* Another parallel region with different clauses */
    #pragma omp parallel private(private_var) firstprivate(sum) copyin(global_sink)
    {
        private_var = omp_get_thread_num();
        #pragma omp critical
        {
            global_sink += private_var;
        }
    }
    
    /* Parallel sections */
    #pragma omp parallel sections private(i) reduction(*:sum)
    {
        #pragma omp section
        {
            for (i = 0; i < 10; i++) sum *= 2;
        }
        
        #pragma omp section
        {
            for (i = 0; i < 10; i++) sum /= 3;
        }
    }
    
    /* Task with dependencies */
    #pragma omp task depend(inout: sum) if(n > 1000)
    {
        sum = sum % 1000;
    }
    
    global_sink = sum;
}

/* Main test driver */
int main(void) {
    int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* Test 1: Create multiple IDENTIFIER_NODE instances */
    printf("Test 1: Creating IDENTIFIER_NODE instances...\n");
    {
        /* Deeply nested scopes with same variable names */
        volatile int x = 1;
        {
            extern int x;  /* Different declaration */
            volatile int y = x + 1;
            {
                double x = 3.14;  /* Different type */
                volatile int z = (int)x + y;
                checksum += z;
            }
        }
        
        /* More nesting */
        {
            long x = 100L;
            {
                char x = 'A';
                {
                    float x = 2.718f;
                    checksum += (int)x;
                }
            }
        }
        
        /* Function scope test */
        auto int func_local(void) {
            static int counter = 0;
            return counter++;
        }
        
        checksum += func_local();
    }
    
    /* Test 2: TREE_VEC nodes */
    printf("Test 2: Creating TREE_VEC nodes...\n");
    vector_test();
    checksum += global_sink;
    
    /* Test 3: SSA_NAME nodes */
    printf("Test 3: Creating SSA_NAME nodes...\n");
    checksum += ssa_test(50);
    
    /* Test 4: BLOCK nodes */
    printf("Test 4: Creating BLOCK nodes...\n");
    block_test();
    checksum += global_sink;
    
    /* Test 5: CONSTRUCTOR nodes */
    printf("Test 5: Creating CONSTRUCTOR nodes...\n");
    constructor_test();
    checksum += global_sink;
    
    /* Test 6: OMP_CLAUSE nodes */
    printf("Test 6: Creating OMP_CLAUSE nodes...\n");
    #ifdef _OPENMP
    omp_test(1000);
    checksum += global_sink;
    #else
    printf("OpenMP not enabled, skipping OMP_CLAUSE test\n");
    #endif
    
    /* Test 7: C++ specific BINFO nodes */
    printf("Test 7: Creating BINFO nodes...\n");
    #ifdef __cplusplus
    use_hierarchy();
    checksum += 999;  /* Arbitrary value for C++ mode */
    #else
    printf("Not in C++ mode, skipping BINFO test\n");
    #endif
    
    /* Call external functions for unresolved identifiers */
    printf("Test 8: Creating unresolved IDENTIFIER_NODE references...\n");
    checksum += external_func1();
    external_func2(checksum);
    checksum += (int)external_func3(checksum);
    
    /* Final checksum */
    printf("Final checksum: %d\n", checksum);
    
    /* Use all variables to prevent warnings */
    (void)checksum;
    
    return 0;
}
