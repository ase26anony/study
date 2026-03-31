/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global variables and usage */
int global_var_1 = 10;
int global_var_2 = 20;
float global_var_3 = 3.14;
double global_var_4 = 2.71828;
char global_var_5 = 'A';

/* External function declarations to force identifier lookups */
extern int external_func_1(int);
extern void external_func_2(double);
extern char* external_func_3(void);

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
#endif

/* Pattern 3: TREE_BINFO - C++ class hierarchy (if compiled as C++) */
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
static int ssa_pattern_1(int n) {
    int x = 0;
    int y = 1;
    for (int i = 0; i < n; ++i) {
        x = x + i;      /* Creates SSA_NAME for x */
        y = y * 2;      /* Creates SSA_NAME for y */
    }
    
    int z = x;
    for (int j = 0; j < y; ++j) {
        z = z - j;      /* Creates SSA_NAME for z */
    }
    return z;
}

__attribute__((noinline))
static float ssa_pattern_2(float init) {
    float a = init;
    float b = init * 2.0f;
    for (int i = 0; i < 100; ++i) {
        a = a + b;      /* Creates SSA_NAME for a */
        b = b * 1.01f;  /* Creates SSA_NAME for b */
    }
    return a + b;
}

/* Pattern 5: BLOCK - Nested blocks and statement expressions */
__attribute__((noinline))
static int block_pattern(void) {
    int outer = 0;
    
    /* Level 1 block */
    {
        int inner1 = 10;
        
        /* Level 2 block */
        {
            int inner2 = 20;
            outer = inner1 + inner2;
            
            /* Level 3 block with statement expression */
            outer = ({
                int temp = outer * 2;
                temp + 5;
            });
        }
    }
    
    /* Another block with label address */
    void* target = &&end_block;
    goto *target;
    
    /* Unreachable code */
    outer = 999;
    
end_block:
    return outer;
}

/* Pattern 6: CONSTRUCTOR - Structure and array initializers */
__attribute__((noinline))
static int constructor_pattern(void) {
    /* Structure with designated initializers */
    struct ComplexStruct {
        int int_field;
        float float_field;
        double double_field;
        char char_field;
        int array_field[3];
    };
    
    struct ComplexStruct cs = {
        .int_field = 42,
        .float_field = 3.14f,
        .double_field = 2.71828,
        .char_field = 'X',
        .array_field = {1, 2, 3}
    };
    
    /* Array initializer */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Compound literal */
    int* ptr = (int[4]){100, 200, 300, 400};
    
    /* Nested structure initializer */
    struct Nested {
        struct {
            int a;
            int b;
        } inner;
        int c;
    } nested = { .inner = {.a = 1, .b = 2}, .c = 3 };
    
    return cs.int_field + arr[0] + ptr[1] + nested.inner.a;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
__attribute__((noinline))
static int omp_pattern(int size) {
    int sum = 0;
    int product = 1;
    int* arr = 0;
    
    if (size > 0) {
        arr = (int*)__builtin_alloca(size * sizeof(int));
        for (int i = 0; i < size; i++) {
            arr[i] = i + 1;
        }
    }
    
    /* Multiple OpenMP directives with various clauses */
    #pragma omp parallel for private(size) shared(arr) reduction(+:sum) if(size > 100)
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    #pragma omp parallel sections private(product)
    {
        #pragma omp section
        {
            product = 1;
            for (int i = 1; i <= 10; i++) {
                product *= i;
            }
        }
        
        #pragma omp section
        {
            /* Another computation */
            int local_sum = 0;
            #pragma omp parallel for reduction(+:local_sum)
            for (int i = 0; i < 20; i++) {
                local_sum += i;
            }
            sum += local_sum;
        }
    }
    
    #pragma omp task depend(inout: sum)
    {
        sum *= 2;
    }
    
    return sum + product;
}

/* Vector operations function */
__attribute__((noinline))
static int vector_pattern(void) {
#ifdef __GNUC__
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = vec_a + vec_b;      /* Vector addition */
    v4si vec_d = vec_a * vec_b;      /* Vector multiplication */
    
    v4sf vec_f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_f2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf vec_f3 = vec_f1 * vec_f2;   /* Float vector multiplication */
    
    /* Use vectors in operations that might create TREE_VEC nodes */
    int result = 0;
    for (int i = 0; i < 4; i++) {
        result += vec_c[i] + vec_d[i];
    }
    
    return result + (int)vec_f3[0];
#else
    return 42;  /* Fallback for non-GCC compilers */
#endif
}

/* C++ specific pattern for BINFO */
#ifdef __cplusplus
__attribute__((noinline))
static int binfo_pattern(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* These operations involve TREE_BINFO nodes */
    int result = base_ptr->virtual_method();
    
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        result += derived_ptr->additional_method();
    }
    
    /* Multiple inheritance would create more BINFO nodes */
    return result;
}
#endif

/* Main function that combines all patterns */
int main(void) {
    volatile int checksum = 0;  /* volatile to prevent optimization */
    
    /* Use identifiers in various ways */
    checksum += global_var_1;
    checksum += (int)global_var_3;
    checksum += (int)global_var_4;
    checksum += global_var_5;
    
    /* Take addresses of identifiers */
    int* ptr1 = &global_var_1;
    float* ptr2 = &global_var_3;
    checksum += *ptr1 + (int)*ptr2;
    
    /* Use sizeof on identifiers */
    checksum += sizeof(global_var_2);
    checksum += sizeof(global_var_4);
    
    /* Call pattern functions */
    checksum += ssa_pattern_1(100);
    checksum += (int)ssa_pattern_2(10.0f);
    checksum += block_pattern();
    checksum += constructor_pattern();
    checksum += vector_pattern();
    
#ifdef __cplusplus
    checksum += binfo_pattern();
#endif
    
    /* OpenMP pattern - compile with -fopenmp */
    checksum += omp_pattern(50);
    
    /* Final output to prevent dead code elimination */
#ifdef __cplusplus
    std::cout << "Checksum: " << checksum << std::endl;
#else
    printf("Checksum: %d\n", checksum);
#endif
    
    return checksum != 0 ? 0 : 1;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif
