/* tree_node_coverage.c - Test program to trigger specific GCC tree node generation */
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
};

void use_inheritance() {
    Derived d;
    d.a = 1;  /* This should generate BINFO nodes */
    d.b = 2;
    
    Base* bp = &d;
    bp->vfunc();
    
    DeepDerived dd;
    dd.a = 3;
    dd.b = 4;
    dd.c = 5;
}
#endif

/* Function to trigger SSA_NAME creation with complex control flow */
int ssa_test(int n) {
    int i, s = 0, t = 1, u = 0;
    
    /* Complex loop with multiple branches and updates */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t ^= s;
        } else {
            s *= 2;
            u += s;
        }
        
        if (s > 1000) {
            s /= 2;
            t = u % 7;
        }
        
        switch (i % 4) {
            case 0: u += 1; break;
            case 1: u += t; break;
            case 2: u += s; break;
            case 3: u += i; break;
        }
    }
    
    return s + t + u;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
void identifier_test(void) {
    int result = 0;
    
    /* Level 1 */
    {
        volatile int x = 1;
        result += x;
        
        /* Level 2 */
        {
            /* Different x in inner scope */
            volatile int x = x + 2;  /* Uses outer x in initializer */
            result += x;
            
            /* Level 3 */
            {
                /* Another x */
                extern int x;  /* Declaration only */
                volatile int y = 3;
                result += y;
                
                /* Level 4 - function scope */
                {
                    static int x = 4;  /* Static variable */
                    volatile int z = x;
                    result += z;
                    
                    /* Level 5 - loop scope */
                    for (int x = 0; x < 3; x++) {
                        volatile int w = x;
                        result += w;
                        
                        /* Level 6 - if scope */
                        if (w > 0) {
                            volatile int x = w * 2;
                            result += x;
                        }
                    }
                }
            }
        }
    }
    
    /* More identifier variations */
    {
        /* Same name in different types */
        volatile float x = 1.5f;
        volatile double xd = 2.5;
        result += (int)x + (int)xd;
    }
    
    global_sink = result;
}

/* Function for TREE_VEC nodes using vector extensions */
void vector_test(void) {
    int result = 0;
    
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    /* Extract elements */
    for (int i = 0; i < 4; i++) {
        result += c[i] + d[i];
    }
    
    /* Float vectors */
    v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fb = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fc = fa * fb;
    
    /* Array compound literals */
    int* arr1 = (int[]){1, 2, 3, 4, 5};
    int* arr2 = (int[3]){10, 20, 30};
    
    for (int i = 0; i < 3; i++) {
        result += arr1[i] + arr2[i];
    }
    
    /* Nested array initializer */
    int* arr3 = (int[]){[0] = 100, [2] = 200, [4] = 300};
    result += arr3[0] + arr3[2] + arr3[4];
    
    global_sink = result;
}

/* Function with complex blocks for BLOCK nodes */
void block_test(void) {
    int result = 0;
    
    /* Block 1 */
    {
        int a = 1;
    label1:
        a += 2;
        
        /* Block 2 */
        {
            int b = 3;
            if (a > b) {
                goto label3;
            }
        label2:
            b *= 2;
            goto label4;
        }
        
        /* Block 3 */
        {
            int c = 5;
        label3:
            c += a;
            goto label2;
        }
    }
    
label4:
    /* Block 4 with switch */
    {
        int x = 0;
        switch (result % 3) {
            case 0: {
                int y = 10;
                x += y;
                break;
            }
            case 1: {
                int y = 20;
                x += y;
                goto label5;
            }
            case 2: {
                int y = 30;
                x += y;
                break;
            }
        }
        result += x;
    }
    
label5:
    /* Deeply nested try-catch in C++ */
    #ifdef __cplusplus
    try {
        /* Nested block in try */
        {
            int z = 100;
            if (z > 50) {
                throw z;
            }
        }
    } catch (int e) {
        /* Catch block */
        result += e;
    }
    #endif
    
    global_sink = result;
}

