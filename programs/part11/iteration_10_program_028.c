/* test_tree.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func(void);
extern volatile int external_var;

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

void use_inheritance(Derived *d) {
    d->a = 1;  /* Accesses through inheritance hierarchy */
    d->b = 2;
}
#endif

/* Function to force SSA_NAME creation with complex control flow */
int ssa_test(int n) {
    int i, s = 0;
    volatile int sink = 0;
    
    /* Complex loop with multiple branches for SSA */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * 2;
            if (s > 100) {
                s /= 2;
            }
        } else {
            s *= 3;
            if (i % 3 == 0) {
                s -= 5;
            }
        }
        
        /* Nested condition */
        switch (i % 4) {
            case 0: s += 1; break;
            case 1: s -= 2; break;
            case 2: s *= 1; break;
            case 3: s /= (s != 0 ? s : 1); break;
        }
    }
    
    sink = s;  /* Prevent dead code elimination */
    return s;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
void identifier_test(void) {
    volatile int result = 0;
    
    /* Level 1 - outer scope 'x' */
    {
        int x = 1;
        result += x;
        
        /* Level 2 - inner scope with different 'x' */
        {
            extern int x;  /* Unresolved external */
            volatile int y = x;  /* Forces different identifier node */
            result += y;
            
            /* Level 3 - another 'x' */
            {
                static int x = 3;
                volatile int z = x;
                result += z;
                
                /* Level 4 - yet another 'x' in loop */
                for (int x = 0; x < 2; x++) {
                    volatile int w = x;
                    result += w;
                    
                    /* Level 5 - parameter shadowing */
                    auto shadow_func = [](int x) -> int {
                        volatile int inner = x;
                        return inner * 2;
                    };
                    result += shadow_func(x);
                }
            }
        }
    }
    
    /* More identifier variations */
    {
        /* Same name in different contexts */
        struct x { int a; };
        enum x_enum { X_VALUE };
        
        volatile struct x x_struct = { .a = 10 };
        volatile enum x_enum x_enum_val = X_VALUE;
        
        result += x_struct.a + x_enum_val;
    }
}

/* Function for TREE_VEC nodes using GCC vector extensions */
void vector_test(void) {
    /* Vector types with different sizes */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v8sf __attribute__((vector_size(32)));
    typedef short v16hi __attribute__((vector_size(32)));
    
    /* Vector operations */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c - d;
    
    /* Compound literal in array initializer */
    int *arr = (int[]){e[0], e[1], e[2], e[3], 9, 10};
    volatile int *volatile_arr = arr;
    
    /* More complex vector operations */
    v8sf f1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8sf f2 = f1 * 2.0f;
    v8sf f3 = f1 + f2;
    
    /* Vector comparison */
    v4si mask = a > b;
    v4si masked = a & mask;
    
    /* Prevent optimization */
    volatile v4si sink1 = c;
    volatile v8sf sink2 = f3;
    volatile v4si sink3 = masked;
}

/* Function with complex BLOCK structure */
void block_test(void) {
    volatile int counter = 0;
    
    /* Outer block with label */
    outer_block: {
        int a = 1;
        counter += a;
        
        /* Inner block 1 */
        {
            int b = 2;
            counter += b;
            goto middle_block;  /* Jump forward */
            
            /* Unreachable but creates BLOCK structure */
            {
                int unreachable = 99;
                counter += unreachable;
            }
        }
        
        /* This should be skipped by goto */
        counter += 999;
    }
    
    /* Middle block */
    middle_block: {
        int c = 3;
        counter += c;
        
        /* Deeply nested blocks */
        {
            int d = 4;
            {
                int e = 5;
                {
                    int f = 6;
                    counter += d + e + f;
                    goto inner_label;
                }
            }
        }
        
        goto end_block;
    }
    
    /* Labeled inner block */
    inner_label: {
        int g = 7;
        counter += g;
        goto outer_block;  /* Jump back */
    }
    
    end_block: {
        int h = 8;
        counter += h;
    }
    
    /* Switch with blocks */
    switch (counter % 3) {
        case 0: {
            int case_var = 100;
            counter += case_var;
            break;
        }
        case 1: {
            int case_var = 200;  /* Same name, different scope */
            counter += case_var;
            break;
        }
        case 2: {
            int case_var = 300;
            counter += case_var;
            break;
        }
    }
}

