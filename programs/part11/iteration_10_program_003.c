/* test_tree.c - Comprehensive tree node coverage test */
#include <stdio.h>
#include <stdlib.h>

/* External functions to create unresolved identifiers */
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
    DeepDerived dd;
    dd.a = 1;      /* Accesses through inheritance chain */
    dd.b = 2;
    dd.c = 3;
    Base* bp = &dd;
    bp->vfunc();   /* Virtual call */
    global_sink = dd.a + dd.b + dd.c;
}
#endif

/* Function to generate SSA_NAME nodes with complex control flow */
int ssa_generator(int n) {
    int i, j, k = 0;
    int result = 0;
    
    /* Complex loop with multiple assignments */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            j = i * 2;
            result += j;
        } else {
            j = i / 2;
            result -= j;
        }
        
        /* Nested condition */
        if (result > 100) {
            k = result % 7;
            result = k;
        } else if (result < -50) {
            k = result * 3;
            result = k + 1;
        } else {
            k = i;
            result += k * 2;
        }
        
        /* Switch creates more SSA complexity */
        switch (i % 4) {
            case 0: result <<= 1; break;
            case 1: result >>= 1; break;
            case 2: result ^= 0xFF; break;
            case 3: result |= 0xAA; break;
        }
    }
    
    return result;
}

/* Function with deeply nested blocks and labels */
void block_generator(void) {
    int outer = 0;
    
    /* Level 1 block */
    {
        int a = 1;
        volatile int prevent_merge1 = a;
        
    level1_label:
        a++;
        
        /* Level 2 block */
        {
            int b = 2;
            volatile int prevent_merge2 = b;
            
        level2_label:
            b += a;
            
            /* Level 3 block */
            {
                int c = 3;
                volatile int prevent_merge3 = c;
                
            level3_label:
                c = a + b;
                
                /* Jump between blocks */
                if (c > 10) goto level1_label;
                if (c < 5) goto level2_label;
                
                global_sink = c;
            }
            
            if (b < 20) goto level3_label;
        }
        
        if (a < 15) goto level2_label;
    }
    
    /* Another block with same variable name */
    {
        int a = 100;  /* Same name, different scope */
        volatile int x = a;
        {
            extern int a;  /* External declaration in nested scope */
            volatile int y = a;
        }
    }
}

/* Function to generate CONSTRUCTOR nodes */
void constructor_generator(void) {
    /* Struct with designated initializers */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Nested {
        struct Point p;
        int arr[4];
        union {
            int i;
            float f;
        } u;
    };
    
    /* Complex designated initializers */
    struct Point p1 = { .y = 2, .x = 1, .z = 3 };
    struct Point p2 = { .x = 10 };
    struct Point p3 = { 0 };
    
    /* Array with designated initializers */
    int arr1[10] = { [0] = 1, [5] = 2, [9] = 3 };
    int arr2[] = { [0 ... 4] = 1, [5 ... 9] = 2 };
    
    /* Nested designated initializers */
    struct Nested n1 = {
        .p = { .x = 1, .y = 2 },
        .arr = { [1] = 10, [3] = 20 },
        .u = { .f = 3.14f }
    };
    
    /* Partial initialization */
    struct Nested n2 = {
        .p.z = 100,
        .arr[2] = 50
    };
    
    /* Union initializer */
    union Data {
        int i;
        float f;
        char str[20];
    } data = { .f = 2.718f };
    
    global_sink = p1.x + p2.x + arr1[5] + n1.arr[1] + (int)data.f;
}

/* Function using GCC vector extensions for TREE_VEC */
void vector_generator(void) {
    /* Various vector types */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    /* Vector initializations */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = a & b;
    
    /* Vector operations */
    v4sf f = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf g = f * f;
    v4sf h = f + g;
    
    v8hi v1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi v2 = v1 << 1;
    
    /* Array compound literals */
    int *ptr1 = (int[]){1, 2, 3, 4};
    int *ptr2 = (int[]){[0] = 10, [3] = 40};
    float *ptr3 = (float[]){1.0f, [2] = 3.0f, 4.0f};
    
    /* Vector in struct */
    struct VecStruct {
        v4si vec;
        int scalar;
    } vs = { .vec = {10, 20, 30, 40}, .scalar = 100 };
    
    /* Prevent optimization */
    volatile v4si sink1 = c;
    volatile v4sf sink2 = h;
    global_sink = vs.vec[0] + ptr1[0] + (int)sink1[0];
}

