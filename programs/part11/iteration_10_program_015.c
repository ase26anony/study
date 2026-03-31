/* test_tree.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func(int);
extern volatile int external_var;

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

void use_inheritance() {
    DeepDerived dd;
    dd.a = 1;      /* Accesses through inheritance hierarchy */
    dd.b = 2;
    dd.c = 3;
    
    Base* bp = &dd;
    bp->vfunc();   /* Virtual call */
}
#endif

/* Function to create complex control flow for SSA_NAME */
int complex_control_flow(int n) {
    int i, s = 0, t = 1, u = 2;
    
    /* Complex loop with multiple branches */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t = s ^ i;
        } else {
            s *= u + 1;
            u = (u << 1) | (i & 0xFF);
        }
        
        switch (i % 4) {
            case 0: s += 1; break;
            case 1: s -= t; break;
            case 2: s ^= u; break;
            case 3: s = (s << 1) | (u & 1); break;
        }
    }
    
    /* Nested loops with breaks/continues */
    for (i = 0; i < 100; i++) {
        if (i > 50) break;
        for (int j = 0; j < 10; j++) {
            if (j == i % 10) continue;
            s += j * i;
        }
    }
    
    return s;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
int nested_scopes_test(void) {
    int result = 0;
    
    /* Level 1 */
    {
        volatile int x = 1;
        result += x;
        
        /* Level 2 */
        {
            /* Different x in inner scope */
            volatile int x = 2;
            result += x * 10;
            
            /* Level 3 */
            {
                /* Yet another x */
                extern int x;  /* Unresolved external */
                volatile int y = 3;
                result += y * 100;
                
                /* Level 4 - function scope x */
                {
                    auto int x = 4;  /* C++ auto or C auto storage */
                    volatile int z = x;
                    result += z * 1000;
                }
            }
        }
        
        /* Back to outer x */
        result += x * 10000;
    }
    
    /* Another block with same variable names */
    {
        volatile long x = 5L;
        result += (int)x;
        
        {
            volatile float x = 6.0f;
            result += (int)x;
        }
    }
    
    return result;
}

/* Function using vector extensions for TREE_VEC */
#ifdef __GNUC__
int vector_operations(void) {
    int sum = 0;
    
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    /* Use compound literals with arrays (creates TREE_VEC) */
    int* arr1 = (int[]){1, 2, 3, 4, 5};
    int* arr2 = (int[]){[0] = 10, [2] = 20, [4] = 30};
    
    /* Multi-dimensional compound literal */
    int(*md_arr)[3] = (int[][3]){{1, 2, 3}, {4, 5, 6}};
    
    /* Vector operations */
    v4si e = c + d;
    v4si f = e >> 1;
    
    /* Extract elements to prevent optimization */
    volatile int v0 = e[0];
    volatile int v1 = e[1];
    volatile int v2 = e[2];
    volatile int v3 = e[3];
    
    sum = v0 + v1 + v2 + v3;
    sum += arr1[0] + arr2[2];
    sum += md_arr[1][1];
    
    return sum;
}
#endif

/* Function with complex constructors for CONSTRUCTOR nodes */
int constructor_tests(void) {
    int sum = 0;
    
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
        .p2 = { .x = 10, .y = 20 },
        .id = 100
    };
    
    /* Array with designated initializers */
    int arr[10] = { [0] = 1, [5] = 2, [9] = 3, [2] = 4 };
    
    /* Nested array in struct */
    struct WithArray {
        int data[5];
        char name[10];
    };
    
    struct WithArray wa = {
        .data = { [1] = 10, [3] = 30, [0] = 5 },
        .name = { 't', 'e', 's', 't' }
    };
    
    /* Union initializer */
    union Mixed {
        int i;
        float f;
        char c[4];
    };
    
    union Mixed u1 = { .i = 0x12345678 };
    union Mixed u2 = { .f = 3.14f };
    union Mixed u3 = { .c = { 'a', 'b', 'c', 'd' } };
    
    sum = p1.x + p2.y + p3.z + r1.id;
    sum += arr[5] + wa.data[1];
    sum += u1.i & 0xFF;
    
    return sum;
}

