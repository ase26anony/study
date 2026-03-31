/* test_tree.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(void);
extern void external_func2(int);
extern volatile int external_var;

/* Volatile sink to prevent optimization */
static volatile int sink = 0;

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

void use_hierarchy(Base *b) {
    b->a = 42;
    sink = b->a;
}
#endif

/* Function to create SSA_NAME nodes with complex control flow */
int create_ssa_names(int n) {
    int i, s = 0, t = 1, u = 2;
    
    /* Complex loop with multiple assignments */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t = u + 1;
        } else {
            s *= 2 + u;
            u = t - 1;
        }
        
        /* Nested condition */
        if (s > 100) {
            t = s / 2;
            if (u < 50) {
                u = u * 3;
            }
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
            s *= 2;
        }
        j++;
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
                    extern int x; /* Unresolved identifier */
                    result += 4;
                }
            }
        }
        
        /* Another block at level 2 */
        {
            /* Yet another x */
            long x = 5;
            result += (int)x;
            
            /* Reference outer x through pointer */
            {
                int *ptr = &x;
                result += *ptr;
            }
        }
    }
    
    /* More nesting with different variable types */
    {
        double x = 6.0;
        result += (int)x;
        
        {
            char x = '7';
            result += x - '0';
            
            {
                short x = 8;
                result += x;
                
                {
                    unsigned x = 9;
                    result += x;
                }
            }
        }
    }
    
    return result;
}

/* Function to create TREE_VEC nodes using GCC extensions */
void vector_operations(void) {
    /* Vector types using GCC extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    /* Vector initialization and operations */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c - d;
    
    /* More complex vector expressions */
    v4si f = (a + b) * (c - d);
    v4si g = {9, 10, 11, 12};
    v4si h = a + b * c - d / g;
    
    /* Float vectors */
    v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fb = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fc = fa * fb + fa / fb;
    
    /* Array compound literals (also create TREE_VEC) */
    int *arr1 = (int[]){1, 2, 3, 4, 5};
    int *arr2 = (int[3]){6, 7, 8};
    
    /* Nested array initializers */
    int *arr3 = (int[]){[0] = 9, [2] = 10, [4] = 11};
    
    /* Use vectors to prevent dead code elimination */
    sink += c[0] + d[1] + e[2] + f[3];
    sink += (int)fc[0] + (int)fc[1];
    sink += arr1[0] + arr2[1] + arr3[2];
}

/* Function with complex blocks and labels for BLOCK nodes */
int block_and_labels_test(void) {
    int a = 0, b = 0, c = 0;
    
    /* Block 1 */
    {
        int x = 1;
    block1_label:
        a = x;
        
        /* Nested block */
        {
            int y = 2;
            if (a > 0)
                goto block2_label;
            b = y;
        }
        
        goto block3_label;
    }
    
    /* Block 2 */
    {
        int z = 3;
    block2_label:
        c = z;
        goto block4_label;
    }
    
    /* Block 3 */
    {
        int w = 4;
    block3_label:
        a += w;
        goto block5_label;
    }
    
    /* Block 4 */
    {
        int v = 5;
    block4_label:
        b += v;
        goto block6_label;
    }
    
    /* Block 5 */
    {
        int u = 6;
    block5_label:
        c += u;
        goto block7_label;
    }
    
    /* Block 6 */
    {
        int t = 7;
    block6_label:
        a *= t;
        goto block8_label;
    }
    
    /* Block 7 */
    {
        int s = 8;
    block7_label:
        b *= s;
        /* Fall through */
    }
    
    /* Block 8 */
    {
        int r = 9;
    block8_label:
        c *= r;
    }
    
    return a + b + c;
}