/* OpenMP section for OMP_CLAUSE nodes */
void omp_generator(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    int shared_var = 100;
    int reduction_sum = 0;
    int lastprivate_var = 0;
    
    /* Complex OpenMP pragma with multiple clauses */
    #pragma omp parallel for private(private_var) shared(shared_var) \
             reduction(+:reduction_sum) schedule(dynamic, 4) \
             firstprivate(shared_var) lastprivate(lastprivate_var) \
             num_threads(4) if(n > 1000)
    for (i = 0; i < n; i++) {
        private_var = i * 2;
        reduction_sum += private_var;
        lastprivate_var = i;
        
        /* Nested parallel region */
        #pragma omp parallel sections private(private_var) \
                 reduction(*:reduction_sum)
        {
            #pragma omp section
            {
                private_var = i * 3;
                reduction_sum *= (private_var > 0 ? private_var : 1);
            }
            
            #pragma omp section
            {
                private_var = i * 4;
                reduction_sum *= (private_var > 0 ? private_var : 1);
            }
        }
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel private(private_var) \
             copyin(shared_var) proc_bind(close)
    {
        #pragma omp single copyprivate(private_var) nowait
        {
            private_var = shared_var;
        }
        
        #pragma omp for ordered schedule(guided)
        for (i = 0; i < 100; i++) {
            #pragma omp ordered
            {
                sum += i;
            }
        }
    }
    
    /* Task with dependencies */
    #pragma omp task depend(in: shared_var) depend(out: private_var) \
             priority(10) untied mergeable
    {
        private_var = shared_var * 2;
    }
    
    global_sink = sum + reduction_sum + lastprivate_var + private_var;
}

/* Function with many identifier nodes in nested scopes */
void identifier_generator(void) {
    int x = 1;
    volatile int sink = x;
    
    /* Deeply nested scopes with same variable names */
    {
        int x = 2;  /* Shadows outer x */
        volatile int y = x;
        
        {
            extern int x;  /* External declaration */
            volatile int z = x;
            
            {
                int x = 3;  /* Another shadow */
                volatile int w = x;
                
                {
                    auto int x = 4;  /* C++11 auto */
                    volatile int v = x;
                    
                    {
                        register int x = 5;  /* Register storage */
                        volatile int u = x;
                        
                        global_sink += x;
                    }
                }
            }
        }
    }
    
    /* Function scope with parameters */
    auto func = [](int x) -> int {
        {
            int x = x * 2;  /* Parameter shadowing */
            {
                volatile int y = x;
                return y;
            }
        }
    };
    
    /* More complex nesting with loops */
    for (int i = 0; i < 10; i++) {
        int x = i;  /* New x in loop scope */
        {
            int x = x + 1;  /* Uses previous x */
            volatile int tmp = x;
            
            for (int j = 0; j < 5; j++) {
                int x = j * 2;  /* Another x in inner loop */
                volatile int tmp2 = x;
                
                if (j == 2) {
                    int x = 99;  /* x in if block */
                    volatile int tmp3 = x;
                    global_sink += x;
                }
            }
        }
    }
}

int main(void) {
    int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* 1. Generate IDENTIFIER_NODE cases */
    printf("Testing IDENTIFIER_NODE generation...\n");
    identifier_generator();
    checksum += global_sink;
    
    /* 2. Generate TREE_VEC nodes */
    printf("Testing TREE_VEC generation...\n");
    vector_generator();
    checksum += global_sink;
    
    /* 3. Generate CONSTRUCTOR nodes */
    printf("Testing CONSTRUCTOR generation...\n");
    constructor_generator();
    checksum += global_sink;
    
    /* 4. Generate SSA_NAME nodes */
    printf("Testing SSA_NAME generation...\n");
    checksum += ssa_generator(100);
    
    /* 5. Generate BLOCK nodes */
    printf("Testing BLOCK generation...\n");
    block_generator();
    checksum += global_sink;
    
    /* 6. Generate OMP_CLAUSE nodes */
    printf("Testing OMP_CLAUSE generation...\n");
    omp_generator(1000);
    checksum += global_sink;
    
#ifdef __cplusplus
    /* 7. Generate TREE_BINFO nodes (C++ only) */
    printf("Testing TREE_BINFO generation...\n");
    use_hierarchy();
    checksum += global_sink;
#endif
    
    /* Call external functions to create unresolved identifiers */
    checksum += external_func1();
    external_func2(checksum);
    checksum += (int)external_func3(checksum);
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
