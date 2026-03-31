/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global variables and operations */
int global_var1 = 10;
float global_var2 = 20.5;
char global_var3 = 'A';
double global_var4 = 30.75;

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Pattern 3: TREE_BINFO - C++ class hierarchy (only in C++ mode) */
#ifdef __cplusplus
class BaseClass {
public:
    virtual int method() { return 42; }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
public:
    virtual int method() override { return 84; }
};
#endif

/* Pattern 4: SSA_NAME - Functions with loops for SSA generation */
__attribute__((noinline))
int ssa_pattern1(int n) {
    int x = 0;
    int y = 1;
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
    }
    return x + y;
}

__attribute__((noinline))
int ssa_pattern2(int n) {
    int sum = 0;
    int prod = 1;
    for (int i = 1; i <= n; ++i) {
        sum = sum + i;
        prod = prod * i;
        for (int j = 0; j < i; ++j) {
            sum = sum - j;
        }
    }
    return sum * prod;
}

/* Pattern 5: BLOCK - Nested blocks and statement expressions */
__attribute__((noinline))
int block_pattern(int x) {
    int result = 0;
    
    /* Level 1 block */
    {
        int a = x * 2;
        
        /* Level 2 block */
        {
            int b = a + 10;
            
            /* Level 3 block - statement expression */
            result = ({
                int c = b * 3;
                c - 5;
            });
            
            /* Label and goto */
            if (result > 100) {
                goto skip_block;
            }
        }
        
        /* Another block with local variable */
        {
            int d = result / 2;
            result = d * d;
        }
    }
    
skip_block:
    return result;
}

/* Pattern 6: CONSTRUCTOR - Structure and array initializers */
__attribute__((noinline))
int constructor_pattern(void) {
    /* Structure with designated initializer */
    struct ComplexStruct {
        int id;
        float values[4];
        char name[16];
    };
    
    struct ComplexStruct cs = {
        .id = 1001,
        .values = {1.1f, 2.2f, 3.3f, 4.4f},
        .name = "test_struct"
    };
    
    /* Array initializer */
    int matrix[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    
    /* Compound literal */
    int sum = 0;
    int *arr = (int[]){10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        sum += arr[i];
    }
    
    /* Nested structure initializer */
    struct {
        struct {
            int x, y;
        } point;
        int color;
    } nested = { {10, 20}, 0xFF0000 };
    
    return cs.id + matrix[1][1] + sum + nested.point.x;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int product = 1;
    
    #pragma omp parallel for reduction(+:sum) reduction(*:product) private(size) shared(product)
    for (int i = 1; i <= 100; i++) {
        sum += i;
        product *= (i % 10) + 1;
    }
    
    /* Another OpenMP section with different clauses */
    int arr[100];
    #pragma omp parallel for simd schedule(static, 8) collapse(2) ordered
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i * 10 + j] = i * j;
        }
    }
    
    /* OpenMP sections */
    #pragma omp parallel sections num_threads(2)
    {
        #pragma omp section
        {
            sum += 100;
        }
        #pragma omp section
        {
            product *= 2;
        }
    }
    
    return sum + product;
}

/* Vector operations function */
__attribute__((noinline))
int vector_pattern(void) {
#ifdef __GNUC__
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fvec3 = fvec1 * fvec2;
    
    /* Use vectors in operations that prevent optimization */
    int result = 0;
    for (int i = 0; i < 4; i++) {
        result += vec3[i] + (int)fvec3[i];
    }
    return result;
#else
    return 0;
#endif
}

/* C++ specific pattern for BINFO */
#ifdef __cplusplus
__attribute__((noinline))
int binfo_pattern(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Virtual call through base pointer */
    int result = base_ptr->method();
    
    /* Dynamic cast */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        result += derived_ptr->method();
    }
    
    /* Multiple inheritance would create more BINFO nodes */
    return result;
}
#endif

/* Main function that combines all patterns */
int main(void) {
    volatile int checksum = 0;
    
    /* Use IDENTIFIER_NODES in various ways */
    checksum += global_var1;
    checksum += (int)global_var2;
    checksum += global_var3;
    checksum += (int)global_var4;
    
    /* Take addresses to force identifier lookups */
    int* ptr1 = &global_var1;
    float* ptr2 = &global_var2;
    
    /* Use sizeof on identifiers */
    checksum += sizeof(global_var1);
    checksum += sizeof(global_var3);
    
    /* Call SSA pattern functions */
    checksum += ssa_pattern1(100);
    checksum += ssa_pattern2(50);
    
    /* Call block pattern */
    checksum += block_pattern(42);
    
    /* Call constructor pattern */
    checksum += constructor_pattern();
    
    /* Call vector pattern */
    checksum += vector_pattern();
    
#ifdef __cplusplus
    /* Call C++ BINFO pattern */
    checksum += binfo_pattern();
#endif
    
    /* Call OpenMP pattern */
    checksum += omp_pattern(100);
    
    /* Force volatile output to prevent optimization */
    volatile int output = checksum;
    
#ifdef __cplusplus
    std::cout << "Result: " << output << std::endl;
#else
    printf("Result: %d\n", output);
#endif
    
    return 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
