/* test_tree.c - Comprehensive tree node coverage test */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(void);
extern void external_func2(int);
extern volatile int external_var;

/* Helper to prevent optimization */
static volatile int sink;

/* Function to accumulate checksum */
static int checksum = 0;

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
    d.a = 1;        /* Accesses through inheritance */
    d.b = 2;
    
    Base* bp = &d;
    bp->vfunc();    /* Virtual call */
    
    DeepDerived dd;
    dd.a = 3;       /* Multi-level inheritance */
    dd.b = 4;
    dd.c = 5;
    
    checksum += d.a + d.b + dd.a + dd.b + dd.c;
}
#endif

/* Complex control flow for SSA_NAME generation */
int ssa_test(int n) {
    int i, j, k, result = 0;
    
    /* Multiple variables with complex updates */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            j = i * 2;
            k = j + 1;
        } else {
            j = i / 2;
            k = j - 1;
        }
        
        /* Nested conditionals */
        if (k > 0) {
            result += k;
        } else {
            result -= j;
        }
        
        /* Loop with update in middle */
        for (int m = 0; m < 5; m++) {
            if (m == i % 3) {
                result *= 2;
            } else {
                result += m;
            }
        }
    }
    
    /* Switch with variable assignments */
    switch (n % 4) {
        case 0: result += 10; break;
        case 1: result -= 5; break;
        case 2: result *= 3; break;
        case 3: result /= 2; break;
    }
    
    return result;
}

/* Test block nodes with labels and gotos */
void block_test(void) {
    int x = 0;
    
    /* Outer block with label */
    outer_block: {
        int a = 1;
        volatile int block_var1 = a;
        
        /* Inner block 1 */
        {
            int b = 2;
            volatile int block_var2 = b;
            checksum += b;
            goto middle_block;  /* Jump forward */
        }
        
        /* Unreachable but creates block structure */
        {
            int c = 3;
            sink = c;
        }
    }
    
    middle_block: {
        int d = 4;
        volatile int block_var3 = d;
        
        /* Nested block with its own label */
        inner_label: {
            int e = 5;
            volatile int block_var4 = e;
            checksum += e;
            
            if (x < 2) {
                x++;
                goto outer_block;  /* Jump backward */
            }
        }
        
        /* Another block after goto target */
        {
            int f = 6;
            sink = f;
        }
    }
    
    /* Final block */
    {
        int g = 7;
        checksum += g;
    }
}

/* Constructor nodes with various initializers */
void constructor_test(void) {
    /* Struct with designated initializers */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Point p1 = { .x = 1, .y = 2, .z = 3 };
    struct Point p2 = { .y = 5, .z = 6, .x = 4 };  /* Out of order */
    struct Point p3 = { .x = 7 };                  /* Partial init */
    
    /* Nested struct with array */
    struct Widget {
        int id;
        struct Point location;
        int values[4];
    };
    
    struct Widget w1 = {
        .id = 100,
        .location = { .x = 10, .y = 20 },
        .values = { [0] = 1, [2] = 3, [3] = 4 }  /* Sparse array init */
    };
    
    /* Array of structs with mixed initialization */
    struct Point points[3] = {
        { .x = 1, .y = 2 },
        [2] = { .x = 5, .y = 6, .z = 7 },
        { .z = 9 }  /* For index 1 */
    };
    
    /* Union initialization */
    union Data {
        int i;
        float f;
        char str[8];
    };
    
    union Data d1 = { .i = 42 };
    union Data d2 = { .f = 3.14f };
    union Data d3 = { .str = "test" };
    
    checksum += p1.x + p2.y + p3.z + w1.id + points[2].x + d1.i;
}

/* OpenMP clauses for OMP_CLAUSE nodes */
void omp_test(int n) {
    int i, sum = 0;
    int private_var = 0;
    static int shared_var = 0;
    
    /* Multiple clauses in single pragma */
    #pragma omp parallel for private(i) shared(shared_var) reduction(+:sum) \
                schedule(dynamic, 2) num_threads(4) if(n > 1000)
    for (i = 0; i < n; i++) {
        sum += i;
        shared_var++;
    }
    
    /* Another pragma with different clauses */
    int arr[100];
    #pragma omp parallel sections private(private_var) \
                firstprivate(n) copyin(shared_var) nowait
    {
        #pragma omp section
        {
            private_var = 1;
            for (int j = 0; j < n; j++) {
                arr[j] = j * private_var;
            }
        }
        
        #pragma omp section
        {
            private_var = 2;
            for (int j = 0; j < n; j++) {
                arr[j + 50] = j * private_var;
            }
        }
    }
    
    /* Single directive with clause */
    #pragma omp single copyprivate(private_var)
    {
        private_var = sum;
    }
    
    checksum += sum + shared_var + private_var;
}

