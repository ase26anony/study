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

/* Function to create SSA_NAME nodes with complex control flow */
int create_ssa_names(int n) {
    int i, s = 0, t = 1;
    
    /* Complex loop with conditional updates */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t += 2;
        } else {
            s -= i / (t + 1);
            t *= 3;
        }
        
        /* Nested condition */
        if (s > 100) {
            s = s % 100;
            t = t >> 1;
        } else if (s < -50) {
            s = -s;
            t = t << 1;
        }
    }
    
    /* Another loop with switch */
    for (i = 0; i < n; i++) {
        switch (i % 4) {
            case 0: s += i; break;
            case 1: s -= i; t++; break;
            case 2: s *= 2; t--; break;
            case 3: s /= 2; t += i; break;
        }
    }
    
    return s + t;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
int nested_scopes_test() {
    volatile int result = 0;
    
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
                char x = 'A';
                result += x;
                
                /* Level 4 - extern declaration */
                {
                    extern int x;  /* Unresolved identifier */
                    volatile int y = x;  /* This should create IDENTIFIER_NODE */
                    result += y;
                }
            }
        }
        
        /* Another block at level 2 */
        {
            /* Yet another x */
            double x = 3.14159;
            result += (int)x;
            
            /* Use function parameter name as variable */
            {
                int nested_scopes_test = 42;  /* Same name as function */
                result += nested_scopes_test;
            }
        }
    }
    
    /* More nesting with loops */
    for (int i = 0; i < 3; i++) {
        int x = i * 10;
        
        for (int j = 0; j < 2; j++) {
            float x = j * 1.5f;  /* Different x */
            result += (int)x;
            
            if (j == 1) {
                char x = 'X';  /* Another x */
                result += x;
            }
        }
        
        result += x;
    }
    
    return result;
}

/* Function to create TREE_VEC nodes using vector extensions */
#ifdef __GNUC__
int vector_operations() {
    volatile int sum = 0;
    
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    /* Extract elements */
    sum += c[0] + c[1] + c[2] + c[3];
    sum += d[0] + d[1] + d[2] + d[3];
    
    /* More vector operations */
    v4si e = {10, 20, 30, 40};
    v4si f = e > a;
    sum += f[0] + f[1] + f[2] + f[3];
    
    /* Float vectors */
    v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fb = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fc = fa * fb;
    
    sum += (int)fc[0] + (int)fc[1] + (int)fc[2] + (int)fc[3];
    
    /* Array compound literals - also create TREE_VEC */
    int *arr1 = (int[]){1, 2, 3, 4, 5};
    int *arr2 = (int[3]){10, 20, 30};
    
    for (int i = 0; i < 5; i++) sum += arr1[i];
    for (int i = 0; i < 3; i++) sum += arr2[i];
    
    /* Nested array initializer */
    int *arr3 = (int[]){[0] = 100, [2] = 200, [4] = 300};
    for (int i = 0; i < 5; i++) sum += arr3[i];
    
    return sum;
}
#endif

/* Function with BLOCK nodes and goto */
int block_and_goto_test() {
    volatile int val = 0;
    
    /* Outer block */
    {
        int a = 1;
    lab1:
        val += a;
        
        /* Inner block 1 */
        {
            int b = 2;
            val += b;
            goto lab3;  /* Jump forward */
        }
        
    lab2:
        a *= 2;
        val += a;
        goto lab4;
        
        /* Inner block 2 */
        {
            int c = 3;
        lab3:
            val += c;
            goto lab2;  /* Jump backward */
        }
    }
    
lab4:
    /* Another block structure */
    {
        int x = 10;
        
        if (val > 0) {
            /* Block in if */
            int y = 20;
            val += x + y;
            goto done;
        } else {
            /* Block in else */
            int z = 30;
            val += x + z;
        }
        
        /* Loop with block */
        for (int i = 0; i < 3; i++) {
            int loop_var = i * 5;
            val += loop_var;
            
            if (i == 1) {
                goto skip;  /* Jump out of loop block */
            }
        }
        
    skip:
        /* Empty block */
        {}
    }
    
done:
    return val;
}