/* Function with constructors for CONSTRUCTOR nodes */
void constructor_tests(void) {
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
    
    /* Various initializers */
    struct Point p1 = { .x = 1, .y = 2, .z = 3 };
    struct Point p2 = { .y = 5, .z = 6, .x = 4 }; /* Out of order */
    struct Point p3 = { .x = 7 }; /* Partial init */
    struct Point p4 = { 8, 9 }; /* Traditional, partial */
    
    /* Nested designated initializers */
    struct Rect r1 = {
        .p1 = { .x = 1, .y = 2 },
        .p2 = { .z = 3 },
        .id = 100
    };
    
    struct Rect r2 = {
        .p1.x = 10,
        .p2.y = 20,
        .id = 200
    };
    
    /* Array with designated initializers */
    int arr1[10] = { [0] = 1, [5] = 2, [9] = 3 };
    int arr2[5] = { [1 ... 3] = 42 };
    
    /* Nested array in struct */
    struct WithArray {
        int data[5];
        char name[10];
    };
    
    struct WithArray wa1 = {
        .data = { [0] = 1, [2] = 3, [4] = 5 },
        .name = { 't', 'e', 's', 't' }
    };
    
    struct WithArray wa2 = {
        .data = { 1, 2, [4] = 5 },
        .name = "hello"
    };
    
    /* Union initializers */
    union Mixed {
        int i;
        float f;
        char c[4];
    };
    
    union Mixed u1 = { .i = 42 };
    union Mixed u2 = { .f = 3.14f };
    union Mixed u3 = { .c = { 'a', 'b', 'c' } };
    
    /* Use all variables to prevent optimization */
    sink += p1.x + p2.y + p3.z + p4.x;
    sink += r1.id + r2.id;
    sink += arr1[0] + arr2[1];
    sink += wa1.data[0] + wa2.data[4];
    sink += u1.i + (int)u2.f + u3.c[0];
}

/* OpenMP tests for OMP_CLAUSE nodes */
void openmp_tests(int n) {
    int i, sum = 0, private_var = 0;
    int arr[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Test 1: Parallel for with multiple clauses */
    #pragma omp parallel for private(i) shared(arr, sum) reduction(+:sum) schedule(dynamic, 2) if(n > 1000)
    for (i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* Test 2: Parallel region with various clauses */
    #pragma omp parallel num_threads(4) default(none) shared(sum, arr) private(private_var) firstprivate(n)
    {
        private_var = omp_get_thread_num();
        #pragma omp critical
        {
            sum += private_var;
        }
    }
    
    /* Test 3: Sections */
    #pragma omp parallel sections private(i) reduction(+:sum)
    {
        #pragma omp section
        {
            for (i = 0; i < 25; i++) sum += arr[i];
        }
        
        #pragma omp section
        {
            for (i = 25; i < 50; i++) sum += arr[i] * 2;
        }
        
        #pragma omp section
        {
            for (i = 50; i < 75; i++) sum += arr[i] / 2;
        }
    }
    
    /* Test 4: Task with dependencies */
    #pragma omp parallel
    #pragma omp single
    {
        int x = 0, y = 0;
        #pragma omp task depend(out: x)
        { x = 1; }
        
        #pragma omp task depend(in: x) depend(out: y)
        { y = x + 2; }
        
        #pragma omp task depend(in: y)
        { sum += y; }
    }
    
    /* Test 5: SIMD loop */
    #pragma omp simd reduction(+:sum) aligned(arr:16) safelen(8)
    for (i = 0; i < 100; i++) {
        sum += arr[i] * 3;
    }
    
    sink = sum;
}

/* Main test driver */
int main(void) {
    int checksum = 0;
    
    printf("Starting comprehensive tree node coverage test...\n");
    
    /* 1. Test IDENTIFIER_NODE creation */
    printf("Testing IDENTIFIER_NODE...\n");
    checksum += nested_scopes_test();
    
    /* 2. Test TREE_VEC creation */
    printf("Testing TREE_VEC...\n");
    vector_operations();
    checksum += sink;
    
    /* 3. Test SSA_NAME creation */
    printf("Testing SSA_NAME...\n");
    checksum += create_ssa_names(50);
    
    /* 4. Test BLOCK creation */
    printf("Testing BLOCK...\n");
    checksum += block_and_labels_test();
    
    /* 5. Test CONSTRUCTOR creation */
    printf("Testing CONSTRUCTOR...\n");
    constructor_tests();
    checksum += sink;
    
    /* 6. Test OpenMP clauses (compile with -fopenmp) */
#ifdef _OPENMP
    printf("Testing OMP_CLAUSE...\n");
    openmp_tests(1000);
    checksum += sink;
#else
    printf("OpenMP not enabled, skipping OMP_CLAUSE tests\n");
#endif
    
#ifdef __cplusplus
    /* 7. Test BINFO creation (C++ only) */
    printf("Testing TREE_BINFO (C++ only)...\n");
    Derived d;
    DeepDerived dd;
    use_hierarchy(&d);
    use_hierarchy(&dd);
    checksum += sink;
#endif
    
    /* Call external functions to create unresolved identifiers */
    checksum += external_func1();
    external_func2(checksum);
    checksum += external_var;
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
