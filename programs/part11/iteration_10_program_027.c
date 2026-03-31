/* test_tree.c - Comprehensive test for GCC tree node coverage */
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
    virtual void method() {}
};

struct Derived : Base {
    int b;
    void method() override {}
};

struct DeepDerived : Derived {
    int c;
};

void use_inheritance() {
    DeepDerived dd;
    dd.a = 1;      /* Accesses through inheritance hierarchy */
    dd.b = 2;
    dd.c = 3;
    dd.method();   /* Virtual call */
    
    Base* bp = &dd;
    bp->method();  /* Polymorphic call */
}
#endif

/* Function to create SSA_NAME nodes with complex control flow */
int create_ssa_names(int n) {
    int i, j, k = 0;
    int result = 0;
    
    /* Complex loop with multiple branches */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            for (j = 0; j < i; j++) {
                if (j % 3 == 0) {
                    k += j * 2;
                } else if (j % 3 == 1) {
                    k -= j;
                } else {
                    k = k * 3 + 1;
                }
                
                /* Switch inside nested loops */
                switch (j % 4) {
                    case 0: result += k; break;
                    case 1: result -= k; break;
                    case 2: result ^= k; break;
                    case 3: result |= k; break;
                }
            }
        } else {
            int temp = i * 2;
            while (temp > 0) {
                result += temp;
                temp /= 2;
                
                /* Early exit with goto */
                if (temp < 10) goto early_exit;
            }
        early_exit:
            result += i;
        }
    }
    
    /* Phi nodes from multiple paths */
    return result + k;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE creation */
void create_identifiers(void) {
    int x = 1;
    volatile int sink1 = x;
    
    /* Nested block 1 */
    {
        int x = 2;  /* Same name, different scope */
        volatile int sink2 = x;
        
        /* Nested block 2 */
        {
            extern int x;  /* External declaration */
            volatile int sink3 = x;
            
            /* Nested block 3 */
            {
                int x = 4;
                volatile int sink4 = x;
                
                /* Function scope */
                auto int func() {
                    int x = 5;
                    volatile int sink5 = x;
                    return sink5;
                }
                sink4 += func();
            }
        }
    }
    
    /* Loop scopes */
    for (int i = 0; i < 3; i++) {
        int x = i + 10;
        volatile int sink6 = x;
        
        for (int j = 0; j < 2; j++) {
            int x = j + 20;
            volatile int sink7 = x;
        }
    }
    
    /* Switch with scopes */
    switch (x) {
        case 1: {
            int x = 100;
            volatile int sink8 = x;
            break;
        }
        case 2: {
            int x = 200;
            volatile int sink9 = x;
            break;
        }
    }
}

