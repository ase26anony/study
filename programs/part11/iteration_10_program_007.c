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

void use_hierarchy(Base *b, Derived *d, DeepDerived *dd) {
    b->a = 1;
    d->a = 2;
    d->b = 3;
    dd->a = 4;
    dd->b = 5;
    dd->c = 6;
}
#endif

/* Function to generate SSA_NAME nodes */
int complex_control_flow(int n) {
    int i, s = 0, t = 1;
    
    /* Complex loop with multiple branches */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t += 2;
        } else {
            s -= i / (t + 1);
            t *= 3;
        }
        
        switch (i % 4) {
            case 0: s += 1; break;
            case 1: s += t; break;
            case 2: s -= t; break;
            case 3: s *= 2; break;
        }
    }
    
    /* Nested loops */
    for (i = 0; i < 5; i++) {
        for (int j = 0; j < 3; j++) {
            s += i * j;
            if (j == 1) break;
        }
    }
    
    return s;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
int nested_scopes_test(void) {
    int result = 0;
    
    /* Level 1 */
    {
        int x = 1;
        result += x;
        
        /* Level 2 */
        {
            /* Different x in inner scope */
            float x = 2.5f;
            result += (int)x;
            
            /* Level 3 */
            {
                /* Another x */
                volatile int x = 3;
                result += x;
                
                /* Level 4 - extern declaration */
                {
                    extern int x;  /* Unresolved identifier */
                    volatile int y = x + 1;
                    result += y;
                }
            }
        }
        
        /* Another sibling scope */
        {
            /* Yet another x */
            long x = 100;
            result += (int)x;
        }
    }
    
    /* Function scope with same name */
    {
        char x = 'A';
        result += x;
    }
    
    return result;
}

/* Function to generate BLOCK nodes with labels and gotos */
int block_and_label_test(void) {
    int result = 0;
    
    /* Outer block */
    {
        int a = 0;
        
    block1:
        a++;
        result += a;
        
        /* Inner block 1 */
        {
            int b = 10;
            result += b;
            
            if (a < 3)
                goto block2;
        }
        
        /* Inner block 2 */
        {
            int c = 20;
        block2:
            c += a;
            result += c;
            
            if (a < 5)
                goto block1;
        }
    }
    
    /* Another block with computed goto */
    {
        void *labels[] = { &&label1, &&label2, &&label3 };
        int i = 0;
        
    label_start:
        i++;
        goto *labels[i % 3];
        
    label1:
        result += 1;
        if (i < 10) goto label_start;
        goto label_end;
        
    label2:
        result += 2;
        if (i < 10) goto label_start;
        goto label_end;
        
    label3:
        result += 3;
        if (i < 10) goto label_start;
        
    label_end:
        result += 100;
    }
    
    return result;
}

/* Function with CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    struct {
        int x;
        int y;
    } nested;
    int arr[5];
    union {
        int i;
        float f;
    } u;
};

struct ComplexStruct constructor_test(void) {
    /* Various initializers creating CONSTRUCTOR nodes */
    
    /* Designated initializer with nested struct */
    struct ComplexStruct s1 = {
        .a = 1,
        .nested = { .x = 2, .y = 3 },
        .arr = { [0] = 10, [2] = 20, [4] = 30 },
        .u = { .f = 3.14f }
    };
    
    /* Partial array initializer */
    int partial_arr[10] = { [1] = 100, [5] = 500, [9] = 900 };
    sink(&partial_arr);
    
    /* Nested designated initializers */
    struct {
        struct {
            int a;
            int b;
        } inner;
        int c;
    } nested = { .inner = { .a = 1, .b = 2 }, .c = 3 };
    sink(&nested);
    
    /* Union with designated initializer */
    union {
        struct {
            int a;
            int b;
        } s;
        long long ll;
    } u = { .s = { .a = 1, .b = 2 } };
    sink(&u);
    
    return s1;
}

