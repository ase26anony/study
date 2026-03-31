/* test_tree.c - Comprehensive tree node coverage test */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(void);
extern void external_func2(int);
extern volatile int external_var;

/* Helper to prevent optimization */
static volatile int sink;

/* Macro to use results and prevent dead code elimination */
#define USE(x) do { sink = (x); } while(0)

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

struct DeepDerived : Derived {
    int c;
};

void use_inheritance() {
    DeepDerived dd;
    dd.a = 1;      /* Access through inheritance chain */
    dd.b = 2;
    dd.c = 3;
    USE(dd.a + dd.b + dd.c);
    
    Base* bp = &dd;
    bp->virt();    /* Virtual call */
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
            k = result % 100;
            result = k;
        } else if (result < 0) {
            k = -result;
            result = k;
        }
        
        /* Switch to create more phi nodes */
        switch (i % 4) {
            case 0: result <<= 1; break;
            case 1: result >>= 1; break;
            case 2: result ^= 0xFF; break;
            case 3: result |= 0xAA; break;
        }
    }
    
    return result;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
void identifier_generator() {
    int x = 1;
    USE(x);
    
    {
        /* Shadow outer x */
        float x = 2.0f;
        USE((int)x);
        
        {
            /* Another shadow */
            char x = 'A';
            USE(x);
            
            {
                /* Reference to external x */
                extern int x;  /* Unresolved identifier */
                volatile int y = x;  /* Use it */
                USE(y);
            }
        }
    }
    
    /* More shadowing with different types */
    {
        double x = 3.14;
        USE((int)x);
        
        for (int x = 0; x < 5; x++) {
            /* Loop variable shadows outer x */
            volatile int y = x * 2;
            USE(y);
            
            {
                /* Block inside loop with another x */
                long x = 100L;
                USE((int)x);
            }
        }
    }
    
    /* Function parameter shadowing */
    auto shadow_func = [](int x) -> int {
        {
            float x = x * 1.5f;  /* Parameter shadowed in block */
            USE((int)x);
        }
        return x;
    };
    
    USE(shadow_func(10));
}

/* Function to generate TREE_VEC nodes using vector extensions */
void vector_generator() {
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c - d;
    
    USE(e[0] + e[1] + e[2] + e[3]);
    
    v4sf f = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf g = f * 2.0f;
    USE((int)g[0]);
    
    v8hi h = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi i = h << 1;
    USE(i[0] + i[7]);
    
    /* Array compound literals */
    int *p = (int[]){1, 2, 3, 4, 5};
    int *q = (int[]){[0] = 10, [2] = 20, [4] = 30};
    
    USE(p[0] + q[2]);
    
    /* Nested vector operations */
    v4si j = {a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]};
    v4si k = j * (v4si){2, 2, 2, 2};
    USE(k[0] + k[3]);
}

/* Function with complex blocks for BLOCK nodes */
void block_generator() {
    int a = 0;
    
    /* Multiple nested blocks with labels */
    {
        int b = 1;
    lab1:
        b = a + 1;
        USE(b);
        
        {
            int c = 2;
            if (b > 0) {
                goto lab2;  /* Jump to outer block */
            }
            c = 3;
        }
        
        b = 4;
    }
    
lab2:
    a = 5;
    
    {
        int d = 6;
    lab3:
        d = a * 2;
        
        {
            int e = 7;
            if (d > 10) {
                goto lab4;
            }
            goto lab3;  /* Local jump */
        }
    }
    
    /* Unreachable but creates blocks */
    if (0) {
    lab4:
        a = 100;
    }
    
    USE(a);
    
    /* Switch with blocks in cases */
    switch (a) {
        case 5: {
            int x = 50;
            {
                int y = x * 2;
                USE(y);
            }
            break;
        }
        case 10: {
            int x = 100;
            goto lab5;
        }
        default: {
            int x = 0;
            USE(x);
        }
    }
    
lab5:
    a = 999;
    USE(a);
}