/* Function to create TREE_VEC nodes using GCC extensions */
void create_tree_vec(void) {
    /* Vector types using GCC extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef double v2df __attribute__((vector_size(16)));
    
    /* Vector initializations */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = a & b;
    
    /* Vector operations */
    v4sf f = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf g = f * 2.0f;
    
    v2df h = {1.0, 2.0};
    v2df i = {3.0, 4.0};
    v2df j = h + i;
    
    /* Array compound literals */
    int *p1 = (int[]){1, 2, 3, 4, 5};
    int *p2 = (int[]){6, 7, 8, 9, 10};
    
    /* Nested compound literals */
    struct Point { int x; int y; };
    struct Point *points = (struct Point[]){
        {.x = 1, .y = 2},
        {.x = 3, .y = 4},
        {.x = 5, .y = 6}
    };
    
    /* Multi-dimensional compound literal */
    int (*matrix)[3] = (int[][3]){
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    
    /* Use vectors to prevent dead code elimination */
    volatile v4si sink_vec = c + d + e;
    volatile v4sf sink_vecf = g;
    volatile v2df sink_vecd = j;
}

/* Function with complex blocks for BLOCK node creation */
void create_blocks(void) {
    int a = 0;
    
    /* Label and goto for block structure */
    block1: {
        int b = 1;
        volatile int sink1 = b;
        goto block3;
    }
    
    block2: {
        int c = 2;
        volatile int sink2 = c;
        goto block4;
    }
    
    block3: {
        int d = 3;
        volatile int sink3 = d;
        
        /* Nested block with label */
        {
            int e = 4;
        inner_block:
            volatile int sink4 = e;
            goto block2;
        }
    }
    
    block4: {
        int f = 5;
        volatile int sink5 = f;
        
        /* Switch with blocks and labels */
        switch (f) {
            case 5: {
                int g = 6;
            case_label:
                volatile int sink6 = g;
                break;
            }
            default: {
                int h = 7;
                goto case_label;
            }
        }
    }
    
    /* Deeply nested blocks */
    {
        int i = 8;
        {
            int j = 9;
            {
                int k = 10;
                {
                    int l = 11;
                    volatile int sink7 = i + j + k + l;
                }
            }
        }
    }
}

/* Function to create CONSTRUCTOR nodes */
void create_constructors(void) {
    /* Struct with designated initializers */
    struct S {
        int a;
        int b;
        int c[3];
        struct {
            int x;
            int y;
        } nested;
    };
    
    /* Various initializations */
    struct S s1 = { .a = 1, .b = 2, .c = {3, 4, 5}, .nested = {.x = 6, .y = 7} };
    struct S s2 = { .a = 8, .c = {[1] = 9, [2] = 10} };  /* Partial */
    struct S s3 = { .b = 11, .nested.x = 12 };           /* Mixed */
    
    /* Array with designated initializers */
    int arr1[10] = { [0] = 1, [5] = 2, [9] = 3 };
    int arr2[5] = { [1 ... 3] = 4 };
    
    /* Nested designated initializers */
    struct Outer {
        struct Inner {
            int a;
            int b;
        } inner;
        int c;
    };
    
    struct Outer outer = { .inner = {.a = 1, .b = 2}, .c = 3 };
    struct Outer outer2 = { .inner.a = 4, .c = 5 };
    
    /* Union initializers */
    union U {
        int i;
        float f;
        double d;
    };
    
    union U u1 = { .i = 42 };
    union U u2 = { .f = 3.14f };
    union U u3 = { .d = 2.71828 };
    
    /* Complex nested initializer */
    struct Complex {
        int a;
        struct {
            int b[2];
            struct {
                int c;
                int d;
            } inner;
        } mid;
        int e;
    };
    
    struct Complex comp = { 
        .a = 1, 
        .mid = { 
            .b = {[0] = 2, [1] = 3}, 
            .inner = {.c = 4, .d = 5} 
        }, 
        .e = 6 
    };
    
    /* Use variables to prevent optimization */
    volatile struct S sink_s1 = s1;
    volatile struct S sink_s2 = s2;
    volatile int sink_arr1 = arr1[0];
    volatile struct Outer sink_outer = outer;
    volatile union U sink_u1 = u1;
    volatile struct Complex sink_comp = comp;
}

/* OpenMP section for OMP_CLAUSE nodes */
#ifdef _OPENMP
void create_omp_clauses(void) {
    int i, n = 100;
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (i = 0; i < n; i++) {
        arr[i] = i;
    }
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel default(none) private(i) shared(n, arr, sum) \
                         num_threads(4) if(n > 50)
    {
        int local_sum = 0;
        
        /* For loop with schedule clause */
        #pragma omp for schedule(dynamic, 5) nowait \
                     reduction(+:sum) ordered \
                     collapse(1)
        for (i = 0; i < n; i++) {
            #pragma omp ordered
            sum += arr[i];
            local_sum += arr[i] * 2;
        }
        
        /* Barrier */
        #pragma omp barrier
        
        /* Single construct */
        #pragma omp single copyprivate(i) nowait
        {
            i = 0;
        }
        
        /* Critical section */
        #pragma omp critical (my_critical)
        {
            sum += local_sum;
        }
        
        /* Master section */
        #pragma omp master
        {
            sum *= 2;
        }
        
        /* Sections */
        #pragma omp sections private(i) lastprivate(sum)
        {
            #pragma omp section
            {
                i = 1;
                sum += i;
            }
            
            #pragma omp section
            {
                i = 2;
                sum += i * 2;
            }
        }
        
        /* Task construct */
        #pragma omp task untied mergeable if(n > 10) final(n > 90) \
                         priority(1) depend(inout: sum)
        {
            sum += 1000;
        }
        
        /* Taskwait */
        #pragma omp taskwait
        
        /* Atomic operation */
        #pragma omp atomic update
        sum += 1;
        
        /* Flush */
        #pragma omp flush(sum)
    }
    
    /* Parallel for with reduction */
    #pragma omp parallel for reduction(+:sum) \
                schedule(static, 10) linear(i:1) \
                aligned(arr: 16) safelen(8) simdlen(4)
    for (i = 0; i < n; i++) {
        sum += arr[i] * 3;
    }
    
    /* Teams and distribute */
    #pragma omp target teams distribute parallel for \
                map(tofrom: sum) device(0) num_teams(2) \
                thread_limit(8) defaultmap(tofrom:scalar)
    for (i = 0; i < n; i++) {
        sum += arr[i] * 4;
    }
    
    global_sink = sum;
}
#endif

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* Test 1: IDENTIFIER_NODE */
    printf("Testing IDENTIFIER_NODE creation...\n");
    create_identifiers();
    checksum += 1;
    
    /* Test 2: TREE_VEC */
    printf("Testing TREE_VEC creation...\n");
    create_tree_vec();
    checksum += 2;
    
    /* Test 3: SSA_NAME */
    printf("Testing SSA_NAME creation...\n");
    checksum += create_ssa_names(20);
    
    /* Test 4: BLOCK */
    printf("Testing BLOCK creation...\n");
    create_blocks();
    checksum += 4;
    
    /* Test 5: CONSTRUCTOR */
    printf("Testing CONSTRUCTOR creation...\n");
    create_constructors();
    checksum += 8;
    
#ifdef __cplusplus
    /* Test 6: TREE_BINFO (C++ only) */
    printf("Testing TREE_BINFO creation...\n");
    use_inheritance();
    checksum += 16;
#endif

#ifdef _OPENMP
    /* Test 7: OMP_CLAUSE */
    printf("Testing OMP_CLAUSE creation...\n");
    create_omp_clauses();
    checksum += 32;
#endif
    
    /* Call external functions for unresolved identifiers */
    checksum += external_func1();
    external_func2(checksum);
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum & 0xFF;  /* Return non-zero to indicate test ran */
}
