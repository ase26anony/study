/* test_tree_nodes.c - Comprehensive test to trigger tree_kind classification */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: IDENTIFIER_NODE - Global variables with various uses */
int global_var_1 = 10;
float global_var_2 = 20.5;
char global_var_3 = 'A';
double global_var_4 = 30.75;

/* External function declarations to force identifier lookups */
extern int external_func_1(int);
extern void external_func_2(float);
extern char external_func_3(void);

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Pattern 3: TREE_BINFO - C++ class hierarchy */
#ifdef __cplusplus
class BaseClass {
public:
    virtual int virtual_method() { return 42; }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
public:
    virtual int virtual_method() override { return 84; }
    int additional_method() { return 168; }
};
#endif

/* Pattern 4: SSA_NAME - Functions with loops for SSA generation */
__attribute__((noinline))
int ssa_pattern_1(int n) {
    int x = 0;
    int y = 1;
    
    /* Multiple loops to generate SSA_NAME nodes */
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
    }
    
    for (int j = n; j > 0; --j) {
        x = x - j;
        y = y / (j + 1);
    }
    
    int z = 0;
    while (z < n) {
        x = x ^ z;  /* XOR operation */
        y = y | z;  /* OR operation */
        z++;
    }
    
    return x + y;
}

__attribute__((noinline))
float ssa_pattern_2(float start, int iterations) {
    float result = start;
    float accumulator = 0.0f;
    
    for (int i = 0; i < iterations; ++i) {
        result = result * 1.1f;
        accumulator = accumulator + result;
        
        if (i % 2 == 0) {
            result = result - 0.5f;
        } else {
            result = result + 0.5f;
        }
    }
    
    return accumulator;
}

/* Pattern 5: BLOCK - Nested blocks and statement expressions */
__attribute__((noinline))
int block_pattern(void) {
    int outer = 0;
    
    /* Level 1 block */
    {
        int level1 = 10;
        
        /* Level 2 block */
        {
            int level2 = 20;
            
            /* Level 3 block */
            {
                int level3 = 30;
                outer = level1 + level2 + level3;
            }
        }
    }
    
    /* GCC statement expression (creates a block) */
    int stmt_expr_result = ({
        int temp_a = 5;
        int temp_b = 10;
        int temp_c = temp_a * temp_b;
        temp_c + 15;
    });
    
    /* Labels and goto for block nodes */
    void* label_ptr = &&my_label;
    goto *label_ptr;
    
my_label:
    return outer + stmt_expr_result;
}

/* Pattern 6: CONSTRUCTOR - Structure and array initializers */
__attribute__((noinline))
int constructor_pattern(void) {
    /* Structure with designated initializers */
    struct ComplexStruct {
        int int_field;
        float float_field;
        double double_field;
        char char_field;
    };
    
    struct ComplexStruct cs = {
        .int_field = 100,
        .float_field = 200.5f,
        .double_field = 300.75,
        .char_field = 'Z'
    };
    
    /* Array initializer */
    int array_init[5] = {1, 2, 3, 4, 5};
    
    /* Nested structure initializer */
    struct Nested {
        struct {
            int a;
            int b;
        } inner;
        int outer;
    } nested = { .inner = { .a = 10, .b = 20 }, .outer = 30 };
    
    /* Compound literal */
    int* dynamic_array = (int[3]){100, 200, 300};
    
    return cs.int_field + array_init[2] + nested.outer + dynamic_array[1];
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int* arr = (int*)malloc(size * sizeof(int));
    
    if (!arr) return -1;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP parallel for with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static)
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP section with different clauses */
    int max_val = 0;
    #pragma omp parallel sections private(i) shared(arr, max_val)
    {
        #pragma omp section
        {
            int local_max = 0;
            for (int i = 0; i < size/2; i++) {
                if (arr[i] > local_max) local_max = arr[i];
            }
            #pragma omp critical
            {
                if (local_max > max_val) max_val = local_max;
            }
        }
        
        #pragma omp section
        {
            int local_max = 0;
            for (int i = size/2; i < size; i++) {
                if (arr[i] > local_max) local_max = arr[i];
            }
            #pragma omp critical
            {
                if (local_max > max_val) max_val = local_max;
            }
        }
    }
    
    free(arr);
    return sum + max_val;
}

/* Vector operations function */
__attribute__((noinline))
#ifdef __GNUC__
v4si vector_pattern(v4si a, v4si b) {
    v4si result;
    
    /* Various vector operations */
    result = a + b;
    result = result * a;
    result = result - b;
    result = result / (a + 1);
    
    /* Vector comparisons */
    v4si mask = a > b;
    result = result & mask;
    
    return result;
}
#else
int vector_pattern(int a, int b) {
    return a + b;  /* Fallback for non-GCC */
}
#endif

/* Main function that ties everything together */
int main(void) {
    volatile int checksum = 0;
    
    /* Use IDENTIFIER_NODE patterns */
    checksum += global_var_1;
    checksum += (int)global_var_2;
    checksum += global_var_3;
    checksum += (int)global_var_4;
    
    /* Take addresses of identifiers */
    int* ptr1 = &global_var_1;
    float* ptr2 = &global_var_2;
    checksum += *ptr1 + (int)*ptr2;
    
    /* Use sizeof on identifiers */
    checksum += sizeof(global_var_1);
    checksum += sizeof(global_var_3);
    
    /* Call SSA pattern functions */
    checksum += ssa_pattern_1(100);
    checksum += (int)ssa_pattern_2(1.0f, 50);
    
    /* Call block pattern */
    checksum += block_pattern();
    
    /* Call constructor pattern */
    checksum += constructor_pattern();
    
    /* Call OpenMP pattern */
    checksum += omp_pattern(1000);
    
    /* Vector operations */
    #ifdef __GNUC__
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_result = vector_pattern(vec_a, vec_b);
    checksum += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    #endif
    
    /* C++ polymorphism for BINFO */
    #ifdef __cplusplus
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    checksum += base_ptr->virtual_method();
    
    /* dynamic_cast for BINFO usage */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        checksum += derived_ptr->additional_method();
    }
    #endif
    
    printf("Final checksum: %d\n", checksum);
    return checksum == 0 ? 0 : 1;
}

#ifdef __cplusplus
}
#endif
