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

void use_inheritance() {
    Derived d;
    d.a = 1;  /* This should generate BINFO nodes */
    d.b = 2;
    
    DeepDerived dd;
    dd.a = 3;
    dd.b = 4;
    dd.c = 5;
    
    Base* bp = &d;
    bp->vfunc();
}
#endif

/* Function to generate SSA_NAME nodes with complex control flow */
int ssa_generator(int n) {
    int i, j, k = 0;
    volatile int sink = 0;
    
    /* Complex control flow for SSA */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            for (j = 0; j < i; j++) {
                if (j % 3 == 0) {
                    k += j * 2;
                } else if (j % 3 == 1) {
                    k -= j / 2;
                } else {
                    k = k * 3 + 1;
                }
            }
        } else {
            int temp = i * i;
            while (temp > 0) {
                k += temp % 10;
                temp /= 10;
            }
        }
        
        /* Switch to add more complexity */
        switch (i % 4) {
            case 0: k = k << 1; break;
            case 1: k = k >> 1; break;
            case 2: k = k ^ 0x55; break;
            case 3: k = ~k; break;
        }
    }
    
    sink = k;  /* Prevent dead code elimination */
    return k;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
void nested_scopes() {
    /* Level 1 */
    int x = 1;
    volatile int y = x;
    
    {
        /* Level 2 - shadow x */
        int x = 2;
        y += x;
        
        {
            /* Level 3 - another x */
            volatile int x = 3;
            y += x;
            
            {
                /* Level 4 - yet another x */
                int x = 4;
                y += x;
                
                {
                    /* Level 5 - pointer to x */
                    int* x = &y;
                    *x += 5;
                }
            }
        }
    }
    
    /* More shadowing in loops */
    for (int i = 0; i < 3; i++) {
        int x = i * 10;  /* New x in loop scope */
        y += x;
        
        for (int j = 0; j < 2; j++) {
            int x = j * 100;  /* Another x in inner loop */
            y += x;
        }
    }
    
    global_sink += y;
}

/* Function using GCC vector extensions for TREE_VEC */
void vector_operations() {
    /* Different vector types */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    /* Vector initializations */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c - d;
    
    /* More complex vector operations */
    v4sf f = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf g = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf h = f * g + g;
    
    v8hi v1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi v2 = {8, 7, 6, 5, 4, 3, 2, 1};
    v8hi v3 = v1 & v2;
    v8hi v4 = v1 | v2;
    
    /* Array compound literals */
    int* arr1 = (int[]){1, 2, 3, 4, 5};
    int* arr2 = (int[]){[0] = 10, [2] = 20, [4] = 30};
    int* arr3 = (int[]){[0 ... 9] = 42, [5] = 99};
    
    /* Nested array initializers */
    int* arr4 = (int[]){1, {2, 3}, 4};
    
    volatile int sink = 0;
    sink += c[0] + h[0] + v3[0] + arr1[0] + arr2[0] + arr3[0] + arr4[0];
    global_sink += sink;
}

/* Function with complex blocks and labels for BLOCK nodes */
void block_test() {
    int a = 0;
    
    /* Block 1 */
    {
        int b = 1;
    lab1:
        b = a + 1;
        
        /* Nested block */
        {
            int c = 2;
            if (a > 0) {
                goto lab3;
            }
        lab2:
            c = 3;
            goto lab4;
        }
        
        goto lab2;
    }
    
    /* Block 2 */
    {
        int d = 4;
    lab3:
        d = 5;
        goto lab5;
    }
    
    /* Block 3 */
    {
        int e = 6;
    lab4:
        e = 7;
        goto lab6;
    }
    
    /* Block 4 */
    {
        int f = 8;
    lab5:
        f = 9;
        {
            int g = 10;
        lab6:
            g = 11;
            a = f + g;
        }
    }
    
    global_sink += a;
}

