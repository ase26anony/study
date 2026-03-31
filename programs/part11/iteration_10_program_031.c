/* test_tree.c - Comprehensive tree node coverage test */
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
    virtual void virt() {}
};

struct Derived : Base {
    int b;
    void virt() override {}
};

struct MultiDerived : Derived {
    int c;
};

void use_inheritance() {
    MultiDerived md;
    md.a = 1;      /* Accesses through inheritance hierarchy */
    md.b = 2;
    md.c = 3;
    Base* bp = &md;
    bp->virt();    /* Virtual call */
}
#endif

/* Function to create SSA_NAME nodes with complex control flow */
int ssa_test(int n) {
    int i, s = 0, t = 1, u = 2;
    
    /* Complex loop with multiple assignments */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t = u + 1;
        } else {
            s *= 2;
            u = t - 1;
        }
        
        /* Nested condition */
        if (s > 100) {
            t = s / 2;
            if (u < 50) {
                u = t + u;
            }
        }
    }
    
    /* Another loop with phi nodes */
    int j = 0, k = 0;
    while (j < n) {
        if (j % 3 == 0) {
            k = j * 2;
        } else if (j % 3 == 1) {
            k = j + k;
        } else {
            k = k - j;
        }
        j++;
    }
    
    return s + t + u + k;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
void identifier_test(void) {
    int x = 1;
    global_sink = x;
    
    {
        /* Shadowing variable */
        float x = 2.0f;
        global_sink = (int)x;
        
        {
            /* Another shadow */
            volatile int x = 3;
            global_sink = x;
            
            {
                /* External declaration in inner scope */
                extern int external_x;
                volatile int y = external_x + x;
                global_sink = y;
            }
        }
    }
    
    /* More shadowing in loops */
    for (int i = 0; i < 5; i++) {
        double x = i * 1.5;
        global_sink += (int)x;
        
        for (int j = 0; j < 3; j++) {
            /* Different type, same name */
            char x = 'A' + j;
            global_sink += x;
        }
    }
    
    /* Function parameter shadowing */
    {
        auto func = [](int x) -> int {
            {
                long x = x * 2L;  /* Shadows parameter */
                return (int)x;
            }
        };
        global_sink += func(10);
    }
}

/* Function for TREE_VEC nodes using vector extensions */
void vector_test(void) {
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    /* Vector operations */
    v4si e = c + d;
    v4si f = e - a;
    
    /* Store to volatile to prevent elimination */
    volatile v4si sink_vec = f;
    (void)sink_vec;
    
    /* Float vectors */
    v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fb = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fc = fa * fb;
    
    /* Array compound literals */
    int *p = (int[]){1, 2, 3, 4, 5};
    int *q = (int[]){[0] = 10, [2] = 20, [4] = 30};
    
    /* Nested array initializers */
    int arr[2][3] = { {1, 2, 3}, {4, 5, 6} };
    int arr2[3][2] = { [0][0] = 1, [1][1] = 2, [2][0] = 3 };
    
    global_sink += p[0] + q[2] + arr[1][1] + arr2[2][0];
}

/* Function with complex blocks for BLOCK nodes */
void block_test(void) {
    int a = 0;
    
    /* Label and goto creating basic blocks */
    start:
    a++;
    
    {
        int b = a * 2;
        
        inner_block:
        b += 5;
        
        {
            int c = b + 10;
            if (c > 20) {
                goto end_block;
            } else {
                goto inner_block;
            }
        }
    }
    
    end_block:
    
    /* Switch with blocks */
    switch (a) {
        case 1: {
            int x = 100;
            goto start;
        }
        case 2: {
            int y = 200;
            break;
        }
        default: {
            int z = 300;
            goto final;
        }
    }
    
    final:
    
    /* Nested blocks with variables */
    {
        int level1 = 1;
        {
            int level2 = level1 + 1;
            {
                int level3 = level2 + 1;
                {
                    int level4 = level3 + 1;
                    global_sink += level4;
                }
            }
        }
    }
}

