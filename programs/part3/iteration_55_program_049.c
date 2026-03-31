/* test_tree_nodes.c - Comprehensive test to trigger tree_kind classification */
#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global variables with various uses */
int global_var1 = 10;
float global_var2 = 20.5;
char global_var3 = 'A';
static int static_var = 30;
extern int extern_func(int);  /* Forces identifier lookup */

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Pattern 3: TREE_BINFO - C++ class hierarchy */
#ifdef __cplusplus
class BaseClass {
public:
    virtual int method() { return 1; }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
public:
    virtual int method() override { return 2; }
    int extra_method() { return 3; }
};
#endif

/* Pattern 4: SSA_NAME - Functions with loops for SSA generation */
__attribute__((noinline))
int ssa_pattern1(int n) {
    int x = 0;
    int y = 1;
    /* Multiple loops to create SSA names */
    for (int i = 0; i < n; ++i) {
        x = x + i;  /* Creates SSA_NAME for x */
        y = y * 2;  /* Creates SSA_NAME for y */
    }
    
    int z = x;
    for (int j = 0; j < n/2; ++j) {
        z = z - j;  /* Creates SSA_NAME for z */
    }
    
    return x + y + z;
}

__attribute__((noinline))
int ssa_pattern2(int limit) {
    int a = 1, b = 2, c = 3;
    /* Complex control flow for SSA */
    for (int i = 0; i < limit; i++) {
        if (i % 2 == 0) {
            a = a + i;
        } else {
            b = b * i;
        }
        c = c + a + b;  /* Phi node candidate */
    }
    return a * b + c;
}

/* Pattern 5: BLOCK - Nested blocks and statement expressions */
__attribute__((noinline))
int block_pattern(int val) {
    int result = 0;
    
    /* Level 1 block */
    {
        int inner1 = val * 2;
        
        /* Level 2 block */
        {
            int inner2 = inner1 + 5;
            
            /* Level 3 block with statement expression (GCC extension) */
            result = ({
                int temp = inner2;
                temp * 3;
            });
        }
    }
    
    /* Another block with label address */
    {
        void* target = &&end_block;
        goto *target;
        
        dead_code:
            result += 100;
        
        end_block:
            result += 50;
    }
    
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
        .id = 42,
        .values = {1.1f, 2.2f, 3.3f, 4.4f},
        .name = "test"
    };
    
    /* Array initializer */
    int matrix[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    
    /* Compound literal */
    int* dynamic_array = (int[]){10, 20, 30, 40, 50};
    
    /* Nested initializer */
    struct Nested {
        struct {
            int a;
            int b;
        } inner;
        int c;
    } nested = { .inner = {.a = 1, .b = 2}, .c = 3 };
    
    return cs.id + matrix[1][1] + dynamic_array[2] + nested.inner.a;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int product = 1;
    int array[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        array[i] = i + 1;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(size) shared(array) reduction(+:sum) reduction(*:product) schedule(static, 10)
    for (int i = 0; i < 100; i++) {
        sum += array[i];
        if (array[i] > 0) {
            product *= array[i];
        }
    }
    
    /* Another OpenMP section */
    #pragma omp parallel
    {
        #pragma omp single
        {
            sum += 1000;
        }
        
        #pragma omp for nowait
        for (int i = 0; i < 50; i++) {
            product -= i;
        }
    }
    
    return sum + product;
}

/* Vector operations function */
__attribute__((noinline))
int vector_pattern(void) {
#ifdef __GNUC__
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = vec_a + vec_b;
    v4si vec_d = vec_a * vec_b;
    
    v4sf fvec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec_b = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fvec_c = fvec_a * fvec_b;
    
    /* Extract results */
    int result = vec_c[0] + vec_c[1] + vec_d[2];
    result += (int)fvec_c[3];
    return result;
#else
    return 42; /* Fallback */
#endif
}

/* C++ specific patterns */
#ifdef __cplusplus
__attribute__((noinline))
int cpp_binfo_pattern(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Use polymorphism - triggers BINFO lookups */
    int result = base_ptr->method();
    
    /* Cast operations */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        result += derived_ptr->extra_method();
    }
    
    /* Array of base pointers */
    BaseClass* poly_array[3];
    poly_array[0] = &derived;
    
    return result;
}
#endif

/* Main function that ties everything together */
int main(void) {
    volatile int checksum = 0;  /* volatile to prevent optimization */
    
    /* Use IDENTIFIER_NODE patterns */
    checksum += global_var1;
    checksum += (int)global_var2;
    checksum += global_var3;
    checksum += static_var;
    
    /* Take addresses and use sizeof */
    checksum += (int)(&global_var1 != 0);
    checksum += sizeof(global_var2);
    
    /* Call SSA pattern functions */
    checksum += ssa_pattern1(100);
    checksum += ssa_pattern2(50);
    
    /* Call block pattern */
    checksum += block_pattern(10);
    
    /* Call constructor pattern */
    checksum += constructor_pattern();
    
    /* Call vector pattern */
    checksum += vector_pattern();
    
#ifdef __cplusplus
    /* Call C++ BINFO pattern */
    checksum += cpp_binfo_pattern();
#endif
    
    /* Call OpenMP pattern */
    checksum += omp_pattern(100);
    
    /* Final output to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    
    return checksum > 0 ? 0 : 1;
}
