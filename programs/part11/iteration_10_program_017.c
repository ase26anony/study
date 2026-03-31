/* test_tree.c - Comprehensive tree node coverage test */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func(void);
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
    void vfunc() override {}
};

void use_hierarchy(Derived *d, DeepDerived *dd) {
    d->a = 1;        /* Accesses through inheritance */
    dd->b = 2;       /* Accesses through multiple inheritance */
    Base *b = d;     /* Upcast */
    b->vfunc();      /* Virtual call */
}
#endif

/* Function to generate SSA_NAME nodes */
int ssa_test(int n) {
    int i, s = 0, t = 1, u = 2;
    
    /* Complex control flow for SSA */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t = u ^ s;
        } else {
            s *= 2 + u;
            u = t - i;
        }
        
        switch (i % 4) {
            case 0: s += 1; break;
            case 1: t += s; break;
            case 2: u ^= t; break;
            case 3: s = u * t; break;
        }
    }
    
    /* More SSA complexity */
    int x = s, y = t, z = u;
    while (x > 0) {
        y = (x & 1) ? y + z : y - z;
        z = (y % 2) ? z * 2 : z / 2;
        x >>= 1;
    }
    
    return s + t + u + x + y + z;
}

/* Function with deep nesting for BLOCK nodes */
int block_test(int val) {
    int result = 0;
    
    /* Level 1 block */
    {
        int a = val;
        result += a;
        
    level1_label:
        if (a > 0) {
            /* Level 2 block */
            {
                int b = a * 2;
                result += b;
                
                if (b % 3 == 0) {
                    /* Level 3 block */
                    {
                        int c = b / 3;
                        volatile int prevent_merge = c;
                        result += c;
                        goto level2_label;  /* Jump between blocks */
                    }
                }
                
            level2_label:
                result *= 2;
            }
            
            a--;
            goto level1_label;
        }
    }
    
    /* Another block with switch */
    {
        int x = result % 10;
        
        switch (x) {
            case 0: {
                int inner = x * 2;
                result += inner;
                break;
            }
            case 1: {
                int inner = x + 5;
                result -= inner;
                break;
            }
            default: {
                int inner = x * x;
                result ^= inner;
                break;
            }
        }
    }
    
    return result;
}

/* Function for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    struct {
        int x;
        int y;
        int z;
    } nested;
    union {
        int u_int;
        float u_float;
    } data;
    int arr[4];
};

union MixedUnion {
    int i;
    float f;
    struct {
        char a, b, c, d;
    } bytes;
};

void constructor_test(void) {
    /* Designated initializers with nesting */
    struct ComplexStruct cs = {
        .a = 42,
        .nested = { .x = 1, .y = 2, .z = 3 },
        .data = { .u_float = 3.14f },
        .arr = { [0] = 10, [2] = 20, [3] = 30 }
    };
    
    /* Partial array initialization */
    int sparse_array[10] = { [2] = 100, [5] = 200, [9] = 300 };
    
    /* Nested designated initializers */
    struct {
        int a;
        struct {
            int b[3];
            int c;
        } inner;
    } deep = { .a = 1, .inner = { .b = {[1] = 5}, .c = 6 } };
    
    /* Union initializer */
    union MixedUnion mu = { .bytes = { .a = 'A', .c = 'C' } };
    
    /* Compound literal as initializer */
    int *ptr = (int[]){1, 2, 3, 4, 5};
    
    sink(&cs);
    sink(sparse_array);
    sink(&deep);
    sink(&mu);
    sink(ptr);
}

/* Function for TREE_VEC nodes */
void vector_test(void) {
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c > d;
    
    v4sf f = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf g = f * 2.0f;
    
    /* Array compound literals */
    int *p1 = (int[]){1, 2, 3};
    int *p2 = (int[]){4, 5, 6, 7};
    
    /* Vector operations */
    v4si h = {0};
    for (int i = 0; i < 4; i++) {
        h[i] = a[i] + b[i] * c[i];
    }
    
    sink(&c);
    sink(&g);
    sink(p1);
    sink(p2);
    sink(&h);
}

/* OpenMP section for OMP_CLAUSE nodes */
void omp_test(int *array, int n) {
    int i, sum = 0, product = 1;
    volatile int sink_var;
    
    #pragma omp parallel for private(i) shared(array, sum) \
        reduction(+:sum) schedule(dynamic, 2) if(n > 1000)
    for (i = 0; i < n; i++) {
        sum += array[i];
    }
    
    #pragma omp parallel sections private(i) \
        firstprivate(product) lastprivate(sink_var) \
        num_threads(4)
    {
        #pragma omp section
        {
            for (i = 0; i < n/2; i++) {
                product *= array[i];
            }
        }
        
        #pragma omp section
        {
            sink_var = product;
            #pragma omp critical
            {
                sum += product;
            }
        }
    }
    
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            sink_var = sum;
        }
        
        #pragma omp barrier
        
        #pragma omp for collapse(2) ordered
        for (int x = 0; x < 10; x++) {
            for (int y = 0; y < 10; y++) {
                #pragma omp ordered
                {
                    array[x * 10 + y] = x + y;
                }
            }
        }
    }
    
    sink(&sum);
    sink(&product);
}

/* Main test driver */
int main(void) {
    int checksum = 0;
    volatile int sink_val;
    
    /* 1. IDENTIFIER_NODE tests - deeply nested scopes with same names */
    printf("Testing IDENTIFIER_NODE...\n");
    {
        int x = 1;
        checksum += x;
        
        {
            extern int x;  /* Different declaration */
            volatile int y = x + 1;
            checksum += y;
        }
        
        {
            float x = 3.14f;  /* Different type, same name */
            checksum += (int)x;
            
            {
                double x = 2.71828;  /* Another scope level */
                volatile int z = (int)x;
                checksum += z;
            }
        }
    }
    
    /* Call external to create unresolved identifiers */
    checksum += external_func();
    sink_val = external_var;
    
    /* 2. SSA_NAME tests */
    printf("Testing SSA_NAME...\n");
    checksum += ssa_test(50);
    
    /* 3. BLOCK tests */
    printf("Testing BLOCK...\n");
    checksum += block_test(25);
    
    /* 4. CONSTRUCTOR tests */
    printf("Testing CONSTRUCTOR...\n");
    constructor_test();
    checksum += 1234;  /* Arbitrary value since constructor_test uses sink */
    
    /* 5. TREE_VEC tests */
    printf("Testing TREE_VEC...\n");
    vector_test();
    checksum += 5678;
    
    /* 6. OpenMP tests */
    printf("Testing OMP_CLAUSE...\n");
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i + 1;
    }
    omp_test(array, 100);
    for (int i = 0; i < 100; i++) {
        checksum += array[i];
    }
    
#ifdef __cplusplus
    /* 7. BINFO tests (C++ only) */
    printf("Testing TREE_BINFO...\n");
    Derived d;
    DeepDerived dd;
    use_hierarchy(&d, &dd);
    checksum += d.a + dd.b;
#endif
    
    printf("Final checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