/* Function for CONSTRUCTOR nodes with complex initializers */
void constructor_test(void) {
    /* Struct with designated initializers */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Rect {
        struct Point p1;
        struct Point p2;
        int id;
    };
    
    /* Complex designated initializers */
    struct Point p1 = { .x = 1, .y = 2, .z = 3 };
    struct Point p2 = { .y = 20, .z = 30, .x = 10 };
    struct Point p3 = { .x = 100 };
    
    /* Nested designated initializers */
    struct Rect r1 = { 
        .p1 = { .x = 1, .y = 2, .z = 3 },
        .p2 = { .x = 4, .y = 5, .z = 6 },
        .id = 100
    };
    
    struct Rect r2 = {
        .p1.x = 10,
        .p2.y = 20,
        .id = 200
    };
    
    /* Array with designated initializers */
    int arr1[10] = { [0] = 1, [5] = 2, [9] = 3 };
    int arr2[5][3] = { [0][0] = 1, [1][1] = 2, [2][2] = 3 };
    
    /* Union initializers */
    union Data {
        int i;
        float f;
        char str[20];
    };
    
    union Data d1 = { .i = 100 };
    union Data d2 = { .f = 3.14f };
    union Data d3 = { .str = "Hello" };
    
    /* Partial initialization */
    struct Partial {
        int a;
        int b;
        int c;
        int d;
    };
    
    struct Partial part = { .a = 1, .c = 3 };
    
    global_sink += p1.x + r1.id + arr1[5] + d1.i + part.c;
}

/* OpenMP test for OMP_CLAUSE nodes */
void omp_test(int n) {
    int i, sum = 0, prod = 1;
    int arr[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP with multiple clauses */
    #pragma omp parallel for private(i) shared(arr, sum) reduction(+:sum) schedule(dynamic, 4) if(n > 1000)
    for (i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP region with different clauses */
    #pragma omp parallel num_threads(4) default(none) firstprivate(n) copyin(global_sink)
    {
        int local_sum = 0;
        #pragma omp for reduction(*:prod) nowait
        for (i = 0; i < 50; i++) {
            prod *= (arr[i] % 10) + 1;
        }
        
        #pragma omp barrier
        
        #pragma omp single copyprivate(local_sum)
        {
            local_sum = sum / 2;
        }
    }
    
    /* OpenMP sections */
    #pragma omp parallel sections private(i) lastprivate(prod)
    {
        #pragma omp section
        {
            for (i = 0; i < 25; i++) {
                arr[i] *= 2;
            }
        }
        
        #pragma omp section
        {
            for (i = 25; i < 50; i++) {
                arr[i] /= 2;
            }
        }
    }
    
    global_sink += sum + prod;
}

int main(void) {
    int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* Test 1: IDENTIFIER_NODE */
    printf("Testing IDENTIFIER_NODE...\n");
    identifier_test();
    checksum += global_sink;
    
    /* Test 2: TREE_VEC */
    printf("Testing TREE_VEC...\n");
    vector_test();
    checksum += global_sink;
    
    /* Test 3: SSA_NAME */
    printf("Testing SSA_NAME...\n");
    checksum += ssa_test(100);
    
    /* Test 4: BLOCK nodes */
    printf("Testing BLOCK nodes...\n");
    block_test();
    checksum += global_sink;
    
    /* Test 5: CONSTRUCTOR nodes */
    printf("Testing CONSTRUCTOR nodes...\n");
    constructor_test();
    checksum += global_sink;
    
    /* Test 6: OpenMP clauses */
    printf("Testing OMP_CLAUSE nodes...\n");
    omp_test(100);
    checksum += global_sink;
    
#ifdef __cplusplus
    /* Test 7: TREE_BINFO (C++ only) */
    printf("Testing TREE_BINFO nodes...\n");
    use_inheritance();
#endif
    
    printf("Final checksum: %d\n", checksum);
    
    /* Call external functions to create unresolved identifiers */
    external_func1();
    external_func2(checksum);
    
    return checksum != 0 ? 0 : 1;
}
