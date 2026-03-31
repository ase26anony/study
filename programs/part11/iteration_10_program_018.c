/* test_tree.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(int);
extern void external_func2(void);
extern volatile int external_var;

/* Sink function to prevent optimization */
static void sink(void *p) {
    volatile static void *sink_ptr;
    sink_ptr = p;
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

void use_hierarchy() {
    DeepDerived dd;
    dd.a = 1;      /* Accesses through inheritance chain */
    dd.b = 2;
    dd.c = 3;
    Base *bp = &dd;
    bp->vfunc();   /* Virtual call */
    sink(&dd);
}
#endif

/* Function with complex control flow for SSA_NAME */
int ssa_test(int n) {
    int i, s = 0, t = 1, u = 0;
    
    /* Complex loop with multiple updates */
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
            case 1: t -= s; break;
            case 2: u ^= t; break;
            case 3: s = t + u; break;
        }
    }
    
    /* Nested loops with variable updates */
    while (s > 0) {
        for (int j = 0; j < 5; j++) {
            s -= j;
            if (s < 0) goto exit_loop;
        }
        t++;
    }
exit_loop:
    
    return s + t + u;
}

/* Function with many blocks and labels */
void block_test(void) {
    int x = 0;
    
    block1: {
        volatile int a = 1;
        x += a;
        goto block3;
    }
    
    block2: {
        volatile int b = 2;
        x += b;
        goto block4;
    }
    
    block3: {
        volatile int c = 3;
        x += c;
        goto block2;
    }
    
    block4: {
        volatile int d = 4;
        x += d;
        
        /* Deeply nested block */
        {
            int e = 5;
            {
                volatile int f = 6;
                {
                    int g = 7;
                    x += e + f + g;
                }
            }
        }
    }
    
    sink(&x);
}

/* Constructor nodes with various initializations */
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

union MixedUnion {
    int i;
    float f;
    struct {
        char a;
        char b;
    } chars;
};

void constructor_test(void) {
    /* Designated initializers with partial initialization */
    struct ComplexStruct cs1 = {
        .a = 10,
        .b = {[0] = 1, [2] = 3, [3] = 4},
        .nested = {.x = 5},
        .u = {.u2 = 3.14f}
    };
    
    /* Array compound literal */
    int *arr = (int[]){1, 2, 3, 4, 5};
    
    /* Nested designated initializers */
    struct ComplexStruct cs2 = {
        .b = {[1] = 10, [3] = 20},
        .nested.x = 100,
        .nested.y = 200,
        .a = 50
    };
    
    /* Union initializer */
    union MixedUnion mu = {.chars = {.a = 'A', .b = 'B'}};
    
    /* Zero initialization with partial designators */
    struct ComplexStruct cs3 = {0};
    
    sink(&cs1);
    sink(arr);
    sink(&cs2);
    sink(&mu);
    sink(&cs3);
}

/* Vector operations for TREE_VEC */
void vector_test(void) {
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
    
    /* Array compound literal in vector context */
    int *p = (int[]){10, 20, 30, 40};
    v4si j = {p[0], p[1], p[2], p[3]};
    
    /* Vector operations */
    v4si k = a > b;
    v4si l = a & b;
    
    sink(&c);
    sink(&g);
    sink(&i);
    sink(&j);
    sink(&k);
    sink(&l);
}

/* OpenMP pragmas for OMP_CLAUSE nodes */
void omp_test(int *array, int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    
    #pragma omp parallel for private(i) shared(array, n) reduction(+:sum) \
            schedule(dynamic, 4) num_threads(2) if(n > 100)
    for (i = 0; i < n; i++) {
        sum += array[i];
    }
    
    #pragma omp parallel private(private_var) firstprivate(sum)
    {
        private_var = omp_get_thread_num();
        #pragma omp critical
        {
            sum += private_var;
        }
    }
    
    #pragma omp sections nowait
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
    
    sink(&sum);
}

/* Helper for OpenMP */
int omp_get_thread_num(void) {
    return 0;
}

/* Main test driver */
int main(void) {
    int checksum = 0;
    
    printf("Testing GCC tree node coverage...\n");
    
    /* 1. Test IDENTIFIER_NODE creation with shadowing */
    {
        int x = 1;
        checksum += x;
        
        {
            extern int x;  /* Different identifier node */
            volatile int y = x;
            checksum += y;
        }
        
        {
            long x = 2L;   /* Another shadowing */
            checksum += (int)x;
            
            {
                double x = 3.0;  /* More shadowing */
                checksum += (int)x;
                
                {
                    /* Multiple declarations in same scope */
                    int a = 1, b = 2, c = 3;
                    volatile int d = a + b + c;
                    checksum += d;
                }
            }
        }
    }
    
    /* 2. Test SSA_NAME creation */
    checksum += ssa_test(20);
    
    /* 3. Test BLOCK nodes */
    block_test();
    checksum += 1;
    
    /* 4. Test CONSTRUCTOR nodes */
    constructor_test();
    checksum += 2;
    
    /* 5. Test TREE_VEC nodes */
    vector_test();
    checksum += 3;
    
    /* 6. Test OpenMP clauses */
    int arr[50];
    for (int i = 0; i < 50; i++) arr[i] = i;
    omp_test(arr, 50);
    checksum += 4;
    
#ifdef __cplusplus
    /* 7. Test C++ BINFO nodes */
    use_hierarchy();
    checksum += 5;
#endif
    
    /* Use external identifiers */
    checksum += external_var;
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
