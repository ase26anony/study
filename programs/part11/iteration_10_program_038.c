/* test_tree.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(void);
extern double external_func2(int);
extern void external_func3(char*);

/* Volatile sink to prevent optimization */
volatile int volatile_sink = 0;

#ifdef __cplusplus
/* C++ specific code for BINFO nodes */
struct Base {
    int base_data;
    virtual void base_method() {}
};

struct Derived : Base {
    int derived_data;
    void base_method() override {}
};

struct DeepDerived : Derived {
    int deep_data;
};

void use_inheritance() {
    DeepDerived dd;
    dd.base_data = 10;      /* Accesses through inheritance chain */
    dd.derived_data = 20;
    dd.deep_data = 30;
    volatile_sink = dd.base_data + dd.derived_data + dd.deep_data;
}
#endif

/* Function to create complex control flow for SSA_NAME */
int ssa_test(int n) {
    int i, j, k = 0;
    int result = 0;
    
    /* Complex control flow with multiple assignments */
    for (i = 0; i < n; i++) {
        if (i % 3 == 0) {
            j = i * 2;
            result += j;
        } else if (i % 3 == 1) {
            j = i / 2;
            result -= j;
        } else {
            j = i + 1;
            result *= (j % 5);
        }
        
        /* Nested loop with variable updates */
        for (k = 0; k < (i % 4); k++) {
            if (k % 2 == 0) {
                result += k * i;
            } else {
                result -= k * i;
            }
        }
    }
    
    /* Switch with variable modifications */
    switch (n % 4) {
        case 0: result = result * 2; break;
        case 1: result = result + 100; break;
        case 2: result = result - 50; break;
        case 3: result = result / 3; break;
    }
    
    return result;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
void identifier_test(void) {
    int checksum = 0;
    
    /* Level 1 scope */
    {
        int x = 1;
        volatile int y = x;
        checksum += x;
        
        /* Level 2 scope */
        {
            /* Different x in inner scope */
            float x = 2.5f;
            volatile float z = x;
            checksum += (int)x;
            
            /* Level 3 scope */
            {
                /* Another x */
                char x = 'A';
                volatile char w = x;
                checksum += x;
                
                /* Level 4 scope with extern declaration */
                {
                    /* Try to reference outer x (will create identifier node) */
                    extern int x;  /* Unresolved external */
                    volatile int v = 0;
                    if (v) v = x;  /* Reference to create identifier node */
                }
            }
        }
    }
    
    /* More shadowing with same name in different contexts */
    for (int i = 0; i < 3; i++) {
        /* i is loop variable */
        int i = i * 2;  /* New i shadows loop i */
        checksum += i;
        
        {
            double i = 3.14 * i;
            checksum += (int)i;
        }
    }
    
    volatile_sink = checksum;
}

/* Function using vector extensions for TREE_VEC */
void vector_test(void) {
    /* Vector types using GCC extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c - d;
    
    v4sf f = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf g = f * 2.0f;
    
    v8hi h = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi i = h << 1;
    
    /* Array compound literals */
    int *arr1 = (int[]){1, 2, 3, 4, 5};
    int *arr2 = (int[]){[0]=10, [2]=20, [4]=30};
    int *arr3 = (int[]){[0 ... 4]=0, [2]=42};
    
    /* Multi-dimensional array literal */
    int (*md_arr)[3] = (int[][3]){{1,2,3}, {4,5,6}, {7,8,9}};
    
    volatile_sink = c[0] + d[1] + e[2] + (int)g[3] + i[4] + 
                    arr1[0] + arr2[2] + arr3[2] + md_arr[1][1];
}

/* Function with complex blocks and labels for BLOCK nodes */
void block_test(void) {
    int checksum = 0;
    
    /* Outer block with label */
    outer_block: {
        int a = 1;
        checksum += a;
        
        /* Inner block 1 */
        {
            int b = 2;
            checksum += b;
            goto middle_block;  /* Jump forward */
        }
        
        /* Unreachable code creates separate block */
        {
            int c = 3;
            checksum += c;
        }
        
        middle_block: {
            int d = 4;
            checksum += d;
            
            /* Nested block with its own label */
            inner_block: {
                int e = 5;
                checksum += e;
                goto outer_block_end;  /* Jump out */
            }
            
            /* Another unreachable block */
            {
                int f = 6;
                checksum += f;
            }
        }
    }
    
    outer_block_end: {
        int g = 7;
        checksum += g;
        
        /* Switch with cases as blocks */
        switch (checksum % 3) {
            case 0: {
                int h = 8;
                checksum += h;
                break;
            }
            case 1: {
                int i = 9;
                checksum += i;
                break;
            }
            case 2: {
                int j = 10;
                checksum += j;
                break;
            }
        }
    }
    
    volatile_sink = checksum;
}

/* Function with various constructors for CONSTRUCTOR nodes */
void constructor_test(void) {
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
        .p1 = { .x = 1, .y = 2, .z = 3 },
        .p2 = { .x = 4, .y = 5, .z = 6 },
        .id = 1
    };
    
    struct Rect r2 = { 
        .p2.y = 50,  /* Nested designated */
        .id = 2,
        .p1 = { .x = 10, .z = 30 }
    };
    
    /* Array with designated initializers */
    int arr1[10] = { [0] = 1, [5] = 2, [9] = 3 };
    int arr2[10] = { [0 ... 4] = 10, [5 ... 9] = 20 };
    int arr3[5][3] = { [0][0] = 1, [1][1] = 2, [2][2] = 3 };
    
    /* Union initializers */
    union Data {
        int i;
        float f;
        char str[20];
    };
    
    union Data d1 = { .i = 42 };
    union Data d2 = { .f = 3.14f };
    union Data d3 = { .str = "Hello" };
    
    /* Mixed initializers */
    struct Mixed {
        int a;
        int b[3];
        struct Point p;
    } m1 = { .a = 1, .b = {[1] = 5}, .p = { .x = 10, .y = 20 } };
    
    volatile_sink = p1.x + p2.y + p3.z + r1.id + r2.id + 
                    arr1[5] + arr2[9] + arr3[1][1] + d1.i + m1.b[1];
}

