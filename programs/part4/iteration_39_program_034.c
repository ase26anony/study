/* test_tree_coverage.c - Comprehensive test to trigger specific tree node codes */

/* Prevent excessive inlining */
#define NOINLINE __attribute__((noinline))

/* Generate many unique identifiers */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

/* Vector types for TREE_VEC */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Struct for CONSTRUCTOR */
struct ComplexStruct {
    int a;
    float b;
    double c;
    char d;
};

/* Another struct for more constructor complexity */
struct NestedStruct {
    struct ComplexStruct inner;
    int arr[3];
};

/* Global variables to prevent optimization */
volatile int global_volatile = 0;
int global_result = 0;

/* Function declarations to ensure tree nodes are created */
NOINLINE int func1(int x);
NOINLINE float func2(float x);
NOINLINE double func3(double x);
NOINLINE int test_identifiers_and_blocks(void);
NOINLINE int test_vectors_and_ssa(void);
NOINLINE int test_aggregate_init(void);
NOINLINE int test_omp_clauses(void);

/* Many functions with unique identifiers for IDENTIFIER_NODE */
NOINLINE int MAKE_ID(0)(void) { return 1; }
NOINLINE int MAKE_ID(1)(void) { return 2; }
NOINLINE int MAKE_ID(2)(void) { return 3; }
NOINLINE int MAKE_ID(3)(void) { return 4; }
NOINLINE int MAKE_ID(4)(void) { return 5; }
NOINLINE int MAKE_ID(5)(void) { return 6; }
NOINLINE int MAKE_ID(6)(void) { return 7; }
NOINLINE int MAKE_ID(7)(void) { return 8; }
NOINLINE int MAKE_ID(8)(void) { return 9; }
NOINLINE int MAKE_ID(9)(void) { return 10; }

/* Complex function with many identifiers and nested blocks */
NOINLINE int test_identifiers_and_blocks(void) {
    /* Outer block */
    int outer_var = MAKE_ID(0)();
    
    {
        /* First inner block */
        int inner_var_1 = MAKE_ID(1)();
        volatile int block_local_1 = inner_var_1 + outer_var;
        
        {
            /* Deeply nested block */
            int deep_var = MAKE_ID(2)();
            volatile int block_local_2 = deep_var * 2;
            
            /* Memory barrier to prevent reordering */
            asm volatile("" : : : "memory");
        }
    }
    
    {
        /* Another independent block */
        float float_var = MAKE_ID(3)() * 1.5f;
        volatile float block_local_3 = float_var;
        
        if (global_volatile) {
            /* Block inside if statement */
            int conditional_var = MAKE_ID(4)();
            volatile int block_local_4 = conditional_var;
        } else {
            /* Block inside else statement */
            double double_var = MAKE_ID(5)();
            volatile double block_local_5 = double_var;
        }
    }
    
    /* Loop with block */
    for (int i = 0; i < 3; i++) {
        /* Block inside loop */
        int loop_var = MAKE_ID(6)() + i;
        volatile int block_local_6 = loop_var;
        
        {
            /* Extra nested block in loop */
            char char_var = MAKE_ID(7)() + i;
            volatile char block_local_7 = char_var;
        }
    }
    
    return outer_var;
}

/* Function to generate SSA_NAME and TREE_VEC nodes */
NOINLINE int test_vectors_and_ssa(void) {
    /* Vector operations for TREE_VEC */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = vec_a + vec_b;
    v4si vec_d = vec_a * vec_b;
    
    /* Mixed vector types */
    v4sf vec_f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_f2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec_f3 = vec_f1 + vec_f2;
    
    /* Complex SSA pattern with loop and conditionals */
    int ssa_var = 0;
    int result = 0;
    
    /* This creates complex SSA form */
    for (int i = 0; i < 10; i++) {
        volatile int loop_control = global_volatile;
        
        if (loop_control > 0) {
            ssa_var = i * 2;  /* SSA phi node will be created here */
        } else {
            ssa_var = i + 3;  /* Another SSA phi node source */
        }
        
        /* Another conditional inside loop */
        if (i % 2 == 0) {
            ssa_var += vec_a[i % 4];  /* Using vector element */
        } else {
            ssa_var -= vec_b[i % 4];
        }
        
        result += ssa_var;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* More SSA complexity with nested loops */
    int x = 0, y = 0, z = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if ((i + j) % 2 == 0) {
                x = i + j;
            } else {
                x = i - j;
            }
            y += x;
            
            /* Vector operation in nested loop */
            vec_c[i % 4] = vec_c[i % 4] + x;
        }
        z += y;
    }
    
    /* Extract results from vectors to prevent elimination */
    int vec_sum = vec_c[0] + vec_c[1] + vec_c[2] + vec_c[3];
    vec_sum += vec_d[0] + vec_d[1];
    
    return result + z + vec_sum;
}

