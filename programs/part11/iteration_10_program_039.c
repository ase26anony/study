/* test_tree.c - Comprehensive tree node coverage test */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(int);
extern void external_func2(void);
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

void use_hierarchy() {
    DeepDerived dd;
    dd.a = 1;      /* Accesses through inheritance chain */
    dd.b = 2;
    dd.c = 3;
    Base* bp = &dd;
    bp->vfunc();   /* Virtual call */
    checksum += dd.a + dd.b + dd.c;
}
#endif

/* Complex control flow for SSA_NAME generation */
int ssa_test(int n) {
    int i, s = 0, t = 1, u = 0;
    
    /* Multiple assignments in loops create SSA names */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t = s % 7;  /* Reassignment creates new SSA name */
        } else {
            u = s + t;
            s = u * 2;  /* Another SSA name */
        }
        
        /* Nested condition with reassignment */
        if (s > 100) {
            t = s / 3;
        } else if (s < 50) {
            t = s * 2;
        } else {
            t = s + 5;
        }
    }
    
    /* Switch with reassignments */
    switch (n % 4) {
        case 0: s = t + 1; break;
        case 1: s = t * 2; break;
        case 2: s = t - 3; break;
        case 3: s = t / 2; break;
    }
    
    return s + t + u;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
void identifier_test(void) {
    /* Level 1 scope */
    int x = 1;
    checksum += x;
    
    {
        /* Level 2 scope - different 'x' */
        volatile int x = 2;
        checksum += x;
        
        {
            /* Level 3 scope - yet another 'x' */
            float x = 3.0f;
            checksum += (int)x;
            
            {
                /* Level 4 scope - pointer x */
                int* x = &checksum;
                *x += 4;
                
                {
                    /* Level 5 scope - array x */
                    int x[2] = {5, 6};
                    checksum += x[0] + x[1];
                    
                    /* Reference external x */
                    extern volatile int external_x;
                    checksum += 7;
                }
            }
        }
    }
    
    /* Function scope x again */
    x = 8;
    checksum += x;
    
    /* Multiple variables with same name in different blocks */
    for (int i = 0; i < 2; i++) {
        volatile int var = i * 10;
        checksum += var;
        
        {
            volatile int var = i * 20 + 1;
            checksum += var;
        }
    }
}

/* Vector operations for TREE_VEC */
void vector_test(void) {
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    /* Compound literal array initializer */
    int* p = (int[]){10, 20, 30, 40};
    
    /* More complex vector operations */
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf f3 = f1 * f2;
    
    /* Use results to prevent elimination */
    sink = c[0] + d[1];
    checksum += p[0] + (int)f3[0];
    
    /* Another TREE_VEC example with initialization */
    struct Point { int x; int y; int z; };
    struct Point points[3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    checksum += points[1].y;
}

/* Constructor nodes with designated initializers */
void constructor_test(void) {
    /* Nested struct with designated initializers */
    struct Inner {
        int a;
        int b[3];
    };
    
    struct Outer {
        int x;
        struct Inner inner;
        float f;
        int arr[4];
    };
    
    /* Partial and nested designated initialization */
    struct Outer o1 = {
        .x = 100,
        .inner = {
            .a = 10,
            .b = {[1] = 20, [2] = 30}  /* Partial array init */
        },
        .f = 3.14f,
        .arr = {1, 2}  /* Partial init */
    };
    
    checksum += o1.x + o1.inner.b[1];
    
    /* Union with designated initializer */
    union Data {
        int i;
        float f;
        char str[4];
    };
    
    union Data d1 = { .i = 42 };
    union Data d2 = { .f = 2.718f };
    union Data d3 = { .str = "ABC" };
    
    checksum += d1.i + (int)d2.f + d3.str[0];
    
    /* Array of structs with designated init */
    struct Inner arr[2] = {
        [0] = { .a = 1, .b = {2, 3} },
        [1] = { .a = 4, .b = {[2] = 5} }
    };
    
    checksum += arr[1].b[2];
}

/* Complex blocks with labels for BLOCK nodes */
void block_test(void) {
    int a = 0;
    
    /* Outer block with label */
    outer_block:
    {
        int b = 1;
        checksum += b;
        
        /* Inner block 1 */
        {
            int c = 2;
            checksum += c;
            
            if (a < 5) {
                goto middle_block;  /* Jump forward */
            }
        }
        
        /* Unreachable code creates separate blocks */
        int d = 3;
        checksum += d;
    }
    
    middle_block:
    {
        int e = 4;
        checksum += e;
        
        /* Nested block with its own label */
        {
            deep_label:
            int f = 5;
            checksum += f;
            
            if (a++ < 3) {
                goto outer_block;  /* Jump backward */
            }
        }
        
        /* Another block after goto target */
        {
            int g = 6;
            checksum += g;
            
            if (checksum > 100) {
                goto deep_label;
            }
        }
    }
    
    /* Switch creates multiple blocks */
    switch (a) {
        case 0: {
            int h = 7;
            checksum += h;
            break;
        }
        case 1: {
            int i = 8;
            checksum += i;
            break;
        }
        default: {
            int j = 9;
            checksum += j;
            break;
        }
    }
}

/* OpenMP pragmas for OMP_CLAUSE nodes */
void omp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    static int shared_var = 0;
    
    /* Multiple clauses in single pragma */
    #pragma omp parallel for private(i) shared(shared_var) reduction(+:sum) \
                schedule(dynamic, 2) num_threads(2) if(n > 1000)
    for (i = 0; i < n; i++) {
        sum += i;
        shared_var++;
    }
    
    checksum += sum + shared_var;
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel private(private_var) firstprivate(n) \
                copyin(shared_var) default(shared)
    {
        private_var = omp_get_thread_num();
        #pragma omp atomic
        shared_var += private_var;
    }
    
    /* Sections with nowait */
    #pragma omp parallel sections private(i) lastprivate(sum)
    {
        #pragma omp section
        {
            sum = 1;
            for (i = 0; i < 10; i++) {
                #pragma omp atomic
                checksum++;
            }
        }
        
        #pragma omp section
        {
            sum = 2;
            #pragma omp critical
            {
                checksum += 10;
            }
        }
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
    
    /* Test 3: SSA_NAME (with optimization) */
    printf("Testing SSA_NAME...\n");
    checksum += ssa_test(50);
    
    /* Test 4: CONSTRUCTOR */
    printf("Testing CONSTRUCTOR...\n");
    constructor_test();
    
    /* Test 5: BLOCK */
    printf("Testing BLOCK...\n");
    block_test();
    
    /* Test 6: OMP_CLAUSE */
    printf("Testing OMP_CLAUSE...\n");
    omp_test(100);
    
    #ifdef __cplusplus
    /* Test 7: TREE_BINFO (C++ only) */
    printf("Testing TREE_BINFO...\n");
    use_hierarchy();
    #endif
    
    /* Final checksum */
    printf("Final checksum: %d\n", checksum);
    
    /* Use external identifiers */
    if (external_var > 0) {
        checksum = external_func1(checksum);
    }
    
    return checksum % 256;
}

/* Dummy function to satisfy external references */
#ifdef __cplusplus
extern "C" {
#endif

volatile int external_x = 0;

int omp_get_thread_num(void) {
    return 0;
}

#ifdef __cplusplus
}
#endif