/* Function to generate CONSTRUCTOR nodes */
void constructor_generator() {
    /* Struct with designated initializers */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Rect {
        struct Point p1;
        struct Point p2;
        int color;
    };
    
    /* Complex designated initializers */
    struct Point p1 = { .x = 1, .y = 2, .z = 3 };
    struct Point p2 = { .y = 20, .z = 30, .x = 10 };
    struct Point p3 = { .x = 100, .z = 300 };  /* Partial */
    
    USE(p1.x + p2.y + p3.z);
    
    /* Nested designated initializers */
    struct Rect r1 = {
        .p1 = { .x = 1, .y = 2, .z = 3 },
        .p2 = { .x = 4, .y = 5, .z = 6 },
        .color = 0xFF0000
    };
    
    struct Rect r2 = {
        .p2.y = 50,
        .color = 0x00FF00,
        .p1 = { .x = 10, .z = 30 }
    };
    
    USE(r1.p1.x + r2.p2.y);
    
    /* Array with designated initializers */
    int arr1[10] = { [0] = 1, [5] = 2, [9] = 3 };
    int arr2[10] = { [1 ... 3] = 10, [7 ... 9] = 20 };
    
    USE(arr1[5] + arr2[3]);
    
    /* Nested array in struct */
    struct WithArray {
        int id;
        int values[5];
        struct Point points[3];
    };
    
    struct WithArray wa = {
        .id = 1,
        .values = { [0] = 100, [2] = 200, [4] = 300 },
        .points = {
            [0] = { .x = 1, .y = 2 },
            [2] = { .x = 3, .y = 4, .z = 5 }
        }
    };
    
    USE(wa.values[2] + wa.points[0].x);
    
    /* Union initializers */
    union Data {
        int i;
        float f;
        char str[4];
    };
    
    union Data d1 = { .i = 42 };
    union Data d2 = { .f = 3.14f };
    union Data d3 = { .str = "ABC" };
    
    USE(d1.i + (int)d2.f + d3.str[0]);
}

/* OpenMP function for OMP_CLAUSE nodes */
void omp_generator(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    static int shared_var = 0;
    
    /* OpenMP with multiple clauses */
    #pragma omp parallel for private(i) shared(sum, shared_var) \
                reduction(+:sum) schedule(dynamic, 2) \
                num_threads(4) if(n > 1000)
    for (i = 0; i < n; i++) {
        int local = i * i;
        sum += local;
        #pragma omp atomic
        shared_var++;
    }
    
    USE(sum + shared_var);
    
    /* Another OpenMP region with different clauses */
    int max_val = 0;
    int min_val = 0;
    
    #pragma omp parallel sections private(private_var) \
                firstprivate(n) lastprivate(max_val, min_val) \
                copyin(shared_var) nowait
    {
        #pragma omp section
        {
            private_var = n * 2;
            #pragma omp critical
            {
                if (private_var > max_val) max_val = private_var;
            }
        }
        
        #pragma omp section
        {
            private_var = n / 2;
            #pragma omp critical
            {
                if (private_var < min_val || min_val == 0) min_val = private_var;
            }
        }
    }
    
    USE(max_val + min_val);
    
    /* OpenMP with collapse clause */
    int matrix_sum = 0;
    
    #pragma omp parallel for collapse(2) reduction(+:matrix_sum) \
                ordered schedule(static)
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 10; col++) {
            #pragma omp ordered
            matrix_sum += row * col;
        }
    }
    
    USE(matrix_sum);
}

int main() {
    int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* Test 1: IDENTIFIER_NODE generation */
    printf("Testing IDENTIFIER_NODE...\n");
    identifier_generator();
    checksum += sink;
    
    /* Test 2: TREE_VEC generation */
    printf("Testing TREE_VEC...\n");
    vector_generator();
    checksum += sink;
    
    /* Test 3: SSA_NAME generation */
    printf("Testing SSA_NAME...\n");
    checksum += ssa_generator(100);
    
    /* Test 4: BLOCK generation */
    printf("Testing BLOCK...\n");
    block_generator();
    checksum += sink;
    
    /* Test 5: CONSTRUCTOR generation */
    printf("Testing CONSTRUCTOR...\n");
    constructor_generator();
    checksum += sink;
    
    /* Test 6: OMP_CLAUSE generation */
    printf("Testing OMP_CLAUSE...\n");
    omp_generator(1000);
    checksum += sink;
    
    /* Test 7: C++ BINFO generation (if in C++ mode) */
#ifdef __cplusplus
    printf("Testing BINFO (C++ only)...\n");
    use_inheritance();
    checksum += sink;
#endif
    
    /* Test 8: Mixed test with all features */
    printf("Running mixed test...\n");
    {
        /* Combined test block */
        volatile int mixed = 0;
        
        /* Vector in block */
        typedef int v2si __attribute__((vector_size(8)));
        v2si v = {1, 2};
        mixed += v[0];
        
        /* Shadowing in nested blocks */
        {
            int x = 1;
            {
                float x = 2.0f;
                mixed += (int)x;
            }
        }
        
        /* Constructor in block */
        struct { int a; int b; } s = { .a = 1, .b = 2 };
        mixed += s.a + s.b;
        
        USE(mixed);
        checksum += sink;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
