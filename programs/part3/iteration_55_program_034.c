/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */
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

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Pattern 3: TREE_BINFO - C++ class hierarchy (only in C++ mode) */
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
    for (int i = 0; i < n; ++i) {
        x = x + i;      /* Creates SSA_NAME for x */
        y = y * 2;      /* Creates SSA_NAME for y */
    }
    
    int z = x;
    for (int j = 0; j < n; ++j) {
        z = z - j;      /* Creates SSA_NAME for z */
    }
    
    return x + y + z;
}

__attribute__((noinline))
int ssa_pattern2(int n) {
    int a = n;
    int b = 1;
    while (a > 0) {
        b = b * a;      /* Creates SSA_NAME for b */
        a = a - 1;      /* Creates SSA_NAME for a */
    }
    
    int c = 0;
    do {
        c = c + b;      /* Creates SSA_NAME for c */
        b = b / 2;      /* Creates SSA_NAME for b (phi node) */
    } while (b > 10);
    
    return c;
}

/* Pattern 5: BLOCK - Nested blocks and statement expressions */
__attribute__((noinline))
int block_pattern(void) {
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
        
        /* Another block with label address */
        void* target = &&exit_block;
        goto *target;
        
        inner1 = 100;  /* Unreachable, but creates more tree nodes */
    }
    
exit_block:
    return outer;
}

/* Pattern 6: CONSTRUCTOR - Structure and array initializers */
__attribute__((noinline))
int constructor_pattern(void) {
    /* Structure with designated initializer */
    struct Point {
        int x;
        int y;
        float z;
    };
    
    struct Point p1 = { .x = 1, .y = 2, .z = 3.0 };
    struct Point p2 = { 4, 5, 6.0 };
    
    /* Array initializer */
    int arr1[5] = {1, 2, 3, 4, 5};
    int arr2[] = {6, 7, 8, 9, 10};
    
    /* Compound literals */
    int sum = ((int[3]){p1.x, p2.x, 3})[0] +
              ((int[3]){p1.y, p2.y, 6})[1];
    
    /* Nested structure initializer */
    struct Nested {
        struct Point pt;
        int id;
    } nested = { .pt = { .x = 10, .y = 20, .z = 30.0 }, .id = 100 };
    
    return sum + nested.id + arr1[0] + arr2[0];
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static)
    for (int i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP section */
    int max_val = 0;
    #pragma omp parallel sections private(i) firstprivate(sum) lastprivate(max_val)
    {
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
        }
        
        #pragma omp section
        {
            for (int i = 50; i < 100; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
        }
    }
    
    return sum + max_val;
}

/* Vector pattern function */
__attribute__((noinline))
int vector_pattern(void) {
#ifdef __GNUC__
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;      /* Vector operation */
    v4si vec4 = vec1 * vec2;      /* Another vector operation */
    
    v4sf fvec1 = {1.0, 2.0, 3.0, 4.0};
    v4sf fvec2 = {0.5, 0.5, 0.5, 0.5};
    v4sf fvec3 = fvec1 * fvec2;
    
    /* Extract elements to force usage */
    int result = vec3[0] + vec3[1] + vec3[2] + vec3[3];
    result += (int)fvec3[0];
    
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
        result += derived_ptr->extra_method();
    }
    
    /* Array of base pointers */
    BaseClass* base_array[3];
    base_array[0] = &derived;
    
    return result;
}
#endif

/* Main function that combines all patterns */
int main(void) {
    volatile int checksum = 0;
    
    /* Use global identifiers in various ways */
    checksum += global_var1;
    checksum += (int)(&global_var2 != 0);
    checksum += sizeof(global_var3);
    checksum += (int)global_var5;
    
    /* Call pattern functions */
    checksum += ssa_pattern1(10);
    checksum += ssa_pattern2(5);
    checksum += block_pattern();
    checksum += constructor_pattern();
    checksum += vector_pattern();
    
#ifdef _OPENMP
    checksum += omp_pattern(100);
#endif
    
#ifdef __cplusplus
    checksum += binfo_pattern();
#endif
    
    /* Prevent dead code elimination */
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
