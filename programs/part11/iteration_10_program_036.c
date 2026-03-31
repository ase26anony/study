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

void use_inheritance(Derived *d) {
    d->a = 1;  /* This should generate TREE_BINFO */
    d->b = 2;
}
#endif

/* Function to force SSA_NAME creation */
int complex_control_flow(int n) {
    int i, s = 0;
    volatile int sink;
    
    /* Complex loop with conditional updates */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * 2;
        } else {
            s = s * 3 - i;
        }
        
        /* Nested condition */
        if (s > 100) {
            s = s / 2;
        } else if (s < 0) {
            s = -s;
        }
    }
    
    sink = s;  /* Prevent optimization */
    return s;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
int nested_scopes_test(void) {
    volatile int result = 0;
    
    /* Level 1 */
    {
        int x = 10;
        result += x;
        
        /* Level 2 */
        {
            extern int x;  /* Different x in different scope */
            volatile int y = x + 5;
            result += y;
            
            /* Level 3 */
            {
                int x = 20;  /* Another x */
                {
                    volatile int z = x * 2;
                    result += z;
                }
            }
            
            /* Level 3 again */
            {
                float x = 3.14f;  /* Yet another x, different type */
                result += (int)x;
            }
        }
        
        /* Another Level 2 */
        {
            static int x = 30;  /* Static x */
            result += x++;
        }
    }
    
    return result;
}

/* Function for BLOCK nodes with goto */
int block_and_goto_test(void) {
    int a = 0;
    volatile int sink;
    
    /* First block with label */
    {
        int b = 10;
    lab1:
        a += b;
        goto lab3;  /* Skip second block */
    }
    
    /* Second block (skipped initially) */
    {
        int c = 20;
    lab2:
        a += c;
        goto lab4;
    }
    
    /* Third block */
    {
        int d = 30;
    lab3:
        a += d;
        goto lab2;  /* Jump back */
    }
    
    /* Fourth block */
    {
        int e = 40;
    lab4:
        a += e;
    }
    
    sink = a;
    return a;
}

/* Function for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    int b[4];
    struct {
        int x;
        int y;
    } nested;
    union {
        int u1;
        float u2;
    } u;
};

struct ComplexStruct constructor_test(void) {
    /* Various initializers to generate CONSTRUCTOR nodes */
    struct ComplexStruct s1 = {
        .a = 1,
        .b = {[0] = 10, [2] = 30},  /* Partial array init */
        .nested = {.x = 100, .y = 200},
        .u = {.u2 = 3.14f}
    };
    
    /* Array with designated initializers */
    int arr[10] = {[0] = 1, [5] = 2, [9] = 3};
    
    /* Nested struct initializer */
    struct ComplexStruct s2 = {
        .b = {1, 2, 3},  /* Partial init */
        .nested.x = 50,
        .u.u1 = 42
    };
    
    volatile int sink = arr[0] + s1.a + s2.nested.x;
    return s1;
}

/* Function using vector extensions for TREE_VEC */
#ifdef __VECTOR_EXTENSIONS__
void vector_operations(void) {
    /* Vector types using GCC extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = f1 * 2.0f;
    
    /* Array compound literal */
    int *p = (int[]){10, 20, 30, 40};
    
    /* More complex vector operations */
    v4si mask = {0, -1, 0, -1};
    v4si e = c & mask;
    
    volatile v4si sink_v = e;
    volatile int sink_p = p[0];
}
#endif

/* OpenMP section for OMP_CLAUSE nodes */
void openmp_test(int *array, int n) {
    int i, sum = 0;
    int private_var = 0;
    
    #pragma omp parallel for private(i) shared(array, n) reduction(+:sum) \
            schedule(dynamic, 4) num_threads(2) if(n > 100)
    for (i = 0; i < n; i++) {
        sum += array[i];
        private_var = i;  /* Private to each thread */
    }
    
    /* Another OpenMP pragma with different clauses */
    #pragma omp parallel sections private(i) firstprivate(sum) \
            lastprivate(private_var) copyin(external_var)
    {
        #pragma omp section
        {
            i = 1;
            sum += external_func(i);
        }
        
        #pragma omp section
        {
            i = 2;
            sum += external_func(i * 2);
        }
    }
    
    volatile int sink = sum + private_var;
}

/* Main test driver */
int main(void) {
    int checksum = 0;
    volatile int sink;
    
    printf("Starting tree node coverage test...\n");
    
    /* 1. Test IDENTIFIER_NODE generation */
    checksum += nested_scopes_test();
    printf("IDENTIFIER_NODE test completed\n");
    
    /* 2. Test SSA_NAME generation */
    checksum += complex_control_flow(20);
    printf("SSA_NAME test completed\n");
    
    /* 3. Test BLOCK nodes */
    checksum += block_and_goto_test();
    printf("BLOCK test completed\n");
    
    /* 4. Test CONSTRUCTOR nodes */
    struct ComplexStruct cs = constructor_test();
    checksum += cs.a + cs.b[0];
    printf("CONSTRUCTOR test completed\n");
    
    /* 5. Test vector extensions (TREE_VEC) */
    #ifdef __VECTOR_EXTENSIONS__
    vector_operations();
    printf("TREE_VEC test completed\n");
    #endif
    
    /* 6. Test OpenMP (OMP_CLAUSE) */
    #ifdef _OPENMP
    int arr[50];
    for (int i = 0; i < 50; i++) arr[i] = i;
    openmp_test(arr, 50);
    printf("OMP_CLAUSE test completed\n");
    #endif
    
    /* 7. Test C++ BINFO nodes if compiled as C++ */
    #ifdef __cplusplus
    Derived d;
    use_inheritance(&d);
    checksum += d.a + d.b;
    printf("TREE_BINFO test completed\n");
    #endif
    
    /* Additional identifier stress test */
    {
        /* Multiple extern declarations */
        extern int foo;
        extern float foo;
        extern double foo;
        
        /* Function with many parameters */
        checksum += external_func(checksum);
    }
    
    sink = checksum;
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed successfully!\n");
    
    return 0;
}
