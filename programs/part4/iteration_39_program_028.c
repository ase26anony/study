/* test_tree_coverage.c - Comprehensive test to trigger specific tree node classifications */

/* For TREE_BINFO - requires C++ or LTO */
#ifdef __cplusplus
class BaseClass {
public:
    virtual void method() { }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual void method() override { }
    int derived_data;
};

void test_binfo() {
    DerivedClass d;
    BaseClass* b = &d;
    b->method();  /* This should generate BINFO nodes */
}
#else
/* For C, we rely on LTO to potentially generate BINFO-like nodes */
struct BaseStruct {
    int data;
};

struct DerivedStruct {
    struct BaseStruct base;
    int extra;
};

void test_binfo_c(void) {
    struct DerivedStruct d;
    struct BaseStruct* b = (struct BaseStruct*)&d;
    volatile int use = b->data;
    (void)use;
}
#endif

/* For IDENTIFIER_NODE - many distinct identifiers */
#define GENERATE_IDENTIFIER(n) identifier_##n
#define USE_IDENTIFIER(n) volatile int GENERATE_IDENTIFIER(n) = n;

void test_identifiers(void) {
    /* Generate many unique identifiers */
    USE_IDENTIFIER(0) USE_IDENTIFIER(1) USE_IDENTIFIER(2)
    USE_IDENTIFIER(3) USE_IDENTIFIER(4) USE_IDENTIFIER(5)
    USE_IDENTIFIER(6) USE_IDENTIFIER(7) USE_IDENTIFIER(8)
    USE_IDENTIFIER(9)
    
    /* More complex identifier usage */
    int complex_identifier_with_long_name_1 = 1;
    int another_complex_name_for_coverage_2 = 2;
    int yet_more_identifiers_here_3 = 3;
    
    /* Use them to prevent optimization */
    asm volatile("" : : "r"(complex_identifier_with_long_name_1),
                     "r"(another_complex_name_for_coverage_2),
                     "r"(yet_more_identifiers_here_3));
}

/* For TREE_VEC - vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

void test_vectors(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* Vector operation */
    v4si d = a * b;  /* Another vector operation */
    
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf f3 = f1 * f2;
    
    /* Use results to prevent optimization */
    volatile v4si* vptr = &c;
    volatile v4sf* fptr = &f3;
    (void)vptr;
    (void)fptr;
}

/* For SSA_NAME - complex control flow with variable modifications */
int test_ssa(int n) {
    int x = 0;
    int y = 1;
    
    /* Loop with multiple assignments to create SSA form */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            x = y + i;      /* Assignment in one branch */
        } else {
            x = y - i;      /* Different assignment in another branch */
        }
        
        if (i % 3 == 0) {
            y = x * 2;      /* Another assignment creating phi nodes */
        } else {
            y = x / 2;
        }
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    return x + y;
}

/* For BLOCK - nested scopes with local variables */
void test_blocks(int iterations) {
    int outer = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Nested block 1 */
        {
            int block_var_1 = i * 2;
            outer += block_var_1;
            
            /* Deeper nested block */
            {
                int inner_var = block_var_1 + 1;
                volatile int use = inner_var;
                (void)use;
            }
        }
        
        /* Another nested block */
        if (i % 2 == 0) {
            int even_block_var = i * 3;
            outer -= even_block_var;
            
            {
                int another_inner = even_block_var / 2;
                volatile int use = another_inner;
                (void)use;
            }
        }
    }
    
    volatile int result = outer;
    (void)result;
}

/* For CONSTRUCTOR - aggregate initialization with non-constant expressions */
struct ComplexStruct {
    int a, b, c;
    float f;
    double d;
};

int global_counter = 0;

int get_next_value(void) {
    return global_counter++;
}

void test_constructors(void) {
    /* Array with non-constant initializers */
    int dynamic_array[4] = {
        get_next_value(),
        get_next_value() * 2,
        get_next_value() + 5,
        get_next_value() - 3
    };
    
    /* Struct with designated initializers and non-constant values */
    struct ComplexStruct s = {
        .a = get_next_value(),
        .b = dynamic_array[0] + 1,
        .c = get_next_value() * 3,
        .f = (float)get_next_value() / 2.0f,
        .d = (double)get_next_value() * 1.5
    };
    
    /* Nested aggregate initialization */
    struct Nested {
        struct ComplexStruct inner;
        int extra[2];
    };
    
    struct Nested n = {
        .inner = {
            .a = get_next_value(),
            .b = get_next_value(),
            .c = get_next_value(),
            .f = 3.14f,
            .d = 2.71828
        },
        .extra = { get_next_value(), get_next_value() }
    };
    
    /* Use all to prevent optimization */
    volatile int* ptr1 = &dynamic_array[0];
    volatile struct ComplexStruct* ptr2 = &s;
    volatile struct Nested* ptr3 = &n;
    (void)ptr1; (void)ptr2; (void)ptr3;
}

/* For OMP_CLAUSE - OpenMP pragmas with various clauses */
#ifdef _OPENMP
#include <omp.h>

void test_omp_clauses(int size) {
    int i;
    int sum = 0;
    int* array = (int*)__builtin_alloca(size * sizeof(int));
    
    /* Initialize array */
    for (i = 0; i < size; i++) {
        array[i] = i;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(i) shared(array, size) \
        reduction(+:sum) schedule(dynamic, 4) \
        default(none) if(size > 100)
    for (i = 0; i < size; i++) {
        sum += array[i];
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel num_threads(4) \
        copyin(global_counter)
    {
        #pragma omp single nowait
        {
            volatile int single_result = sum;
            (void)single_result;
        }
        
        #pragma omp barrier
        
        #pragma omp sections lastprivate(i)
        {
            #pragma omp section
            {
                i = 1;
            }
            #pragma omp section
            {
                i = 2;
            }
        }
    }
    
    volatile int final_sum = sum;
    (void)final_sum;
}
#else
void test_omp_clauses(int size) {
    (void)size;
    /* No OpenMP support */
}
#endif

/* Main function that ties everything together */
int main(int argc, char** argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    /* Test all tree node types */
    test_identifiers();
    
    test_vectors();
    
    int ssa_result = test_ssa(iterations);
    
    test_blocks(iterations);
    
    test_constructors();
    
    test_omp_clauses(iterations);
    
    /* For BINFO nodes */
    #ifdef __cplusplus
    test_binfo();
    #else
    test_binfo_c();
    #endif
    
    /* Final result to prevent dead code elimination */
    volatile int final_check = ssa_result + global_counter;
    return final_check % 256;
}
