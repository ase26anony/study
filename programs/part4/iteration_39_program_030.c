/* test_tree_coverage.c - Comprehensive test for GCC tree node coverage */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization */
static volatile int volatile_counter = 0;
#define NOOPT asm volatile("" : : : "memory")

/* Generate many identifiers for IDENTIFIER_NODE */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(var_, n)

/* Function to create many identifiers */
void test_identifiers_and_blocks(void) {
    NOOPT;
    /* Create multiple identifiers */
    int MAKE_ID(1) = 1;
    int MAKE_ID(2) = 2;
    int MAKE_ID(3) = 3;
    int MAKE_ID(4) = 4;
    int MAKE_ID(5) = 5;
    
    /* Nested blocks for BLOCK nodes */
    {
        int block_local_1 = MAKE_ID(1) + MAKE_ID(2);
        NOOPT;
        {
            int block_local_2 = block_local_1 * 2;
            volatile_counter += block_local_2;
            NOOPT;
            {
                int block_local_3 = block_local_2 / 2;
                volatile_counter -= block_local_3;
            }
        }
    }
    
    /* More blocks in loops */
    for (int i = 0; i < 3; i++) {
        int loop_block_var = i * 10;
        {
            int inner_block_var = loop_block_var + 5;
            volatile_counter += inner_block_var;
        }
    }
}

/* Vector types for TREE_VEC */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with SSA_NAME generation */
int test_vectors_and_ssa(int n) {
    NOOPT;
    /* Vector operations for TREE_VEC */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = c * a;
    
    /* Complex loop for SSA_NAME generation */
    int x = 0, y = 0, z = 0;
    
    for (int i = 0; i < n; i++) {
        NOOPT;
        /* Multiple assignments creating SSA variables */
        if (i % 2 == 0) {
            x = i * 2;
            y = x + 1;
        } else {
            x = i * 3;
            y = x - 1;
        }
        
        /* Phi node candidate */
        z += x + y;
        
        /* Another SSA opportunity */
        int temp;
        if (z > 100) {
            temp = z / 2;
        } else {
            temp = z * 2;
        }
        volatile_counter += temp;
    }
    
    /* Extract from vector */
    int result = c[0] + d[1] + z;
    NOOPT;
    return result;
}

/* Aggregate initialization for CONSTRUCTOR */
struct ComplexStruct {
    int a;
    double b;
    char c;
    int d[4];
};

struct NestedStruct {
    struct ComplexStruct inner;
    float extra;
};

/* Non-constant initializer function */
int get_value(void) {
    NOOPT;
    return volatile_counter % 100 + 1;
}

double get_double(void) {
    NOOPT;
    return (double)(volatile_counter % 100) / 10.0;
}

void test_aggregate_init(void) {
    NOOPT;
    
    /* CONSTRUCTOR with non-constant initializers */
    struct ComplexStruct s1 = {
        .a = get_value(),
        .b = get_double(),
        .c = 'X',
        .d = {get_value(), get_value() + 1, get_value() + 2, get_value() + 3}
    };
    
    /* Array constructor */
    int arr[5] = {
        get_value(),
        get_value() * 2,
        get_value() + 10,
        s1.a,
        s1.d[0]
    };
    
    /* Nested struct constructor */
    struct NestedStruct ns = {
        .inner = {
            .a = arr[0],
            .b = 3.14159,
            .c = 'Y',
            .d = {arr[1], arr[2], arr[3], arr[4]}
        },
        .extra = 2.71828f
    };
    
    volatile_counter += s1.a + arr[2] + (int)ns.extra;
    NOOPT;
}

/* OpenMP for OMP_CLAUSE */
#ifdef _OPENMP
#include <omp.h>

void test_omp_clauses(int size) {
    NOOPT;
    int i;
    int sum = 0;
    int* data = (int*)malloc(size * sizeof(int));
    
    if (!data) return;
    
    /* Initialize data */
    for (i = 0; i < size; i++) {
        data[i] = i + 1;
    }
    
    /* OpenMP with multiple clauses for OMP_CLAUSE nodes */
    #pragma omp parallel for private(i) shared(data, size) reduction(+:sum) schedule(dynamic) num_threads(4)
    for (i = 0; i < size; i++) {
        sum += data[i] * 2;
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel private(i) shared(sum, volatile_counter)
    {
        #pragma omp single
        {
            volatile_counter += sum;
        }
        
        #pragma omp barrier
        
        #pragma omp for nowait
        for (i = 0; i < 10; i++) {
            #pragma omp atomic
            volatile_counter++;
        }
    }
    
    /* OpenMP sections */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < 5; i++) {
                #pragma omp atomic
                sum--;
            }
        }
        
        #pragma omp section
        {
            for (i = 0; i < 5; i++) {
                #pragma omp atomic
                sum++;
            }
        }
    }
    
    volatile_counter += sum;
    free(data);
    NOOPT;
}
#else
void test_omp_clauses(int size) {
    (void)size;
    printf("OpenMP not supported\n");
}
#endif

/* C++ compatible code for TREE_BINFO */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int method() { return 1; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int method() override { return 2; }
    int derived_data;
};

void test_cpp_binfo(void) {
    DerivedClass d;
    BaseClass* b = &d;
    volatile_counter += b->method();
    
    DerivedClass* d2 = dynamic_cast<DerivedClass*>(b);
    if (d2) {
        volatile_counter += d2->method();
    }
}
#else
/* For C, we'll use LTO to potentially generate BINFO nodes */
struct BaseStruct {
    int type_id;
    void* vtable[2];  /* Simulate minimal vtable */
};

struct DerivedStruct {
    struct BaseStruct base;
    int extra_data;
};

void test_cpp_binfo(void) {
    /* This might generate BINFO-like structures with -flto */
    struct DerivedStruct d = {{0, {NULL, NULL}}, 42};
    struct BaseStruct* b = (struct BaseStruct*)&d;
    volatile_counter += d.extra_data;
}
#endif

/* Main function that ties everything together */
int main(int argc, char** argv) {
    NOOPT;
    
    /* Use command line argument to prevent constant folding */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations < 1) iterations = 10;
    
    printf("Starting tree coverage test...\n");
    
    /* Test all constructs */
    test_identifiers_and_blocks();
    
    int ssa_result = test_vectors_and_ssa(iterations);
    printf("SSA test result: %d\n", ssa_result);
    
    test_aggregate_init();
    
    test_omp_clauses(iterations * 2);
    
    test_cpp_binfo();
    
    /* Final result to prevent dead code elimination */
    int final_result = volatile_counter + ssa_result;
    printf("Final volatile counter: %d\n", volatile_counter);
    printf("Total result: %d\n", final_result);
    
    return final_result % 256;
}