/* Function to generate CONSTRUCTOR nodes */
NOINLINE int test_aggregate_init(void) {
    /* Non-constant initializers create CONSTRUCTOR nodes */
    int dynamic_val = global_volatile + 1;
    
    /* Struct with non-constant initializer */
    struct ComplexStruct cs = {
        .a = func1(dynamic_val),
        .b = func2(dynamic_val * 1.0f),
        .c = func3(dynamic_val * 1.0),
        .d = (char)(dynamic_val % 256)
    };
    
    /* Array with non-constant initializers */
    int arr[4] = {
        func1(1),
        func1(2) + dynamic_val,
        func1(3) * 2,
        func1(4) / (dynamic_val > 0 ? dynamic_val : 1)
    };
    
    /* Nested struct with array */
    struct NestedStruct ns = {
        .inner = {
            .a = func1(5),
            .b = func2(6.0f),
            .c = func3(7.0),
            .d = 'X'
        },
        .arr = { func1(8), func1(9), func1(10) }
    };
    
    /* More complex constructor with mixed expressions */
    struct {
        int a;
        float b[2];
        struct { int x; int y; } point;
    } complex_agg = {
        .a = dynamic_val * 2,
        .b = { func2(1.0f), func2(2.0f) + dynamic_val },
        .point = { .x = func1(3), .y = func1(4) }
    };
    
    /* Use all constructed values to prevent optimization */
    int sum = cs.a + (int)cs.b + (int)cs.c + cs.d;
    sum += arr[0] + arr[1] + arr[2] + arr[3];
    sum += ns.inner.a + (int)ns.inner.b + (int)ns.inner.c + ns.inner.d;
    sum += ns.arr[0] + ns.arr[1] + ns.arr[2];
    sum += complex_agg.a + (int)complex_agg.b[0] + (int)complex_agg.b[1];
    sum += complex_agg.point.x + complex_agg.point.y;
    
    return sum;
}

/* Function to generate OMP_CLAUSE nodes */
NOINLINE int test_omp_clauses(void) {
    int i;
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel private(i) shared(arr, sum) reduction(+:sum) \
        num_threads(4) if(global_volatile > 0)
    {
        /* Nested OpenMP for with schedule clause */
        #pragma omp for schedule(dynamic, 5) nowait
        for (i = 0; i < 100; i++) {
            sum += arr[i];
        }
        
        /* OpenMP barrier */
        #pragma omp barrier
        
        /* OpenMP sections with different clauses */
        #pragma omp sections private(i) lastprivate(sum)
        {
            #pragma omp section
            {
                int section_sum = 0;
                for (i = 0; i < 50; i++) {
                    section_sum += arr[i];
                }
                sum += section_sum;
            }
            
            #pragma omp section
            {
                int section_sum = 0;
                for (i = 50; i < 100; i++) {
                    section_sum += arr[i];
                }
                sum += section_sum;
            }
        }
        
        /* OpenMP single construct */
        #pragma omp single copyprivate(sum)
        {
            sum = sum * 2;
        }
    }
    
    /* Another OpenMP construct: parallel for with collapse */
    int sum2 = 0;
    int matrix[10][10];
    
    #pragma omp parallel for collapse(2) reduction(+:sum2) \
        schedule(static) ordered
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 10; col++) {
            #pragma omp ordered
            {
                matrix[row][col] = row * col;
                sum2 += matrix[row][col];
            }
        }
    }
    
    /* OpenMP task construct */
    int task_result = 0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task shared(task_result) depend(out: task_result)
            {
                for (int j = 0; j < 100; j++) {
                    task_result += j;
                }
            }
            
            #pragma omp taskwait
        }
    }
    
    return sum + sum2 + task_result;
}

/* Helper functions */
NOINLINE int func1(int x) {
    return x * 2 + global_volatile;
}

NOINLINE float func2(float x) {
    return x * 1.5f + global_volatile;
}

NOINLINE double func3(double x) {
    return x * 2.5 + global_volatile;
}

/* Main function that orchestrates everything */
int main(void) {
    int total = 0;
    
    /* Call all test functions multiple times with different conditions */
    for (int iteration = 0; iteration < 3; iteration++) {
        global_volatile = iteration;
        
        total += test_identifiers_and_blocks();
        total += test_vectors_and_ssa();
        total += test_aggregate_init();
        
        /* Only test OpenMP if supported */
        #ifdef _OPENMP
        total += test_omp_clauses();
        #endif
        
        /* Memory barrier between iterations */
        asm volatile("" : : : "memory");
    }
    
    /* Use the result to prevent dead code elimination */
    global_result = total;
    
    /* Print something so compiler can't optimize everything away */
    if (global_result != 0) {
        return 0;
    }
    
    return 1;
}
