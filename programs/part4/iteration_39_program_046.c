/* test_tree_coverage.c - Comprehensive test to trigger all tree node classifications */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization */
static volatile int volatile_sink;

/* Memory barrier to prevent reordering */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* Generate many unique identifiers for IDENTIFIER_NODE */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

/* Function declarations to ensure identifiers are used */
void use_identifiers(void) {
    /* Generate 20 unique identifiers */
    int MAKE_ID(0) = 1, MAKE_ID(1) = 2, MAKE_ID(2) = 3, MAKE_ID(3) = 4;
    int MAKE_ID(4) = 5, MAKE_ID(5) = 6, MAKE_ID(6) = 7, MAKE_ID(7) = 8;
    int MAKE_ID(8) = 9, MAKE_ID(9) = 10, MAKE_ID(10) = 11, MAKE_ID(11) = 12;
    int MAKE_ID(12) = 13, MAKE_ID(13) = 14, MAKE_ID(14) = 15, MAKE_ID(15) = 16;
    int MAKE_ID(16) = 17, MAKE_ID(17) = 18, MAKE_ID(18) = 19, MAKE_ID(19) = 20;
    
    volatile_sink = MAKE_ID(0) + MAKE_ID(19);
    MEMORY_BARRIER();
}

/* Test TREE_VEC with GCC vector extensions */
void test_vector_operations(void) {
    /* Vector type declaration */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Multiple vector operations */
    v4si result1 = a + b;
    v4si result2 = a * b;
    v4si result3 = result1 - c;
    
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fresult = f1 * f2;
    
    /* Use results to prevent optimization */
    volatile int* vp = (volatile int*)&result3;
    volatile_sink = vp[0] + (int)fresult[0];
    MEMORY_BARRIER();
}

/* Test SSA_NAME generation with complex control flow */
int test_ssa_formation(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Loop with multiple branches to create SSA phi nodes */
    for (int i = 0; i < n; i++) {
        /* Nested block for BLOCK nodes */
        {
            int temp = i * 2;
            if (temp % 3 == 0) {
                x = y + z;
            } else if (temp % 3 == 1) {
                x = y - z;
            } else {
                x = y * z;
            }
            
            /* Another nested block */
            {
                int inner = x + i;
                if (inner > 10) {
                    y = inner - 5;
                } else {
                    y = inner + 5;
                }
            }
        }
        
        /* Switch to create more SSA complexity */
        switch (i % 4) {
            case 0: z = x + y; break;
            case 1: z = x - y; break;
            case 2: z = x * y; break;
            case 3: z = x / (y != 0 ? y : 1); break;
        }
    }
    
    return x + y + z;
}

/* Test CONSTRUCTOR nodes with aggregate initialization */
struct ComplexStruct {
    int a;
    float b;
    double c;
    int* d;
};

int* get_pointer(void) {
    static int value = 42;
    return &value;
}

float compute_float(void) {
    return 3.14159f;
}

void test_aggregate_initialization(void) {
    /* Non-constant initializers for CONSTRUCTOR nodes */
    struct ComplexStruct s1 = {
        .a = rand() % 100,
        .b = compute_float(),
        .c = 2.71828,
        .d = get_pointer()
    };
    
    /* Array with non-constant initializers */
    int arr[5] = {
        rand() % 10,
        rand() % 20,
        rand() % 30,
        rand() % 40,
        rand() % 50
    };
    
    /* Nested struct initialization */
    struct Inner {
        int x;
        int y;
    };
    
    struct Outer {
        struct Inner i;
        int z;
    };
    
    struct Outer o = {
        .i = {rand() % 5, rand() % 10},
        .z = rand() % 100
    };
    
    volatile_sink = s1.a + arr[0] + o.i.x;
    MEMORY_BARRIER();
}

/* Test BLOCK nodes with deeply nested scopes */
void test_nested_blocks(int iterations) {
    int outer = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Level 1 block */
        {
            int level1 = i * 2;
            
            /* Level 2 block */
            {
                int level2 = level1 + 1;
                
                /* Level 3 block */
                {
                    int level3 = level2 * 3;
                    
                    /* Level 4 block with its own variables */
                    {
                        int level4 = level3 - 5;
                        outer += level4;
                        
                        /* Innermost block */
                        {
                            volatile int innermost = level4 % 7;
                            volatile_sink = innermost;
                        }
                    }
                }
            }
        }
        
        /* Another independent block in loop */
        {
            static int counter = 0;
            counter++;
            if (counter % 2 == 0) {
                outer--;
            } else {
                outer++;
            }
        }
    }
    
    volatile_sink = outer;
}

/* Test OMP_CLAUSE nodes with OpenMP */
#ifdef _OPENMP
#include <omp.h>

void test_openmp_clauses(int size) {
    int i;
    int sum = 0;
    int* array = (int*)malloc(size * sizeof(int));
    
    if (!array) return;
    
    /* Initialize array */
    for (i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(i) shared(array, size) reduction(+:sum) \
            schedule(dynamic, 4) default(none) if(size > 100)
    for (i = 0; i < size; i++) {
        /* Nested block inside parallel region */
        {
            int temp = array[i];
            if (temp % 2 == 0) {
                sum += temp * 2;
            } else {
                sum += temp;
            }
        }
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel num_threads(4)
    {
        #pragma omp single nowait
        {
            volatile_sink = sum;
        }
        
        #pragma omp barrier
        
        #pragma omp sections private(i)
        {
            #pragma omp section
            {
                int local_sum = 0;
                for (i = 0; i < size/2; i++) {
                    local_sum += array[i];
                }
                #pragma omp atomic
                sum += local_sum;
            }
            
            #pragma omp section
            {
                int local_sum = 0;
                for (i = size/2; i < size; i++) {
                    local_sum += array[i];
                }
                #pragma omp critical
                sum += local_sum;
            }
        }
    }
    
    volatile_sink = sum;
    free(array);
}
#endif

/* Complex function combining multiple constructs */
void combined_test(void) {
    /* Use identifiers */
    use_identifiers();
    
    /* Vector operations */
    test_vector_operations();
    
    /* SSA formation with blocks */
    int ssa_result = test_ssa_formation(50);
    
    /* Aggregate initialization */
    test_aggregate_initialization();
    
    /* Nested blocks */
    test_nested_blocks(25);
    
    /* Use results */
    volatile_sink = ssa_result;
}

/* Main function that orchestrates everything */
int main(int argc, char** argv) {
    int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Run combined test */
    combined_test();
    
    /* Test SSA separately */
    int result1 = test_ssa_formation(iterations);
    
    /* Test with more iterations for deeper blocks */
    test_nested_blocks(iterations * 2);
    
    #ifdef _OPENMP
    /* Test OpenMP if available */
    test_openmp_clauses(iterations);
    #endif
    
    /* Final aggregate test */
    test_aggregate_initialization();
    
    /* Print result to ensure execution */
    printf("Result: %d (volatile sink: %d)\n", 
           result1, volatile_sink);
    
    return 0;
}