/* Vector extensions for TREE_VEC */
void vector_test(void) {
    /* Various vector types */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    /* Vector initialization and operations */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c > d;  /* Comparison generates vector mask */
    
    /* Float vector operations */
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf f3 = f1 * f2;
    
    /* Mixed size vectors */
    v8hi h1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi h2 = {8, 7, 6, 5, 4, 3, 2, 1};
    v8hi h3 = h1 + h2;
    
    /* Array compound literals (also creates TREE_VEC) */
    int *p1 = (int[]){1, 2, 3, 4, 5};
    int *p2 = (int[]){[0] = 10, [4] = 50, [2] = 30};
    
    /* Nested array in compound literal */
    int (*p3)[3] = (int[][3]){{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    
    /* Use results to prevent elimination */
    sink = c[0] + d[1] + e[2] + (int)f3[3] + h3[4];
    checksum += p1[0] + p2[4] + p3[2][2];
}

/* Deeply nested scopes for IDENTIFIER_NODE */
void identifier_test(void) {
    /* Same name in multiple scopes */
    {
        int counter = 1;
        volatile int sink1 = counter;
        
        {
            /* Shadowing variable */
            int counter = 2;
            volatile int sink2 = counter;
            
            {
                /* Another shadow */
                extern int counter;  /* External declaration */
                volatile int sink3 = counter;  /* Uses external */
                
                {
                    /* Yet another with same name */
                    int counter = 4;
                    volatile int sink4 = counter;
                    checksum += counter;
                }
            }
        }
    }
    
    /* More complex shadowing with loops */
    for (int i = 0; i < 3; i++) {
        int value = i * 10;
        
        {
            int value = i * 20;  /* Shadows outer value */
            
            for (int j = 0; j < 2; j++) {
                int value = i * 30 + j;  /* Shadows again */
                volatile int sink5 = value;
                
                {
                    /* Reference to outer scope variable */
                    extern int value;
                    volatile int sink6 = value;
                    checksum += value;
                }
            }
        }
    }
    
    /* Function parameter shadowing */
    {
        auto func = [](int x) -> int {
            {
                int x = x * 2;  /* Parameter shadowing */
                {
                    volatile int sink7 = x;
                    return x;
                }
            }
        };
        
        checksum += func(5);
    }
    
    /* Multiple extern declarations */
    {
        extern int global_var;
        {
            extern int global_var;  /* Same identifier, different node */
            volatile int sink8 = global_var;
        }
    }
}

int main(void) {
    printf("Starting tree node coverage test...\n");
    
    /* Test each tree node type */
    identifier_test();
    printf("  IDENTIFIER_NODE test complete\n");
    
    vector_test();
    printf("  TREE_VEC test complete\n");
    
    #ifdef __cplusplus
    use_inheritance();
    printf("  TREE_BINFO test complete\n");
    #endif
    
    int ssa_result = ssa_test(20);
    checksum += ssa_result;
    printf("  SSA_NAME test complete (result: %d)\n", ssa_result);
    
    block_test();
    printf("  BLOCK test complete\n");
    
    constructor_test();
    printf("  CONSTRUCTOR test complete\n");
    
    #ifdef _OPENMP
    omp_test(100);
    printf("  OMP_CLAUSE test complete\n");
    #else
    printf("  OMP_CLAUSE test skipped (no OpenMP support)\n");
    #endif
    
    /* Call external functions to create unresolved identifiers */
    if (external_var > 0) {
        external_func2(external_func1());
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy definitions to satisfy linker in standalone test */
#ifndef __cplusplus
/* In C mode, provide dummy base class simulation */
struct DummyBase {
    int base_member;
};

struct DummyDerived {
    struct DummyBase base;
    int derived_member;
};
#endif

/* External variable definitions */
int external_var = 42;
int counter = 999;
int value = 888;
int global_var = 777;

int external_func1(void) { return 1; }
void external_func2(int x) { sink = x; }
