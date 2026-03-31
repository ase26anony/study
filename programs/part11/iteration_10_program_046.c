/* test_tree.c - Comprehensive tree node coverage test */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
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

void use_hierarchy(DeepDerived *d) {
    d->a = 1;      /* Accesses through inheritance */
    d->b = 2;
    d->c = 3;
    checksum += d->a + d->b + d->c;
}
#endif

/* Function to generate SSA_NAME nodes */
int ssa_test(int n) {
    int i, s = 0, t = 1;
    
    /* Complex control flow for SSA */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t = s % 7;
        } else {
            s *= 2;
            t = (t + i) % 5;
        }
        
        /* Nested condition */
        if (s > 100) {
            s = s / 2;
            t = t * 3;
        }
    }
    
    /* Multiple assignments to same variable */
    int x = s;
    x = x + t;
    x = x * 2;
    x = x - 1;
    
    checksum += x;
    return x;
}

/* Function with BLOCK nodes and labels */
void block_test(void) {
    int a = 0;
    
    /* Outer block with label */
    outer_block: {
        int b = 1;
        checksum += b;
        
        /* Inner block */
        {
            int c = 2;
            volatile int prevent_merge = c;
            checksum += c;
            
            if (a < 5) {
                goto middle_block;
            }
        }
        
        /* Unreachable but creates BLOCK */
        {
            int d = 3;
            sink = d;
        }
    }
    
    middle_block: {
        int e = 4;
        checksum += e;
        
        /* Another nested block */
        {
            int f = 5;
            if (f > 0) {
                goto final_block;
            }
        }
    }
    
    final_block:
    a = 6;
    checksum += a;
}

/* Function for CONSTRUCTOR nodes */
void constructor_test(void) {
    /* Struct with designated initializers */
    struct S {
        int a;
        int b[3];
        struct {
            int x;
            int y;
        } nested;
    };
    
    /* Complex constructor with partial initialization */
    struct S s1 = { 
        .a = 1, 
        .b = {[0] = 10, [2] = 30},
        .nested = {.x = 100, .y = 200}
    };
    
    checksum += s1.a + s1.b[0] + s1.b[2] + s1.nested.x;
    
    /* Array with designated initializers */
    int arr[10] = {[0] = 1, [5] = 2, [9] = 3};
    checksum += arr[0] + arr[5] + arr[9];
    
    /* Union initializer */
    union U {
        int i;
        float f;
        char c[4];
    };
    
    union U u1 = {.i = 0x12345678};
    union U u2 = {.c = {'a', 'b', 'c', '\0'}};
    
    checksum += u1.i & 0xFF;
    checksum += u2.c[0];
    
    /* Nested struct initializer */
    struct Outer {
        struct Inner {
            int a;
            int b;
        } inner;
        int c;
    };
    
    struct Outer outer = {.inner = {.a = 7, .b = 8}, .c = 9};
    checksum += outer.inner.a + outer.inner.b + outer.c;
}

/* Function for TREE_VEC nodes using GCC vector extensions */
void vector_test(void) {
    /* Vector types using GCC extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    /* Vector initializations */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    /* Vector operations */
    v4si e = c + d;
    v4si f = e - a;
    
    /* Compound literal for array */
    int *p = (int[]){10, 20, 30, 40};
    checksum += p[0] + p[2];
    
    /* Float vector operations */
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = f1 * 2.0f;
    
    /* Extract elements for checksum */
    int *ai = (int*)&a;
    int *ci = (int*)&c;
    checksum += ai[0] + ai[2] + ci[1] + ci[3];
    
    /* More complex vector expressions */
    v8hi v1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi v2 = v1 << 1;
    short *sv = (short*)&v2;
    checksum += sv[0] + sv[4] + sv[7];
}

/* Function with IDENTIFIER_NODE variations */
void identifier_test(void) {
    /* Deeply nested scopes with same variable names */
    {
        int x = 1;
        volatile int y = x;
        checksum += y;
        
        {
            /* Shadowing with same name */
            int x = 2;
            volatile int z = x;
            checksum += z;
            
            {
                /* Another level */
                extern int x;  /* External declaration */
                volatile int w = 3;
                checksum += w;
                
                {
                    /* Function scope variable */
                    static int x = 4;
                    volatile int v = x;
                    checksum += v;
                }
            }
        }
    }
    
    /* More shadowing in loops */
    for (int i = 0; i < 3; i++) {
        int counter = i * 10;
        {
            int counter = i * 20;  /* Shadows outer counter */
            volatile int tmp = counter;
            checksum += tmp;
        }
        checksum += counter;
    }
    
    /* Switch with shadowing */
    int val = 2;
    switch (val) {
        case 1: {
            int local = 100;
            checksum += local;
            break;
        }
        case 2: {
            int local = 200;  /* Different scope, same name */
            checksum += local;
            break;
        }
    }
}

/* OpenMP test function */
#ifdef _OPENMP
void omp_test(void) {
    int i;
    int sum = 0;
    int array[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        array[i] = i + 1;
    }
    
    /* Multiple OpenMP pragmas with various clauses */
    #pragma omp parallel for private(i) shared(array, sum) reduction(+:sum) schedule(dynamic, 4) num_threads(2)
    for (i = 0; i < 100; i++) {
        sum += array[i];
    }
    
    checksum += sum;
    
    /* Another OpenMP region with different clauses */
    int max_val = 0;
    int min_val = 1000;
    
    #pragma omp parallel sections private(i) firstprivate(array) lastprivate(max_val, min_val) \
            default(none) shared(checksum)
    {
        #pragma omp section
        {
            max_val = array[0];
            for (i = 1; i < 100; i++) {
                if (array[i] > max_val) {
                    max_val = array[i];
                }
            }
        }
        
        #pragma omp section
        {
            min_val = array[0];
            for (i = 1; i < 100; i++) {
                if (array[i] < min_val) {
                    min_val = array[i];
                }
            }
        }
    }
    
    checksum += max_val + min_val;
    
    /* OpenMP with collapse clause */
    int matrix[10][10];
    int matrix_sum = 0;
    
    #pragma omp parallel for collapse(2) reduction(+:matrix_sum) ordered
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 10; col++) {
            matrix[row][col] = row * 10 + col;
            matrix_sum += matrix[row][col];
            #pragma omp ordered
            {
                /* Ordered region */
                checksum += matrix[row][col] % 7;
            }
        }
    }
    
    checksum += matrix_sum;
}
#endif

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
    int ssa_result = ssa_test(20);
    sink = ssa_result;
    
    /* Test 4: BLOCK */
    printf("Testing BLOCK...\n");
    block_test();
    
    /* Test 5: CONSTRUCTOR */
    printf("Testing CONSTRUCTOR...\n");
    constructor_test();
    
#ifdef __cplusplus
    /* Test 6: TREE_BINFO (C++ only) */
    printf("Testing TREE_BINFO...\n");
    DeepDerived dd;
    use_hierarchy(&dd);
#endif
    
#ifdef _OPENMP
    /* Test 7: OMP_CLAUSE */
    printf("Testing OMP_CLAUSE...\n");
    omp_test();
#endif
    
    /* Final checksum */
    printf("Final checksum: %d\n", checksum);
    
    /* Use external identifiers */
    if (external_var > 0) {
        external_func2(checksum);
    }
    
    return checksum & 0xFF;
}
