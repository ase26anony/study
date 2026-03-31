/* test_tree.c - Comprehensive tree node coverage test */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func(int);
extern volatile int global_var;

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

void use_hierarchy() {
    DeepDerived dd;
    dd.a = 1;      /* Accesses through inheritance chain */
    dd.b = 2;
    dd.c = 3;
    Base* bp = &dd;
    bp->vfunc();   /* Virtual call */
}
#endif

/* Function to generate SSA_NAME nodes with complex control flow */
int ssa_test(int n) {
    int i, s = 0, t = 1, u = 2;
    
    /* Complex loop with multiple conditional updates */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t = s - u;
        } else {
            s *= 2 + u;
            u = t ^ s;
        }
        
        /* Nested condition */
        if (s > 100) {
            t = s / 3;
            if (u < 50) {
                u = t * 2;
            }
        } else {
            u = s + t;
        }
    }
    
    /* Another loop with phi nodes */
    int j = 0, k = 0;
    while (j < n) {
        if (j % 3 == 0) {
            k += j;
        } else if (j % 3 == 1) {
            k *= 2;
        } else {
            k = k > 0 ? k - 1 : k + 1;
        }
        j++;
    }
    
    return s + t + u + k;
}

/* Function with deeply nested blocks and labels */
void block_test(void) {
    int level1 = 0;
    
    /* Level 1 block */
    {
        int level2 = 1;
        volatile int sink1 = level2;
        
    block1_label:
        level2++;
        
        /* Level 2 block */
        {
            int level3 = 2;
            volatile int sink2 = level3;
            
            if (level2 < 10)
                goto block1_label;
                
            /* Level 3 block */
            {
                int level4 = 3;
                volatile int sink3 = level4;
                
            block2_label:
                level4 *= 2;
                if (level4 < 100)
                    goto block2_label;
            }
        }
    }
    
    /* Another block with switch */
    {
        int x = 0;
        
    switch_block:
        switch(x) {
            case 0: x = 1; goto switch_block;
            case 1: x = 2; break;
            default: break;
        }
    }
}

/* Function with complex identifier usage */
void identifier_test(void) {
    /* Shadowing in nested scopes */
    {
        int x = 1;
        volatile int y = x;
        
        {
            /* Different x in inner scope */
            float x = 2.0f;
            volatile float z = x;
            
            {
                /* Yet another x */
                char x = 'a';
                volatile char w = x;
                
                /* Reference outer x through pointer */
                {
                    int* px = &((int&)y);
                    volatile int v = *px;
                }
            }
        }
    }
    
    /* Loop variable shadowing */
    for (int i = 0; i < 5; i++) {
        volatile int loop_i = i;
        
        {
            /* Different i in block */
            double i = 3.14;
            volatile double d = i;
        }
    }
    
    /* Function parameter shadowing */
    {
        auto func = [](int param) {
            {
                float param = 1.5f;
                volatile float f = param;
            }
            return param + 1;
        };
        volatile int r = func(10);
    }
}

/* Function using GCC vector extensions */
void vector_test(void) {
    /* Various vector types */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c > d;
    
    v4sf f = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf g = f * 2.0f;
    
    v8hi h = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi i = h << 1;
    
    volatile v4si sink_c = c;
    volatile v4sf sink_g = g;
    volatile v8hi sink_i = i;
    
    /* Array compound literals */
    int* arr1 = (int[]){1, 2, 3, 4, 5};
    int* arr2 = (int[3]){10, 20, 30};
    
    struct Point {
        int x;
        int y;
    };
    
    struct Point* points = (struct Point[]){
        {.x = 1, .y = 2},
        {.x = 3, .y = 4},
        {.x = 5, .y = 6}
    };
    
    volatile int sink_arr = arr1[0] + arr2[1] + points[1].x;
}

/* Function with constructor nodes */
void constructor_test(void) {
    /* Nested struct with designated initializers */
    struct Inner {
        int a;
        int b[3];
    };
    
    struct Outer {
        int id;
        struct Inner inner;
        float values[4];
    };
    
    /* Complex initialization with partial and nested designators */
    struct Outer o1 = {
        .id = 1,
        .inner = {
            .a = 10,
            .b = {[0] = 100, [2] = 300}
        },
        .values = {0.1f, 0.2f, [3] = 0.4f}
    };
    
    struct Outer o2 = {
        .id = 2,
        .inner.a = 20,
        .inner.b[1] = 200,
        .values[2] = 0.3f
    };
    
    /* Union initialization */
    union Data {
        int i;
        float f;
        char str[4];
    };
    
    union Data d1 = {.i = 42};
    union Data d2 = {.f = 3.14f};
    union Data d3 = {.str = "ABC"};
    
    /* Array with mixed initialization */
    int matrix[3][3] = {
        [0][0] = 1, [0][2] = 3,
        [1][1] = 5,
        [2] = {7, 8, 9}
    };
    
    volatile int sink_o = o1.inner.b[0] + o2.values[2] + d1.i + matrix[1][1];
}

/* OpenMP test function */
void omp_test(int n) {
    int i;
    long sum = 0;
    int* array = (int*)malloc(n * sizeof(int));
    
    if (!array) return;
    
    /* Initialize array */
    for (i = 0; i < n; i++) {
        array[i] = i + 1;
    }
    
    /* OpenMP pragma with multiple clauses */
    #pragma omp parallel for private(i) shared(array, n) \
        reduction(+:sum) schedule(dynamic, 4) \
        default(none) if(n > 1000)
    for (i = 0; i < n; i++) {
        sum += array[i];
    }
    
    /* Another OpenMP region with different clauses */
    int max_val = 0;
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            /* Single region */
        }
        
        #pragma omp for reduction(max:max_val) ordered \
            collapse(2) lastprivate(i)
        for (int x = 0; x < 10; x++) {
            for (int y = 0; y < 10; y++) {
                #pragma omp ordered
                {
                    int val = x * 100 + y;
                    if (val > max_val) max_val = val;
                }
            }
        }
    }
    
    /* OMP sections */
    #pragma omp parallel sections private(i) \
        firstprivate(sum)
    {
        #pragma omp section
        {
            i = 1;
        }
        
        #pragma omp section
        {
            i = 2;
        }
    }
    
    volatile long sink_sum = sum;
    volatile int sink_max = max_val;
    
    free(array);
}

int main(void) {
    int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* 1. Test IDENTIFIER_NODE generation */
    printf("Testing IDENTIFIER_NODE...\n");
    identifier_test();
    checksum += 1;
    
    /* 2. Test TREE_VEC generation */
    printf("Testing TREE_VEC...\n");
    vector_test();
    checksum += 2;
    
    /* 3. Test SSA_NAME generation */
    printf("Testing SSA_NAME...\n");
    checksum += ssa_test(100);
    
    /* 4. Test BLOCK generation */
    printf("Testing BLOCK...\n");
    block_test();
    checksum += 4;
    
    /* 5. Test CONSTRUCTOR generation */
    printf("Testing CONSTRUCTOR...\n");
    constructor_test();
    checksum += 8;
    
    /* 6. Test OMP_CLAUSE generation */
    printf("Testing OMP_CLAUSE...\n");
    omp_test(10000);
    checksum += 16;
    
#ifdef __cplusplus
    /* 7. Test TREE_BINFO generation (C++ only) */
    printf("Testing TREE_BINFO...\n");
    use_hierarchy();
    checksum += 32;
#endif
    
    /* Use external identifiers */
    checksum += external_func(checksum);
    checksum += global_var;
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum == 0 ? 0 : 1;
}