/* Function with CONSTRUCTOR nodes */
int constructor_test() {
    volatile int sum = 0;
    
    /* Struct with designated initializers */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Point p1 = { .x = 1, .y = 2, .z = 3 };
    sum += p1.x + p1.y + p1.z;
    
    /* Partial initialization */
    struct Point p2 = { .y = 20, .z = 30 };  /* x will be 0 */
    sum += p2.x + p2.y + p2.z;
    
    /* Nested struct */
    struct Line {
        struct Point start;
        struct Point end;
    };
    
    struct Line l1 = {
        .start = { .x = 1, .y = 2 },
        .end = { .z = 3 }
    };
    sum += l1.start.x + l1.end.z;
    
    /* Array with designated initializers */
    int arr1[10] = { [0] = 1, [5] = 2, [9] = 3 };
    for (int i = 0; i < 10; i++) sum += arr1[i];
    
    /* 2D array */
    int matrix[3][3] = { [0][0] = 1, [1][1] = 2, [2][2] = 3 };
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            sum += matrix[i][j];
    
    /* Union initializer */
    union Data {
        int i;
        float f;
        char str[20];
    };
    
    union Data d1 = { .i = 42 };
    union Data d2 = { .f = 3.14f };
    union Data d3 = { .str = "Hello" };
    
    sum += d1.i + (int)d2.f + d3.str[0];
    
    /* Complex nested initializer */
    struct Complex {
        int a;
        struct {
            int b[3];
            int c;
        } inner;
        int d;
    };
    
    struct Complex comp = {
        .a = 1,
        .inner = {
            .b = { [1] = 5, [2] = 6 },
            .c = 7
        },
        .d = 8
    };
    
    sum += comp.a + comp.inner.b[0] + comp.inner.b[1] + 
           comp.inner.b[2] + comp.inner.c + comp.d;
    
    return sum;
}

/* OpenMP section for OMP_CLAUSE nodes */
#ifdef _OPENMP
int omp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    static int shared_var = 0;
    
    /* Test various OpenMP clauses */
    #pragma omp parallel for private(i) shared(sum) reduction(+:sum) schedule(dynamic, 2) if(n > 1000)
    for (i = 0; i < n; i++) {
        sum += i;
    }
    
    /* Another with more clauses */
    #pragma omp parallel private(private_var) firstprivate(n) shared(shared_var) \
                num_threads(4) default(none) copyin(shared_var)
    {
        private_var = omp_get_thread_num();
        #pragma omp atomic
        shared_var += private_var;
    }
    
    /* Sections with nowait */
    #pragma omp parallel
    {
        #pragma omp sections nowait
        {
            #pragma omp section
            { sum += 1; }
            
            #pragma omp section
            { sum += 2; }
        }
        
        #pragma omp barrier
        
        #pragma omp single copyprivate(private_var)
        {
            private_var = sum;
        }
    }
    
    /* Task with depend clause */
    int a = 0, b = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: a)
        { a = 1; }
        
        #pragma omp task depend(in: a) depend(out: b)
        { b = a + 1; }
        
        #pragma omp task depend(in: b)
        { sum += b; }
    }
    
    return sum + shared_var;
}
#endif

/* Main function that runs all tests */
int main() {
    volatile int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* 1. Test IDENTIFIER_NODE creation */
    printf("Testing IDENTIFIER_NODE...\n");
    checksum += nested_scopes_test();
    
    /* 2. Test TREE_VEC creation */
    #ifdef __GNUC__
    printf("Testing TREE_VEC...\n");
    checksum += vector_operations();
    #endif
    
    /* 3. Test SSA_NAME creation */
    printf("Testing SSA_NAME...\n");
    checksum += create_ssa_names(50);
    
    /* 4. Test BLOCK nodes */
    printf("Testing BLOCK nodes...\n");
    checksum += block_and_goto_test();
    
    /* 5. Test CONSTRUCTOR nodes */
    printf("Testing CONSTRUCTOR nodes...\n");
    checksum += constructor_test();
    
    /* 6. Test OpenMP clauses */
    #ifdef _OPENMP
    printf("Testing OMP_CLAUSE nodes...\n");
    checksum += omp_test(100);
    #endif
    
    /* 7. Test C++ BINFO nodes */
    #ifdef __cplusplus
    printf("Testing BINFO nodes...\n");
    use_inheritance();
    checksum += 1234;  /* Add constant for C++ mode */
    #endif
    
    /* Use external identifiers */
    checksum += external_var;
    checksum += external_func(checksum);
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