/* OpenMP test for OMP_CLAUSE nodes */
void omp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    int shared_var = 0;
    int reduction_sum = 0;
    int last_iter = 0;
    
    /* Test 1: Parallel for with multiple clauses */
    #pragma omp parallel for private(i, private_var) shared(shared_var) \
             reduction(+:reduction_sum) schedule(dynamic, 4) \
             lastprivate(last_iter)
    for (i = 0; i < n; i++) {
        private_var = i * 2;
        reduction_sum += private_var;
        last_iter = i;
        #pragma omp atomic
        shared_var++;
    }
    
    /* Test 2: Parallel sections */
    #pragma omp parallel sections private(i) \
             num_threads(2) if(n > 10)
    {
        #pragma omp section
        {
            for (i = 0; i < n/2; i++) {
                #pragma omp atomic
                sum += i;
            }
        }
        
        #pragma omp section
        {
            for (i = n/2; i < n; i++) {
                #pragma omp atomic
                sum += i * 2;
            }
        }
    }
    
    /* Test 3: Critical with name */
    #pragma omp parallel
    {
        #pragma omp critical (my_critical)
        {
            sum += 1;
        }
        
        /* Test 4: Barrier */
        #pragma omp barrier
        
        /* Test 5: Single with copyprivate */
        #pragma omp single copyprivate(private_var)
        {
            private_var = 42;
        }
    }
    
    /* Test 6: Task with dependencies */
    int task_var = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: task_var)
        {
            task_var = 100;
        }
        
        #pragma omp task depend(in: task_var)
        {
            sum += task_var;
        }
    }
    
    volatile_sink = sum + reduction_sum + shared_var + last_iter + private_var;
}

/* Main test driver */
int main(void) {
    int total_checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* 1. Test IDENTIFIER_NODE generation */
    printf("Testing IDENTIFIER_NODE...\n");
    identifier_test();
    total_checksum += volatile_sink;
    
    /* 2. Test TREE_VEC generation */
    printf("Testing TREE_VEC...\n");
    vector_test();
    total_checksum += volatile_sink;
    
    /* 3. Test SSA_NAME generation (with optimization) */
    printf("Testing SSA_NAME...\n");
    volatile_sink = ssa_test(20);
    total_checksum += volatile_sink;
    
    /* 4. Test BLOCK generation */
    printf("Testing BLOCK...\n");
    block_test();
    total_checksum += volatile_sink;
    
    /* 5. Test CONSTRUCTOR generation */
    printf("Testing CONSTRUCTOR...\n");
    constructor_test();
    total_checksum += volatile_sink;
    
    /* 6. Test OMP_CLAUSE generation */
    printf("Testing OMP_CLAUSE...\n");
    omp_test(50);
    total_checksum += volatile_sink;
    
    /* 7. Test C++ BINFO generation if in C++ mode */
    #ifdef __cplusplus
    printf("Testing TREE_BINFO (C++ only)...\n");
    use_inheritance();
    total_checksum += volatile_sink;
    #endif
    
    /* Call external functions to create unresolved identifiers */
    printf("Creating unresolved identifiers...\n");
    if (total_checksum > 0) {
        /* These create IDENTIFIER_NODEs for external symbols */
        external_func1();
        external_func2(total_checksum);
        external_func3("test");
    }
    
    printf("Total checksum: %d\n", total_checksum);
    printf("Test completed.\n");
    
    return total_checksum != 0 ? 0 : 1;
}
