/* test_tree.c - Comprehensive tree node coverage test */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func(int);
extern volatile int global_counter;

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

void use_hierarchy(Base *b, Derived *d, DeepDerived *dd) {
    b->a = 1;
    d->b = 2;
    dd->c = 3;
    d->a = 4;  /* Access base member through derived pointer */
}
#endif

/* Function to generate SSA_NAME nodes */
int ssa_test(int n) {
    int i, j, k = 0;
    volatile int result = 0;
    
    /* Complex control flow for SSA */
    for (i = 0; i < n; i++) {
        if (i % 3 == 0) {
            j = i * 2;
            k += j;
        } else if (i % 3 == 1) {
            j = i / 2;
            k -= j;
        } else {
            j = i + 1;
            k *= (j > 0) ? j : 1;
        }
        
        /* Nested loop with phi nodes */
        for (int m = 0; m < 5; m++) {
            if (m % 2 == 0) {
                k += m;
            } else {
                k -= m;
            }
        }
    }
    
    /* Switch with multiple assignments to same variable */
    switch (n % 4) {
        case 0: k = k * 2; break;
        case 1: k = k + 100; break;
        case 2: k = k - 50; break;
        case 3: k = k / 2; break;
    }
    
    result = k;
    return result;
}

/* Function with complex block structure */
int block_test(void) {
    volatile int checksum = 0;
    
    /* Level 1 block */
    {
        int a = 1;
        checksum += a;
        
    level2:
        {
            int b = 2;
            checksum += b;
            
            /* Level 3 block with label */
            {
                int c = 3;
                checksum += c;
                goto level4;
            }
        }
        
        /* Unreachable but creates block structure */
        a = 99;
    }
    
level4:
    {
        int d = 4;
        checksum += d;
        
        /* Jump back */
        goto level2;
    }
    
    /* Never reached but creates control flow */
    return checksum;
}

