/* test_tree_coverage.c - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global variables and operations */
int global_var1 = 10;
int global_var2 = 20;
float global_var3 = 3.14;
double global_var4 = 2.71828;
char global_var5 = 'A';

extern int external_func(int);  /* Forces identifier lookup */

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Pattern 3: TREE_BINFO - C++ class hierarchy (if compiled as C++) */
#ifdef __cplusplus
class BaseClass {
public:
    virtual int method() { return 1; }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
public:
    virtual int method() override { return 2; }
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
    for (int j = 0; j < 10; ++j) {
        z = z - j;  /* Creates SSA_NAME for z */
    }
    
    return x + y + z;
}

__attribute__((noinline))
int ssa_pattern2(int n) {
    int a = n;
    int b = 1;
    int c = 0;
    
    /* Complex control flow for SSA */
    while (a > 0) {
        if (a % 2 == 0) {
            b = b * a;
        } else {
            c = c + a;
        }
        a = a - 1;  /* Creates SSA_NAME for a */
    }
    
    return b + c;
}

/* Pattern 5: BLOCK - Nested blocks and statement expressions */
__attribute__((noinline))
int block_pattern(void) {
    int result = 0;
    
    /* Level 1 block */
    {
        int a = 10;
        
        /* Level 2 block */
        {
            int b = 20;
            
            /* Level 3 block with statement expression */
            result = ({
                int c = 30;
                int d = 40;
                /* Nested block inside statement expression */
                {
                    int e = a + b + c + d;
                    e * 2;
                }
            });
        }
    }
    
    /* Another block with goto and label */
    {
        void* label_ptr = &&my_label;
        goto *label_ptr;
        
        my_label:
        result += 100;
    }
    
    return result;
}

/* Pattern 6: CONSTRUCTOR - Structure and array initializers */
__attribute__((noinline))
int constructor_pattern(void) {
    /* Structure with designated initializers */
    struct ComplexStruct {
        int id;
        float values[4];
        char name[16];
        double score;
    };
    
    /* Multiple constructors */
    struct ComplexStruct s1 = {
        .id = 1,
        .values = {1.1f, 2.2f, 3.3f, 4.4f},
        .name = "test",
        .score = 99.5
    };
    
    struct ComplexStruct s2 = {
        .id = 2,
        .values = {5.5f, 6.6f, 7.7f, 8.8f},
        .name = "example",
        .score = 88.5
    };
    
    /* Compound literals */
    int* arr1 = (int[3]){1, 2, 3};
    float* arr2 = (float[]){1.0f, 2.0f, 3.0f, 4.0f};
    
    /* Nested constructors */
    struct {
        struct {
            int x;
            int y;
        } point;
        int data[2];
    } nested = {
        .point = {.x = 10, .y = 20},
        .data = {100, 200}
    };
    
    return s1.id + s2.id + arr1[0] + (int)arr2[0] + nested.point.x;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int product = 1;
    int* arr = (int*)__builtin_alloca(size * sizeof(int));
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        arr[i] = i + 1;
    }
    
    /* Multiple OpenMP directives with various clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static)
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    #pragma omp parallel sections private(product)
    {
        #pragma omp section
        {
            product = 1;
            for (int i = 0; i < size/2; i++) {
                product *= arr[i];
            }
        }
        
        #pragma omp section
        {
            int local_prod = 1;
            for (int i = size/2; i < size; i++) {
                local_prod *= arr[i];
            }
            #pragma omp critical
            product *= local_prod;
        }
    }
    
    /* OpenMP parallel region with multiple clauses */
    int max_val = 0;
    int min_val = 1000000;
    #pragma omp parallel default(none) shared(arr, size, max_val, min_val) \
                         firstprivate(sum) reduction(max:max_val) reduction(min:min_val)
    {
        #pragma omp for nowait
        for (int i = 0; i < size; i++) {
            if (arr[i] > max_val) max_val = arr[i];
            if (arr[i] < min_val) min_val = arr[i];
        }
    }
    
    return sum + product + max_val + min_val;
}

/* Pattern 2: Vector operations function */
__attribute__((noinline))
int vector_pattern(void) {
#ifdef __GNUC__
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;      /* Vector addition */
    v4si vec4 = vec1 * vec2;      /* Vector multiplication */
    v4si vec5 = vec4 - vec3;      /* Vector subtraction */
    
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {0.5f, 0.5f, 0.5f, 0.5f};
    v4sf fvec3 = fvec1 * fvec2;   /* Float vector multiplication */
    
    /* Use vectors in expressions */
    int result = vec3[0] + vec4[1] + vec5[2] + (int)fvec3[3];
    return result;
#else
    return 42;  /* Fallback for non-GCC */
#endif
}

/* Pattern 3: C++ BINFO usage (only in C++ mode) */
#ifdef __cplusplus
__attribute__((noinline))
int binfo_pattern(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Access through base pointer - involves BINFO */
    int result = base_ptr->method();
    
    /* Create another derived object */
    DerivedClass* derived_ptr = new DerivedClass();
    BaseClass& base_ref = *derived_ptr;
    result += base_ref.method();
    
    delete derived_ptr;
    return result;
}
#endif

/* Main function that calls all patterns */
int main(void) {
    volatile int checksum = 0;  /* volatile to prevent optimization */
    
    /* Use IDENTIFIER_NODE patterns */
    checksum += global_var1;
    checksum += global_var2;
    checksum += (int)global_var3;
    checksum += (int)global_var4;
    checksum += global_var5;
    
    /* Take addresses and use sizeof */
    checksum += (int)(&global_var1 != 0);
    checksum += sizeof(global_var2);
    
    /* Call SSA pattern functions */
    checksum += ssa_pattern1(100);
    checksum += ssa_pattern2(50);
    
    /* Call block pattern */
    checksum += block_pattern();
    
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
    
    /* Final volatile output to prevent dead code elimination */
    volatile int final_result = checksum;
    
#ifdef __cplusplus
    std::cout << "Result: " << final_result << std::endl;
#else
    printf("Result: %d\n", final_result);
#endif
    
    return 0;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif
