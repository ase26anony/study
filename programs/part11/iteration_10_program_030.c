/* test_tree.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func(int);
extern volatile int external_var;

/* Helper to prevent optimization */
static volatile int sink;

#ifdef __cplusplus
/* C++ specific code for BINFO nodes */
struct Base {
    int a;
    virtual void foo() {}
};

struct Derived : Base {
    int b;
    void foo() override {}
};

struct DeepDerived : Derived {
    int c;
    void foo() override {}
};

void use_hierarchy() {
    DeepDerived dd;
    dd.a = 1;      /* Accesses through inheritance chain */
    dd.b = 2;
    dd.c = 3;
    Base* bp = &dd;
    bp->foo();     /* Virtual call */
    sink = dd.a + dd.b + dd.c;
}
#endif

/* Function to create SSA_NAME nodes with complex control flow */
int create_ssa_names(int n) {
    int i, j, k, result = 0;
    
    /* Complex loop with multiple branches */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            result += i * 2;
            for (j = 0; j < i; j++) {
                if (j % 3 == 0) {
                    result -= j;
                } else {
                    result += j * 3;
                }
                /* Nested condition */
                k = (j > 5) ? j * 2 : j / 2;
                result ^= k;
            }
        } else {
            result *= 2;
            /* Switch-like logic */
            switch (i % 4) {
                case 0: result += 1; break;
                case 1: result -= 2; break;
                case 2: result *= 3; break;
                case 3: result /= 2; break;
            }
        }
        
        /* Phi node creation */
        int temp = result;
        if (temp > 1000) {
            temp = temp % 1000;
        } else {
            temp = temp * 2;
        }
        result = temp;
    }
    
    return result;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
void nested_scopes() {
    /* Level 1 */
    int x = 1;
    sink = x;
    
    {
        /* Level 2 - shadowing */
        volatile int x = 2;
        sink = x;
        
        {
            /* Level 3 - different type */
            extern int x;  /* Unresolved identifier */
            volatile int y = x + 1;
            sink = y;
            
            {
                /* Level 4 - in loop */
                for (int x = 0; x < 3; x++) {
                    volatile int z = x * 2;
                    sink = z;
                    {
                        /* Level 5 - in conditional */
                        if (z > 0) {
                            double x = 3.14;  /* Different type */
                            sink = (int)x;
                        }
                    }
                }
            }
        }
    }
    
    /* Function parameter shadowing */
    auto lambda = [](int x) -> int {
        volatile int y = x * 2;
        {
            long x = y + 1;  /* Shadow parameter */
            return (int)x;
        }
    };
    
    sink = lambda(5);
}

/* Function to create BLOCK nodes with labels and gotos */
void create_blocks() {
    int a = 0;
    
    /* Block 1 */
    {
        int b = 1;
    block1_label:
        a += b;
        if (a < 10) {
            goto block2_label;
        }
    }
    
    /* Block 2 */
    {
        int c = 2;
    block2_label:
        a += c;
        {
            /* Nested block with label */
            int d = 3;
        inner_block_label:
            a += d;
            if (a < 20) {
                goto block1_label;
            } else {
                goto final_label;
            }
        }
    }
    
final_label:
    sink = a;
}

/* Function to create CONSTRUCTOR nodes */
void create_constructors() {
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
    
    /* Complex designated initializer */
    struct Rect r1 = {
        .p1 = { .x = 1, .y = 2, .z = 3 },
        .p2 = { .x = 4, .y = 5 },
        .id = 100
    };
    
    /* Partial array initialization */
    int arr[10] = { [0] = 1, [5] = 2, [9] = 3 };
    
    /* Nested struct with arrays */
    struct Nested {
        int a;
        struct {
            int b[3];
            int c;
        } inner;
        int d[2][2];
    };
    
    struct Nested n = {
        .a = 1,
        .inner = {
            .b = {[1] = 5, [2] = 6},
            .c = 7
        },
        .d = {{1, 2}, {3, 4}}
    };
    
    /* Union initializer */
    union Data {
        int i;
        float f;
        char str[20];
    };
    
    union Data u = { .f = 3.14f };
    
    sink = r1.p1.x + arr[5] + n.inner.b[1] + (int)u.f;
}

/* Function using vector extensions for TREE_VEC */
void use_vectors() {
    /* Vector types using GCC extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    /* Vector initialization */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Vector operations */
    v4si c = a + b;
    v4si d = a * b;
    v4si e = a & b;
    
    /* Float vectors */
    v4sf f = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf g = f * 2.0f;
    
    /* Array compound literals */
    int *p = (int[]){1, 2, 3, 4, 5};
    int *q = (int[3]){10, 20, 30};
    
    /* Vector comparisons */
    v4si mask = a > b;
    v4si h = __builtin_shuffle(a, b, (v4si){0, 4, 1, 5});
    
    /* Use results to prevent optimization */
    sink = c[0] + d[1] + e[2] + (int)g[3] + p[2] + q[1] + mask[0] + h[3];
}

/* OpenMP function for OMP_CLAUSE nodes */
void openmp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    int shared_var = 0;
    
    /* Multiple OpenMP pragmas with various clauses */
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel private(i) shared(sum, shared_var) \
                         firstprivate(private_var) if(n > 1000)
    {
        #pragma omp for reduction(+:sum) schedule(dynamic, 2) \
                         collapse(1) nowait
        for (i = 0; i < n; i++) {
            sum += i;
            shared_var++;
        }
        
        /* Barrier with clause */
        #pragma omp barrier
        
        /* Single with copyprivate */
        #pragma omp single copyprivate(private_var)
        {
            private_var = omp_get_thread_num();
        }
        
        /* Critical section */
        #pragma omp critical
        {
            shared_var += private_var;
        }
    }
    
    /* Parallel sections */
    #pragma omp parallel sections private(i) shared(sum)
    {
        #pragma omp section
        {
            for (i = 0; i < 100; i++) {
                sum += i * 2;
            }
        }
        
        #pragma omp section
        {
            for (i = 0; i < 100; i++) {
                sum -= i;
            }
        }
    }
    
    /* Task construct */
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (i = 0; i < 10; i++) {
                #pragma omp task depend(in: sum) depend(out: shared_var) \
                                 priority(i % 3)
                {
                    sum += i;
                    shared_var = sum;
                }
            }
        }
    }
    
    sink = sum + shared_var;
}

/* Main test driver */
int main() {
    int checksum = 0;
    
    printf("Testing GCC tree node coverage...\n");
    
    /* 1. Test IDENTIFIER_NODE creation */
    printf("1. Testing IDENTIFIER_NODE...\n");
    nested_scopes();
    checksum += sink;
    
    /* 2. Test TREE_VEC creation */
    printf("2. Testing TREE_VEC...\n");
    use_vectors();
    checksum += sink;
    
    /* 3. Test BLOCK nodes */
    printf("3. Testing BLOCK nodes...\n");
    create_blocks();
    checksum += sink;
    
    /* 4. Test CONSTRUCTOR nodes */
    printf("4. Testing CONSTRUCTOR nodes...\n");
    create_constructors();
    checksum += sink;
    
    /* 5. Test SSA_NAME creation */
    printf("5. Testing SSA_NAME...\n");
    int ssa_result = create_ssa_names(50);
    checksum += ssa_result;
    
    /* 6. Test OpenMP clauses */
    printf("6. Testing OMP_CLAUSE...\n");
    openmp_test(100);
    checksum += sink;
    
#ifdef __cplusplus
    /* 7. Test C++ BINFO nodes */
    printf("7. Testing TREE_BINFO (C++ only)...\n");
    use_hierarchy();
    checksum += sink;
#endif
    
    /* Use external identifiers */
    checksum += external_var;
    checksum += external_func(checksum);
    
    printf("Final checksum: %d\n", checksum);
    printf("Compile with:\n");
    printf("  C mode: gcc -O1 -fopenmp -fdump-tree-all test_tree.c\n");
    printf("  C++ mode: g++ -O1 -fopenmp -fdump-tree-all test_tree.c\n");
    
    return checksum != 0 ? 0 : 1;
}