int main(void) {
    volatile int checksum = 0;
    
    /* ========== IDENTIFIER_NODE tests ========== */
    printf("Testing IDENTIFIER_NODE...\n");
    
    /* Deeply nested scopes with same variable names */
    {
        int x = 1;
        checksum += x;
        
        {
            /* Different type, same name */
            volatile int x = 2;
            checksum += x;
            
            {
                /* Pointer with same name */
                int *x = (int[]){1, 2, 3};
                checksum += x[0];
                
                {
                    /* Array with same name */
                    int x[3] = {4, 5, 6};
                    checksum += x[1];
                    
                    {
                        /* Function pointer with same name */
                        int (*x)(int) = &external_func;
                        /* Call through function pointer */
                        checksum += 7;
                    }
                }
            }
        }
    }
    
    /* More identifier shadowing */
    {
        volatile long counter = 0;
        
        for (int i = 0; i < 3; i++) {
            /* 'i' shadows outer 'i' in nested loop */
            for (int i = 0; i < 2; i++) {
                counter++;
            }
            
            {
                /* Another 'i' in inner block */
                float i = 3.14f;
                counter += (int)i;
            }
        }
        
        checksum += (int)counter;
    }
    
    /* ========== TREE_VEC tests ========== */
    printf("Testing TREE_VEC...\n");
    
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = vec_a + vec_b;
    v4si vec_d = vec_a * vec_b;
    
    /* Vector operations */
    vec_c = vec_c + vec_d;
    vec_d = vec_a & vec_b;
    
    /* Store to volatile to prevent optimization */
    volatile v4si sink_vec = vec_c;
    checksum += sink_vec[0];
    
    /* Float vectors */
    v4sf fvec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec_b = fvec_a * 2.0f;
    volatile v4sf sink_fvec = fvec_b;
    checksum += (int)sink_fvec[0];
    
    /* Array compound literals */
    int *p1 = (int[]){1, 2, 3, 4, 5};
    int *p2 = (int[]){6, 7, 8, 9, 10};
    checksum += p1[0] + p2[0];
    
    /* Nested compound literals */
    struct Point { int x; int y; };
    struct Point *points = (struct Point[]){{1, 2}, {3, 4}, {5, 6}};
    checksum += points[0].x;
    
    /* ========== CONSTRUCTOR tests ========== */
    printf("Testing CONSTRUCTOR...\n");
    
    /* Designated initializers */
    struct Complex {
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
    
    /* Partial and nested initialization */
    struct Complex c1 = {
        .a = 1,
        .b = {[0] = 10, [2] = 30},
        .nested = {.x = 100, .y = 200},
        .u = {.u2 = 3.14f}
    };
    
    checksum += c1.a + c1.b[0] + c1.nested.x;
    
    /* Array with designated initializers */
    int arr[10] = {[0] = 1, [5] = 2, [9] = 3};
    checksum += arr[5];
    
    /* Union initialization */
    union Data {
        int i;
        float f;
        char str[20];
    } data = {.f = 2.718f};
    
    checksum += (int)data.f;
    
    /* Nested struct with partial init */
    struct Outer {
        int id;
        struct Inner {
            int a;
            int b;
            int c;
        } inner;
        int tail;
    } outer = {
        .id = 999,
        .inner = {.a = 111, .c = 333},
        .tail = 888
    };
    
    checksum += outer.inner.a;
    
    /* ========== BLOCK tests ========== */
    printf("Testing BLOCK...\n");
    
    /* Complex block structure with labels */
    {
        int val = 0;
        
    start_block:
        {
            int a = 1;
            val += a;
            goto middle_block;
        }
        
        {
            int b = 2;  /* Unreachable but creates block */
            val += b;
        }
        
    middle_block:
        {
            int c = 3;
            val += c;
            
            {
                int d = 4;
                val += d;
                goto end_block;
            }
            
            {
                int e = 5;  /* Unreachable */
                val += e;
            }
        }
        
    end_block:
        {
            int f = 6;
            val += f;
            checksum += val;
        }
    }
    
    /* ========== SSA_NAME tests ========== */
    printf("Testing SSA_NAME...\n");
    
    checksum += ssa_test(20);
    
    /* Additional SSA patterns */
    {
        int x = 0, y = 0, z = 0;
        
        /* Loop with multiple induction variables */
        for (int i = 0; i < 10; i++) {
            if (i % 2 == 0) {
                x += i;
                y = x * 2;
            } else {
                x -= i;
                y = x / 2;
            }
            
            z += y;
            
            /* Nested condition */
            if (z > 50) {
                x = 0;
                y = 0;
            }
        }
        
        checksum += z;
    }
    
    /* ========== OpenMP tests ========== */
    printf("Testing OMP_CLAUSE...\n");
    
    /* Multiple OpenMP pragmas with various clauses */
    int omp_sum = 0;
    int omp_array[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        omp_array[i] = i;
    }
    
    /* Parallel for with multiple clauses */
    #pragma omp parallel for private(omp_sum) shared(omp_array) \
        reduction(+:checksum) schedule(dynamic, 5) \
        if(omp_get_max_threads() > 1)
    for (int i = 0; i < 100; i++) {
        int local_sum = omp_array[i];
        checksum += local_sum;
    }
    
    /* Parallel sections */
    #pragma omp parallel sections private(omp_sum) \
        firstprivate(omp_array) num_threads(2)
    {
        #pragma omp section
        {
            omp_sum = 0;
            for (int i = 0; i < 50; i++) {
                omp_sum += omp_array[i];
            }
            checksum += omp_sum;
        }
        
        #pragma omp section
        {
            omp_sum = 0;
            for (int i = 50; i < 100; i++) {
                omp_sum += omp_array[i];
            }
            checksum += omp_sum;
        }
    }
    
    /* Single construct with copyprivate */
    int single_var = 0;
    #pragma omp parallel private(single_var)
    {
        #pragma omp single copyprivate(single_var)
        {
            single_var = 42;
        }
        checksum += single_var;
    }
    
    /* ========== C++ BINFO tests ========== */
#ifdef __cplusplus
    printf("Testing BINFO (C++ only)...\n");
    
    Derived d;
    DeepDerived dd;
    use_hierarchy(&d, &d, &dd);
    
    /* Multiple inheritance-like access */
    Base *bp = &d;
    bp->a = 100;
    checksum += d.a;
    
    /* Virtual function call */
    d.vfunc();
    dd.vfunc();
#endif
    
    /* ========== Final checksum ========== */
    printf("Final checksum: %d\n", checksum);
    
    /* Prevent tail-call optimization */
    volatile int final_result = checksum;
    return final_result;
}

/* Additional function to create more SSA opportunities */
int unused_ssa_function(int param) {
    int x = param;
    int y = 0;
    
    /* Complex phi node creation */
    for (int i = 0; i < param; i++) {
        if (x > 0) {
            y += x;
            x--;
        } else {
            y -= x;
            x++;
        }
    }
    
    /* Switch with breaks and continues in loop */
    while (y > 0) {
        switch (y % 4) {
            case 0:
                x += 10;
                break;
            case 1:
                x -= 5;
                continue;  /* Skip to next iteration */
            case 2:
                x *= 2;
                break;
            default:
                x /= 2;
                break;
        }
        y--;
    }
    
    return x;
}