/* Function with complex blocks and labels for BLOCK nodes */
int block_and_label_test(void) {
    int result = 0;
    volatile int a = 0;
    
    /* Outer block with label */
    outer_block: {
        int x = 1;
        result += x;
        
        /* Inner block 1 */
        {
            int y = 2;
            result += y;
            
            /* Deeply nested block */
            {
                int z = 3;
                result += z;
                goto skip_inner;  /* Jump to label in different block */
            }
            
            /* This part skipped */
            result += 100;
        }
        
    skip_inner:
        /* Another inner block */
        {
            volatile int w = 4;
            result += w;
            
            if (a < 10) {
                a++;
                goto outer_block;  /* Jump back to outer block */
            }
        }
    }
    
    /* Switch with compound statements */
    switch (result % 4) {
        case 0: {
            int temp = result * 2;
            result = temp;
            break;
        }
        case 1: {
            int temp = result / 2;
            result = temp;
            break;
        }
        default: {
            int temp = result ^ 0xFF;
            result = temp;
            break;
        }
    }
    
    return result;
}

/* OpenMP parallel region for OMP_CLAUSE nodes */
#ifdef _OPENMP
int openmp_test(int n) {
    int sum = 0;
    int i;
    
    /* Complex OpenMP pragma with multiple clauses */
    #pragma omp parallel for private(i) shared(sum) \
            reduction(+:sum) schedule(dynamic, 2) \
            num_threads(4) if(n > 1000)
    for (i = 0; i < n; i++) {
        sum += i * (i % 7);
    }
    
    /* Another with different clauses */
    #pragma omp parallel sections private(i) \
            firstprivate(n) nowait
    {
        #pragma omp section
        {
            int local_sum = 0;
            for (i = 0; i < n/2; i++) {
                local_sum += i;
            }
            #pragma omp atomic
            sum += local_sum;
        }
        
        #pragma omp section
        {
            int local_prod = 1;
            for (i = 1; i < 10; i++) {
                local_prod *= i;
            }
            #pragma omp atomic
            sum += local_prod;
        }
    }
    
    /* Single directive with copyprivate */
    int master_val = 0;
    #pragma omp parallel private(i)
    {
        #pragma omp single copyprivate(master_val)
        {
            master_val = 42;
        }
        
        #pragma omp for ordered
        for (i = 0; i < 10; i++) {
            #pragma omp ordered
            {
                sum += master_val + i;
            }
        }
    }
    
    return sum;
}
#endif

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    printf("Starting GCC tree node coverage test...\n");
    
    /* 1. Test IDENTIFIER_NODE creation */
    checksum += nested_scopes_test();
    printf("IDENTIFIER_NODE test complete: %d\n", checksum);
    
    /* 2. Test SSA_NAME creation (requires optimization) */
    checksum ^= complex_control_flow(100);
    printf("SSA_NAME test complete: %d\n", checksum);
    
    /* 3. Test TREE_VEC creation */
    #ifdef __GNUC__
    checksum += vector_operations();
    printf("TREE_VEC test complete: %d\n", checksum);
    #endif
    
    /* 4. Test CONSTRUCTOR nodes */
    checksum += constructor_tests();
    printf("CONSTRUCTOR test complete: %d\n", checksum);
    
    /* 5. Test BLOCK nodes */
    checksum += block_and_label_test();
    printf("BLOCK test complete: %d\n", checksum);
    
    /* 6. Test OpenMP clauses */
    #ifdef _OPENMP
    checksum += openmp_test(500);
    printf("OMP_CLAUSE test complete: %d\n", checksum);
    #endif
    
    /* 7. Use external identifiers */
    volatile int ext_use = external_var;
    checksum += ext_use;
    
    /* Call external function */
    checksum += external_func(checksum);
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