/* Function for CONSTRUCTOR nodes */
void constructor_test(void) {
    int result = 0;
    
    /* Struct with designated initializers */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Point p1 = { .x = 1, .y = 2, .z = 3 };
    struct Point p2 = { .y = 20, .z = 30, .x = 10 };
    struct Point p3 = { .x = 100, .z = 300 };  /* Partial init */
    
    result += p1.x + p2.y + p3.z;
    
    /* Nested struct */
    struct Line {
        struct Point start;
        struct Point end;
    };
    
    struct Line l1 = {
        .start = { .x = 1, .y = 2 },
        .end = { .x = 10, .y = 20, .z = 0 }
    };
    
    result += l1.start.x + l1.end.y;
    
    /* Array with designated initializers */
    int arr1[10] = { [0] = 1, [5] = 2, [9] = 3 };
    int arr2[] = { [0 ... 4] = 10, [5 ... 9] = 20 };
    
    for (int i = 0; i < 10; i++) {
        result += arr1[i] + arr2[i];
    }
    
    /* Union initializers */
    union Data {
        int i;
        float f;
        char str[20];
    };
    
    union Data d1 = { .i = 42 };
    union Data d2 = { .f = 3.14f };
    union Data d3 = { .str = "test" };
    
    result += d1.i + (int)d2.f + d3.str[0];
    
    /* Complex nested initializer */
    struct Complex {
        int a;
        struct {
            int b[3];
            int c;
        } inner;
        int d[2][2];
    };
    
    struct Complex comp = {
        .a = 1,
        .inner = {
            .b = {[1] = 5, [2] = 6},
            .c = 7
        },
        .d = {[0][0] = 8, [1][1] = 9}
    };
    
    result += comp.a + comp.inner.b[1] + comp.d[1][1];
    
    global_sink = result;
}

/* OpenMP test for OMP_CLAUSE nodes */
void omp_test(int n) {
    int i;
    int sum = 0;
    int product = 1;
    int private_var = 0;
    
    /* Test various OpenMP clauses */
    #pragma omp parallel for private(i) shared(sum, n) reduction(+:sum) schedule(dynamic, 2) if(n > 1000)
    for (i = 0; i < n; i++) {
        sum += i;
    }
    
    /* Another parallel region with different clauses */
    #pragma omp parallel private(private_var) firstprivate(n) copyin(global_sink) num_threads(4)
    {
        private_var = omp_get_thread_num();
        #pragma omp for reduction(*:product) ordered collapse(2)
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                #pragma omp ordered
                product *= (j + k + 1);
            }
        }
        
        /* Critical section */
        #pragma omp critical
        {
            sum += private_var;
        }
    }
    
    /* Sections with nowait */
    #pragma omp parallel sections private(i) lastprivate(product)
    {
        #pragma omp section
        {
            for (i = 0; i < 5; i++) {
                sum += i * 2;
            }
        }
        
        #pragma omp section
        {
            for (i = 5; i < 10; i++) {
                product *= i;
            }
        }
    }
    
    global_sink = sum + product;
}

/* Main test driver */
int main(void) {
    int checksum = 0;
    
    printf("Starting tree node coverage tests...\n");
    
    /* 1. Test IDENTIFIER_NODE generation */
    printf("Testing IDENTIFIER_NODE...\n");
    identifier_test();
    checksum += global_sink;
    
    /* 2. Test TREE_VEC generation */
    printf("Testing TREE_VEC...\n");
    vector_test();
    checksum += global_sink;
    
    /* 3. Test SSA_NAME generation */
    printf("Testing SSA_NAME...\n");
    checksum += ssa_test(100);
    
    /* 4. Test BLOCK generation */
    printf("Testing BLOCK...\n");
    block_test();
    checksum += global_sink;
    
    /* 5. Test CONSTRUCTOR generation */
    printf("Testing CONSTRUCTOR...\n");
    constructor_test();
    checksum += global_sink;
    
    #ifdef __cplusplus
    /* 6. Test BINFO generation (C++ only) */
    printf("Testing BINFO...\n");
    use_inheritance();
    #endif
    
    /* 7. Test OMP_CLAUSE generation */
    printf("Testing OMP_CLAUSE...\n");
    #ifdef _OPENMP
    omp_test(100);
    checksum += global_sink;
    #else
    printf("OpenMP not enabled, skipping OMP_CLAUSE test\n");
    #endif
    
    /* Use external functions to create unresolved identifiers */
    checksum += external_func1();
    external_func2(checksum);
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return checksum == 0 ? 0 : 1;
}