/* Function with various constructors for CONSTRUCTOR nodes */
void constructor_test() {
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
    struct Point p3 = { .x = 100, .z = 300 };  /* Partial init */
    
    struct Rect r1 = { 
        .p1 = { .x = 1, .y = 2 }, 
        .p2 = { .z = 3 }, 
        .id = 42 
    };
    
    /* Array with designated initializers */
    int arr1[10] = { [0] = 1, [5] = 2, [9] = 3 };
    int arr2[5][3] = { [0][0] = 1, [2][1] = 2, [4][2] = 3 };
    
    /* Nested array initializers */
    int arr3[3][3] = { {1, 2, 3}, {[1] = 4, 5}, {6} };
    
    /* Union initializers */
    union Data {
        int i;
        float f;
        char str[20];
    };
    
    union Data d1 = { .i = 42 };
    union Data d2 = { .f = 3.14f };
    union Data d3 = { .str = "Hello" };
    
    /* Anonymous struct/union */
    struct {
        union {
            int a;
            float b;
        };
        struct {
            int x;
            int y;
        };
    } anon = { .a = 1, .x = 2, .y = 3 };
    
    volatile int sink = 0;
    sink += p1.x + p2.y + p3.z + r1.id + arr1[0] + arr2[0][0] + arr3[0][0];
    sink += d1.i + (int)d2.f + d3.str[0] + anon.a + anon.x;
    global_sink += sink;
}

/* OpenMP section for OMP_CLAUSE nodes */
void omp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    int shared_var = 0;
    int reduction_var = 0;
    
    /* Test 1: Basic parallel for with multiple clauses */
    #pragma omp parallel for private(i, private_var) shared(shared_var) \
        reduction(+:reduction_var) schedule(dynamic, 4) num_threads(2) \
        if(n > 100)
    for (i = 0; i < n; i++) {
        private_var = i * 2;
        shared_var += 1;
        reduction_var += private_var;
    }
    
    /* Test 2: Parallel region with sections */
    #pragma omp parallel default(none) shared(sum, n) private(i) \
        firstprivate(private_var) copyin(shared_var)
    {
        #pragma omp sections nowait
        {
            #pragma omp section
            {
                for (i = 0; i < n/2; i++) {
                    sum += i;
                }
            }
            
            #pragma omp section
            {
                for (i = n/2; i < n; i++) {
                    sum += i * 2;
                }
            }
        }
        
        #pragma omp barrier
        
        #pragma omp single copyprivate(private_var)
        {
            private_var = sum;
        }
    }
    
    /* Test 3: Task construct with dependencies */
    int a = 0, b = 0, c = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: a) priority(1)
        { a = 1; }
        
        #pragma omp task depend(out: b) priority(2)
        { b = 2; }
        
        #pragma omp task depend(in: a, b) depend(out: c) \
            final(n < 50) mergeable
        { c = a + b; }
        
        #pragma omp taskwait
    }
    
    /* Test 4: SIMD loop */
    int simd_arr[100];
    #pragma omp simd simdlen(8) safelen(16) linear(i:1) \
        aligned(simd_arr:32) reduction(+:sum)
    for (i = 0; i < 100; i++) {
        simd_arr[i] = i;
        sum += simd_arr[i];
    }
    
    /* Test 5: Target offloading */
    #pragma omp target map(tofrom: sum) device(0) \
        depend(inout: sum) nowait
    {
        sum *= 2;
    }
    
    #pragma omp taskwait
    
    global_sink += sum + reduction_var + c;
}

int main() {
    int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* Test 1: Nested scopes for IDENTIFIER_NODE */
    nested_scopes();
    checksum += global_sink;
    printf("IDENTIFIER_NODE test completed\n");
    
    /* Test 2: Vector operations for TREE_VEC */
    vector_operations();
    checksum += global_sink;
    printf("TREE_VEC test completed\n");
    
    /* Test 3: C++ inheritance for TREE_BINFO */
    #ifdef __cplusplus
    use_inheritance();
    checksum += global_sink;
    printf("TREE_BINFO test completed\n");
    #endif
    
    /* Test 4: Complex control flow for SSA_NAME */
    checksum += ssa_generator(50);
    printf("SSA_NAME test completed\n");
    
    /* Test 5: Block and labels for BLOCK */
    block_test();
    checksum += global_sink;
    printf("BLOCK test completed\n");
    
    /* Test 6: Constructors for CONSTRUCTOR */
    constructor_test();
    checksum += global_sink;
    printf("CONSTRUCTOR test completed\n");
    
    /* Test 7: OpenMP for OMP_CLAUSE */
    #ifdef _OPENMP
    omp_test(1000);
    checksum += global_sink;
    printf("OMP_CLAUSE test completed\n");
    #endif
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed successfully!\n");
    
    return 0;
}