/* Vector operations for TREE_VEC */
#ifdef __GNUC__
void vector_operations(void) {
    /* Vector types using GCC extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    /* Vector initializations */
    v4si a = { 1, 2, 3, 4 };
    v4si b = { 5, 6, 7, 8 };
    v4si c = a + b;
    v4si d = a * b;
    v4si e = a & b;
    
    /* Mixed operations */
    v4sf f = { 1.0f, 2.0f, 3.0f, 4.0f };
    v4sf g = f * 2.0f;
    
    /* Array compound literals */
    int *p = (int[]){ 10, 20, 30, 40 };
    int *q = (int[4]){ [0] = 100, [2] = 300 };
    
    /* Vector comparisons */
    v4si mask = a > b;
    v4si selected = __builtin_shuffle(a, b, (v4si){ 0, 4, 1, 5 });
    
    sink(&c);
    sink(&d);
    sink(&e);
    sink(&g);
    sink(&p);
    sink(&q);
    sink(&mask);
    sink(&selected);
    
    /* More complex vector expressions */
    v8hi h1 = { 1, 2, 3, 4, 5, 6, 7, 8 };
    v8hi h2 = { 8, 7, 6, 5, 4, 3, 2, 1 };
    v8hi h3 = h1 + h2;
    v8hi h4 = h1 * h2;
    
    sink(&h3);
    sink(&h4);
}
#endif

/* OpenMP section for OMP_CLAUSE nodes */
#ifdef _OPENMP
void openmp_test(int n) {
    int i;
    long sum = 0;
    long product = 1;
    int private_var = 0;
    
    /* Multiple OpenMP pragmas with various clauses */
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel private(i) shared(sum, n) firstprivate(private_var) \
                         reduction(+:sum) if(n > 1000)
    {
        #pragma omp for schedule(dynamic, 2) nowait
        for (i = 0; i < n; i++) {
            sum += i;
        }
        
        /* Nested parallel for with collapse */
        #pragma omp for collapse(2) ordered
        for (i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                #pragma omp ordered
                sum += i * j;
            }
        }
    }
    
    /* Single construct with copyprivate */
    #pragma omp parallel
    {
        int local_val = 0;
        
        #pragma omp single copyprivate(local_val)
        {
            local_val = 42;
        }
        
        #pragma omp atomic
        sum += local_val;
    }
    
    /* Sections with different clauses */
    #pragma omp parallel sections private(i) reduction(*:product) \
                                   num_threads(4)
    {
        #pragma omp section
        {
            for (i = 0; i < 100; i++) {
                product *= 2;
            }
        }
        
        #pragma omp section
        {
            for (i = 0; i < 100; i++) {
                product *= 3;
            }
        }
    }
    
    /* Task construct */
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (i = 0; i < 10; i++) {
                #pragma omp task depend(inout: sum) priority(i)
                {
                    #pragma omp atomic
                    sum += i;
                }
            }
        }
    }
    
    sink(&sum);
    sink(&product);
}
#endif

int main(void) {
    int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* 1. Test IDENTIFIER_NODE generation */
    printf("Testing IDENTIFIER_NODE...\n");
    checksum += nested_scopes_test();
    
    /* 2. Test SSA_NAME generation */
    printf("Testing SSA_NAME...\n");
    checksum += complex_control_flow(20);
    
    /* 3. Test BLOCK nodes */
    printf("Testing BLOCK nodes...\n");
    checksum += block_and_label_test();
    
    /* 4. Test CONSTRUCTOR nodes */
    printf("Testing CONSTRUCTOR nodes...\n");
    struct ComplexStruct cs = constructor_test();
    checksum += cs.a + cs.nested.x + cs.nested.y;
    
    /* 5. Test TREE_VEC nodes */
    #ifdef __GNUC__
    printf("Testing TREE_VEC nodes...\n");
    vector_operations();
    checksum += 1000;  /* Mark that vectors were tested */
    #endif
    
    /* 6. Test OpenMP clauses */
    #ifdef _OPENMP
    printf("Testing OMP_CLAUSE nodes...\n");
    openmp_test(100);
    checksum += 2000;  /* Mark that OpenMP was tested */
    #endif
    
    /* 7. Call external functions for unresolved identifiers */
    printf("Testing unresolved identifiers...\n");
    checksum += external_func(checksum);
    
    /* 8. Use global variable */
    checksum += global_var;
    
    /* 9. C++ specific tests */
    #ifdef __cplusplus
    printf("Testing C++ BINFO nodes...\n");
    Derived d;
    DeepDerived dd;
    use_hierarchy(&d, &d, &dd);
    checksum += 3000 + d.a + d.b + dd.c;
    #endif
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
