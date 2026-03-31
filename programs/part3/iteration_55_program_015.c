/* test_tree_nodes.c - Comprehensive test to trigger tree_kind classification */

#ifdef __cplusplus
#include <iostream>
extern "C" {
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
    
    for (int j = 0; j < n; ++j) {
        x = x - j;  /* More SSA transformations */
    }
    
    return x + y;
}

__attribute__((noinline))
int ssa_pattern2(int n) {
    int a = 1, b = 2, c = 3;
    /* Complex control flow for SSA */
    for (int i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            a = a + i;
            b = b * 2;
        } else {
            a = a - i;
            c = c + b;
        }
    }
    return a + b + c;
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
            
            /* Level 3 block with statement expression (GCC extension) */
            result = ({
                int temp = b;
                /* Another nested block */
                {
                    temp = temp * 3;
                }
                temp;  /* Returns from statement expression */
            });
        }
    }
    
    /* Label and goto for block creation */
    void* label_ptr = &&my_label;
    
    if (x > 100) {
        goto *label_ptr;
    }
    
    /* Another statement expression */
    result += ({
        int local = 5;
        local * 2;
    });
    
my_label:
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
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Compound literal */
    int sum = 0;
    int* ptr = (int[3]){arr[0], arr[1], arr[2]};
    
    /* Nested structure initializer */
    struct Inner {
        int a, b;
    };
    
    struct Outer {
        struct Inner i;
        int c;
    } outer = {
        .i = {.a = 1, .b = 2},
        .c = 3
    };
    
    /* Union initializer */
    union MyUnion {
        int i;
        float f;
    } u = {.i = 100};
    
    return cs.id + arr[0] + ptr[1] + outer.i.a + u.i;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
#ifdef _OPENMP
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int* arr = (int*)__builtin_alloca(size * sizeof(int));
    
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
            int local_max = arr[0];
            #pragma omp parallel for reduction(max:local_max)
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
            int local_sum = 0;
            #pragma omp parallel for reduction(+:local_sum) collapse(2)
            for (int i = size/2; i < size; i++) {
                for (int j = 0; j < 2; j++) {
                    local_sum += arr[i] + j;
                }
            }
            #pragma omp atomic
            sum += local_sum;
        }
    }
    
    return sum + max_val;
}
#endif

/* Vector pattern function */
__attribute__((noinline))
int vector_pattern(void) {
#ifdef __GNUC__
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;  /* Vector operation */
    v4si vec4 = vec1 * vec2;
    
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {0.5f, 0.5f, 0.5f, 0.5f};
    v4sf fvec3 = fvec1 * fvec2;
    
    /* Extract elements to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += vec3[i] + (int)fvec3[i];
    }
    return sum;
#else
    return 0;
#endif
}

/* C++ BINFO pattern function */
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
    BaseClass* arr[3];
    arr[0] = &derived;
    
    return result;
}
#endif

/* Main function that combines all patterns */
int main(void) {
    volatile int checksum = 0;  /* volatile to prevent optimization */
    
    /* Use global identifiers in various ways */
    checksum += global_var1;
    checksum += (int)global_var2;
    checksum += global_var3;
    checksum += static_var;
    
    /* Take address of identifiers */
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
    /* Call C++ binfo pattern */
    checksum += binfo_pattern();
#endif
    
#ifdef _OPENMP
    /* Call OpenMP pattern */
    checksum += omp_pattern(100);
#endif
    
    /* Final volatile output */
#ifdef __cplusplus
    std::cout << "Checksum: " << checksum << std::endl;
#else
    printf("Checksum: %d\n", checksum);
#endif
    
    return 0;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif
