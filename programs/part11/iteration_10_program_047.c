/* test_tree.c - Comprehensive tree node coverage test */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(void);
extern void external_func2(int);
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
    void vfunc() override {}
};

void use_inheritance() {
    Derived d;
    d.a = 1;  /* Accesses base member - creates BINFO */
    d.b = 2;
    
    DeepDerived dd;
    dd.a = 3;  /* Accesses through deeper hierarchy */
    dd.b = 4;
    dd.c = 5;
    
    Base* bp = &d;
    bp->vfunc();  /* Virtual call */
}
#endif

/* Function to create SSA_NAME nodes with complex control flow */
int create_ssa_names(int n) {
    int i, j, k, result = 0;
    volatile int sink;
    
    /* Complex loop with multiple branches */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            for (j = 0; j < i; j++) {
                if (j % 3 == 0) {
                    result += j * 2;
                } else if (j % 3 == 1) {
                    result -= j;
                } else {
                    result ^= j;
                }
            }
        } else {
            int temp = i * 3;
            while (temp > 0) {
                result += temp % 2;
                temp >>= 1;
            }
        }
        
        /* Switch with multiple cases */
        switch (i % 4) {
            case 0: k = result + i; break;
            case 1: k = result - i; break;
            case 2: k = result * i; break;
            case 3: k = result ^ i; break;
        }
        result = k;
    }
    
    sink = result;
    return result;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
int nested_scopes_identifiers(void) {
    volatile int result = 0;
    
    /* Level 1 scope */
    {
        int x = 1;
        result += x;
        
        /* Level 2 scope */
        {
            int x = 2;  /* Same name, different scope */
            result += x;
            
            /* Level 3 scope with extern declaration */
            {
                extern int x;  /* Unresolved identifier */
                volatile int y = (int)&x;  /* Force use */
                result += y & 0xFF;
                
                /* Level 4 scope */
                {
                    int x = 4;
                    for (int x = 0; x < 3; x++) {  /* Another x in for loop scope */
                        result += x;
                    }
                    result += x;
                }
            }
        }
        
        /* Another sibling scope */
        {
            int x = 5;
            result += x;
            
            /* goto creating complex control flow */
            goto label1;
            
            {
                int unused = 10;
            }
            
            label1:
            result += x * 2;
        }
    }
    
    /* Function scope with parameter shadowing */
    auto func = [](int x) -> int {
        {
            int x = x * 2;  /* Parameter shadowing */
            return x;
        }
    };
    
    result += func(10);
    return result;
}

/* Function to create BLOCK nodes with labels and gotos */
int create_blocks(void) {
    volatile int a = 0;
    
    /* Block 1 */
    {
        int x = 1;
        a += x;
        goto block2;  /* Jump forward */
        
        /* Dead code block */
        {
            int dead = 99;
            a += dead;
        }
    }
    
    /* Block 2 */
    block2: {
        int y = 2;
        a += y;
        
        /* Nested block with label */
        {
            int z = 3;
            inner_label:
            a += z;
            goto block3;
        }
    }
    
    /* Block 3 */
    block3: {
        int w = 4;
        a += w;
        goto inner_label;  /* Jump backward */
    }
    
    return a;
}

/* Function to create CONSTRUCTOR nodes */
int create_constructors(void) {
    /* Struct with designated initializers */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Nested {
        struct Point p;
        int arr[4];
        union {
            int i;
            float f;
        } u;
    };
    
    /* Complex designated initializers */
    struct Point p1 = { .x = 1, .y = 2, .z = 3 };
    struct Point p2 = { .y = 5, .z = 6 };  /* Partial initialization */
    
    /* Nested designated initializers */
    struct Nested n1 = {
        .p = { .x = 1, .y = 2 },
        .arr = { [0] = 10, [2] = 20, [3] = 30 },  /* Sparse array init */
        .u = { .f = 3.14f }
    };
    
    /* Array of structs with designated init */
    struct Point points[3] = {
        [0] = { .x = 1, .y = 2 },
        [2] = { .z = 9 }
    };
    
    /* Union initialization */
    union Data {
        int i;
        float f;
        char str[4];
    } data = { .str = "abc" };
    
    volatile int result = p1.x + p2.y + n1.arr[2] + points[0].y + data.str[0];
    return result;
}

/* Function using GCC vector extensions for TREE_VEC */
int create_tree_vec(void) {
    /* Vector type definitions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    /* Vector operations */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c > d;  /* Comparison generates vector */
    
    /* Float vector operations */
    v4sf f = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf g = f * 2.0f;
    
    /* Mixed operations */
    v8hi h = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi i = h << 1;
    
    /* Array compound literals */
    int *arr1 = (int[]){1, 2, 3, 4, 5};
    int *arr2 = (int[3]){10, 20};  /* Partial init */
    
    /* Nested compound literals */
    struct VecHolder {
        v4si vec;
        int count;
    };
    
    struct VecHolder vh = {
        .vec = (v4si){9, 8, 7, 6},
        .count = 4
    };
    
    volatile int result = c[0] + e[1] + (int)g[2] + i[3] + arr1[2] + arr2[1] + vh.vec[0];
    return result;
}

/* OpenMP function for OMP_CLAUSE nodes */
#ifdef _OPENMP
#include <omp.h>
int create_omp_clauses(int n) {
    int i, sum = 0;
    int private_var = 0;
    static int shared_var = 0;
    int reduction_sum = 0;
    int last_iter = 0;
    
    /* Test 1: Parallel region with multiple clauses */
    #pragma omp parallel private(private_var) shared(shared_var) \
                         firstprivate(n) if(n > 100) num_threads(2)
    {
        private_var = omp_get_thread_num();
        #pragma omp atomic
        shared_var++;
    }
    
    /* Test 2: Parallel for with reduction and schedule */
    #pragma omp parallel for reduction(+:reduction_sum) \
                         schedule(dynamic, 4) collapse(2) \
                         ordered private(i)
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            #pragma omp ordered
            reduction_sum += x * y;
        }
    }
    
    /* Test 3: Sections with nowait */
    #pragma omp parallel sections private(i) nowait
    {
        #pragma omp section
        {
            for (i = 0; i < n; i++) {
                #pragma omp atomic
                sum += i;
            }
        }
        
        #pragma omp section
        {
            for (i = n; i > 0; i--) {
                #pragma omp atomic
                sum -= i;
            }
        }
    }
    
    /* Test 4: Single with copyprivate */
    #pragma omp parallel private(private_var)
    {
        #pragma omp single copyprivate(private_var)
        {
            private_var = 42;
        }
        // private_var should be 42 in all threads
        #pragma omp atomic
        sum += private_var;
    }
    
    /* Test 5: Task with depend clauses */
    int task_a = 0, task_b = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: task_a)
        { task_a = 1; }
        
        #pragma omp task depend(out: task_b)
        { task_b = 2; }
        
        #pragma omp task depend(in: task_a, task_b)
        { sum += task_a + task_b; }
    }
    
    volatile int result = sum + shared_var + reduction_sum;
    return result;
}
#else
int create_omp_clauses(int n) {
    /* Dummy implementation when OpenMP not available */
    return n * 2;
}
#endif

/* Main test driver */
int main(void) {
    volatile int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* 1. Test IDENTIFIER_NODE creation */
    checksum += nested_scopes_identifiers();
    printf("IDENTIFIER_NODE test completed\n");
    
    /* 2. Test TREE_VEC creation */
    checksum += create_tree_vec();
    printf("TREE_VEC test completed\n");
    
    #ifdef __cplusplus
    /* 3. Test TREE_BINFO creation (C++ only) */
    use_inheritance();
    checksum += 1000;  /* Arbitrary value */
    printf("TREE_BINFO test completed\n");
    #endif
    
    /* 4. Test SSA_NAME creation */
    checksum += create_ssa_names(50);
    printf("SSA_NAME test completed\n");
    
    /* 5. Test BLOCK creation */
    checksum += create_blocks();
    printf("BLOCK test completed\n");
    
    /* 6. Test CONSTRUCTOR creation */
    checksum += create_constructors();
    printf("CONSTRUCTOR test completed\n");
    
    /* 7. Test OMP_CLAUSE creation */
    checksum += create_omp_clauses(100);
    printf("OMP_CLAUSE test completed\n");
    
    /* Use external identifiers */
    checksum += (int)&external_func1;
    checksum += (int)&external_func2;
    checksum += external_var;
    
    printf("Final checksum: %d\n", checksum);
    
    /* Prevent dead code elimination */
    volatile int* volatile_sink = &checksum;
    asm volatile("" : : "r"(volatile_sink) : "memory");
    
    return checksum != 0 ? 0 : 1;
}