/* Function for CONSTRUCTOR nodes */
void constructor_test(void) {
    /* Struct with designated initializers */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Nested {
        struct Point p;
        int id;
        float data[4];
    };
    
    /* Complex designated initializers */
    struct Point p1 = { .x = 1, .y = 2, .z = 3 };
    struct Point p2 = { .y = 20, .z = 30, .x = 10 };
    struct Point p3 = { .x = 100 };  /* Partial initialization */
    
    /* Nested designated initializers */
    struct Nested n1 = {
        .p = { .x = 1, .y = 2, .z = 3 },
        .id = 42,
        .data = { [0] = 1.0f, [2] = 3.0f }  /* Sparse array init */
    };
    
    /* Array with designated initializers */
    int sparse_array[10] = { [0] = 1, [5] = 2, [9] = 3 };
    
    /* Union initializers */
    union Data {
        int i;
        float f;
        char str[4];
    };
    
    union Data u1 = { .i = 42 };
    union Data u2 = { .f = 3.14f };
    union Data u3 = { .str = "ABC" };
    
    /* Complex nested initializer */
    struct Complex {
        struct {
            int a;
            int b;
        } inner;
        int arr[3][2];
    };
    
    struct Complex comp = {
        .inner = { .a = 1, .b = 2 },
        .arr = { { [0] = 1, [1] = 2 }, { 3, 4 }, { [1] = 5 } }
    };
    
    /* Prevent optimization */
    volatile struct Point vp1 = p1;
    volatile struct Nested vn1 = n1;
    volatile union Data vu1 = u1;
    volatile struct Complex vcomp = comp;
}

/* OpenMP test for OMP_CLAUSE nodes */
void omp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    static int shared_var = 0;
    
    /* Test various OpenMP clauses */
    #pragma omp parallel for private(i) shared(sum, shared_var) \
            reduction(+:sum) schedule(dynamic, 2) \
            num_threads(4) if(n > 1000)
    for (i = 0; i < n; i++) {
        int local_sum = 0;
        #pragma omp simd reduction(+:local_sum) \
                linear(i:1) safelen(8)
        for (int j = 0; j < 100; j++) {
            local_sum += i * j;
        }
        sum += local_sum;
        shared_var++;
    }
    
    /* More OpenMP constructs */
    #pragma omp parallel private(private_var) \
            firstprivate(sum) copyin(shared_var) \
            default(none)
    {
        private_var = omp_get_thread_num();
        #pragma omp critical
        {
            shared_var += private_var;
        }
        
        #pragma omp barrier
        
        #pragma omp single copyprivate(private_var)
        {
            private_var = 42;
        }
        
        #pragma omp sections lastprivate(private_var)
        {
            #pragma omp section
            {
                private_var = 1;
            }
            #pragma omp section
            {
                private_var = 2;
            }
        }
    }
    
    /* Task with dependencies */
    #pragma omp parallel
    #pragma omp single
    {
        int task_var = 0;
        #pragma omp task depend(out: task_var) priority(1)
        {
            task_var = 100;
        }
        
        #pragma omp task depend(in: task_var) priority(2)
        {
            sum += task_var;
        }
        
        #pragma omp taskwait
    }
    
    volatile int sink = sum + shared_var;
}

/* Main test driver */
int main(void) {
    volatile int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* 1. Test IDENTIFIER_NODE generation */
    printf("Testing IDENTIFIER_NODE...\n");
    identifier_test();
    checksum += 1;
    
    /* 2. Test TREE_VEC generation */
    printf("Testing TREE_VEC...\n");
    vector_test();
    checksum += 2;
    
    /* 3. Test SSA_NAME generation */
    printf("Testing SSA_NAME...\n");
    checksum += ssa_test(50);
    
    /* 4. Test BLOCK generation */
    printf("Testing BLOCK...\n");
    block_test();
    checksum += 4;
    
    /* 5. Test CONSTRUCTOR generation */
    printf("Testing CONSTRUCTOR...\n");
    constructor_test();
    checksum += 8;
    
    /* 6. Test OpenMP clauses */
    printf("Testing OMP_CLAUSE...\n");
    #ifdef _OPENMP
    omp_test(100);
    checksum += 16;
    #else
    printf("OpenMP not enabled, skipping OMP_CLAUSE test\n");
    #endif
    
    /* 7. Test C++ BINFO if in C++ mode */
    #ifdef __cplusplus
    printf("Testing TREE_BINFO (C++ mode)...\n");
    Derived d;
    use_inheritance(&d);
    checksum += 32;
    
    /* Multiple inheritance scenario */
    struct MIBase1 { int a; virtual void f1() {} };
    struct MIBase2 { int b; virtual void f2() {} };
    struct MIderived : MIBase1, MIBase2 { int c; };
    
    MIderived mi;
    mi.a = 1;
    mi.b = 2;
    mi.c = 3;
    #endif
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum & 0xFF;  /* Return non-zero to indicate success */
}

/* Additional functions to create more tree nodes */
void extra_identifier_scopes(void) {
    /* Function with many parameters */
    auto multi_param = [](int a, int b, int c, int d, int e, 
                         int f, int g, int h, int i, int j) {
        return a + b + c + d + e + f + g + h + i + j;
    };
    
    volatile int result = multi_param(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    
    /* Template (C++ only) for additional nodes */
    #ifdef __cplusplus
    template<typename T>
    T template_func(T x) {
        volatile T y = x;
        return y * 2;
    }
    
    volatile int t_result = template_func(42);
    #endif
}
