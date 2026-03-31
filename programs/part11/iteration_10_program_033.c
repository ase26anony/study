/* test_tree.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func(int);
extern volatile int global_var;

#ifdef __cplusplus
/* C++ specific code for TREE_BINFO */
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

void use_inheritance() {
    Derived d;
    d.a = 1;  /* Accesses base class member */
    d.b = 2;
    
    DeepDerived dd;
    dd.a = 3;  /* Accesses grandparent member */
    dd.c = 4;
    
    Base* bp = &d;
    bp->vfunc();  /* Virtual call */
}
#endif

/* Function to force SSA_NAME creation with complex control flow */
int ssa_test(int n) {
    int i, s = 0, t = 1;
    
    /* Complex loop with multiple assignments */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t += 2;
        } else {
            s *= 3;
            t -= 1;
        }
        
        /* Nested condition */
        if (s > 100) {
            s /= 2;
            t = t % 5;
        }
    }
    
    /* Another loop with phi nodes */
    int j = 0;
    while (j < n) {
        if (j % 3 == 0) {
            s += j;
        } else if (j % 3 == 1) {
            s -= j;
        } else {
            s ^= j;
        }
        j++;
    }
    
    return s;
}

/* Function with many BLOCK nodes and labels */
void block_test() {
    int outer = 0;
    
    /* Outer block with label */
    outer_block: {
        int a = 1;
        volatile int sink1 = a;
        
        /* Inner block 1 */
        {
            int b = 2;
            volatile int sink2 = b;
            goto inner_label;
            
            /* Unreachable code creates separate block */
            int unreachable1 = 99;
        }
        
        inner_label: {
            int c = 3;
            volatile int sink3 = c;
            
            /* Deeply nested block */
            {
                int d = 4;
                if (outer > 0) {
                    goto outer_block;
                }
                volatile int sink4 = d;
            }
        }
        
        /* Another block with switch */
        {
            int e = 5;
            switch (e) {
                case 5:
                    goto end_block;
                default:
                    break;
            }
            volatile int sink5 = e;
        }
    }
    
    end_block:
    outer++;
}

/* Function to create CONSTRUCTOR nodes */
void constructor_test() {
    /* Struct with designated initializers */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Point p1 = { .x = 1, .y = 2, .z = 3 };
    struct Point p2 = { .y = 5, .z = 6 };  /* Partial initialization */
    
    /* Nested struct initialization */
    struct Line {
        struct Point start;
        struct Point end;
    };
    
    struct Line l1 = {
        .start = { .x = 0, .y = 0, .z = 0 },
        .end = { .x = 10, .y = 20, .z = .y = 30 }  /* Mixed designators */
    };
    
    /* Array with designated initializers */
    int arr[10] = { [0] = 1, [5] = 2, [9] = 3 };
    
    /* 2D array initialization */
    int matrix[3][3] = { [0][0] = 1, [1][1] = 2, [2][2] = 3 };
    
    /* Union initialization */
    union Data {
        int i;
        float f;
        char str[20];
    };
    
    union Data u1 = { .i = 42 };
    union Data u2 = { .f = 3.14 };
    
    volatile int sink = p1.x + p2.y + arr[5] + matrix[1][1] + u1.i;
}

/* Function using GCC vector extensions for TREE_VEC */
void vector_test() {
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
    
    v8hi h = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi i = h << 2;
    
    /* Array compound literals */
    int *p1 = (int[]){1, 2, 3, 4};
    int *p2 = (int[4]){5, 6, 7, 8};
    
    /* Nested compound literals */
    struct VecPair {
        v4si first;
        v4si second;
    };
    
    struct VecPair vp = {
        .first = (v4si){9, 10, 11, 12},
        .second = (v4si){13, 14, 15, 16}
    };
    
    volatile int sink = c[0] + d[1] + e[2] + (int)g[3] + i[4] + p1[0] + vp.first[0];
}

/* Function with many IDENTIFIER_NODE usages */
void identifier_test() {
    /* Same name in different scopes */
    {
        int x = 1;
        volatile int y = x;
        
        {
            /* Shadowing with same name */
            float x = 2.0f;
            volatile float z = x;
            
            {
                /* Another shadow */
                char x = 'A';
                volatile char w = x;
                
                /* Reference outer x through pointer */
                {
                    int* ptr = &((int&)x);  /* Tricky reference */
                    volatile int v = *ptr;
                }
            }
        }
    }
    
    /* More shadowing with loops */
    for (int i = 0; i < 3; i++) {
        int counter = i * 2;
        {
            double counter = 3.14 * i;
            volatile double d = counter;
            
            for (int j = 0; j < 2; j++) {
                long counter = 100L * j;
                volatile long l = counter;
            }
        }
    }
    
    /* Function parameter shadowing */
    auto shadow_func = [](int param) {
        {
            float param = param * 1.5f;  /* Shadows parameter */
            volatile float f = param;
        }
        return param;
    };
    
    shadow_func(10);
}

/* OpenMP section for OMP_CLAUSE nodes */
void omp_test(int n) {
    int i;
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Various OpenMP pragmas with different clauses */
    #pragma omp parallel for private(i) shared(arr, sum) reduction(+:sum) schedule(dynamic, 4) num_threads(2)
    for (i = 0; i < n; i++) {
        sum += arr[i % 100];
    }
    
    /* Another with different clauses */
    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val) collapse(2) ordered
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            #pragma omp ordered
            {
                int val = x * 10 + y;
                if (val > max_val) max_val = val;
            }
        }
    }
    
    /* Single directive with clauses */
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            volatile int single_thread = 1;
        }
        
        #pragma omp for lastprivate(i)
        for (i = 0; i < 5; i++) {
            volatile int loop_var = i;
        }
    }
    
    /* Sections with private data */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            i = 1;
            volatile int sec1 = i;
        }
        
        #pragma omp section
        {
            i = 2;
            volatile int sec2 = i;
        }
    }
    
    volatile int sink = sum + max_val;
}

int main() {
    int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* Test 1: SSA_NAME creation */
    checksum += ssa_test(20);
    printf("SSA test complete, checksum: %d\n", checksum);
    
    /* Test 2: BLOCK nodes */
    block_test();
    checksum += 100;
    printf("Block test complete, checksum: %d\n", checksum);
    
    /* Test 3: CONSTRUCTOR nodes */
    constructor_test();
    checksum += 200;
    printf("Constructor test complete, checksum: %d\n", checksum);
    
    /* Test 4: Vector extensions (TREE_VEC) */
    vector_test();
    checksum += 300;
    printf("Vector test complete, checksum: %d\n", checksum);
    
    /* Test 5: IDENTIFIER_NODE variations */
    identifier_test();
    checksum += 400;
    printf("Identifier test complete, checksum: %d\n", checksum);
    
    /* Test 6: OpenMP clauses */
    omp_test(50);
    checksum += 500;
    printf("OpenMP test complete, checksum: %d\n", checksum);
    
    #ifdef __cplusplus
    /* Test 7: C++ inheritance (TREE_BINFO) */
    use_inheritance();
    checksum += 600;
    printf("C++ inheritance test complete, checksum: %d\n", checksum);
    #endif
    
    /* Use external identifiers */
    checksum += external_func(checksum);
    checksum += global_var;
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return checksum % 256;
}
