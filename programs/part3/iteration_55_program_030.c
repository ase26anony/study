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

/* Pattern 3: TREE_BINFO - C++ class hierarchy */
#ifdef __cplusplus
class BaseClass {
public:
    virtual int get_value() { return 42; }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
public:
    virtual int get_value() override { return 84; }
    int extra_value = 100;
};
#endif

/* Pattern 4: SSA_NAME - Functions with loops for SSA generation */
__attribute__((noinline))
int ssa_pattern1(int n) {
    int x = 0;
    int y = 1;
    /* Multiple loops to create SSA names */
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
        z = z + 2;
    }
    
    return x + y;
}

__attribute__((noinline))
float ssa_pattern2(float start, int iterations) {
    float a = start;
    float b = start * 2.0f;
    
    for (int i = 0; i < iterations; ++i) {
        a = a * 1.1f;
        b = b / 1.1f;
        if (i % 2 == 0) {
            a = a + b;
        } else {
            b = b - a;
        }
    }
    
    return a + b;
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
                int c = b * 3;
                int d = c - 5;
                d;  /* Returns d */
            });
            
            /* Label and goto for block nodes */
            if (result > 100) {
                goto skip_block;
            }
        }
        
        /* Another nested block */
        {
            volatile int temp = 7;
            result += temp;
        }
    }
    
skip_block:
    /* GCC's computed goto with address of label */
    void* label_ptr = &&end_label;
    
    /* Final block with mixed declarations */
    {
        int final_val = result;
        float final_float = final_val * 1.5f;
        result = (int)final_float;
    }
    
end_label:
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
        double extra;
    };
    
    struct ComplexStruct s1 = {
        .id = 1,
        .values = {1.1f, 2.2f, 3.3f, 4.4f},
        .name = "test",
        .extra = 99.99
    };
    
    /* Array with initializer */
    int matrix[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    
    /* Compound literals */
    int* arr = (int[]){10, 20, 30, 40, 50};
    
    /* Nested compound literal */
    struct Point {
        int x, y;
    };
    
    struct Point* points = (struct Point[]){
        {.x = 1, .y = 2},
        {.x = 3, .y = 4},
        {.x = 5, .y = 6}
    };
    
    /* Union with initializer */
    union Data {
        int i;
        float f;
        char str[4];
    } data = {.i = 0x12345678};
    
    return s1.id + matrix[1][1] + arr[2] + points[0].x + data.i;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives with various clauses */
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int product = 1;
    int* array = 0;
    
    if (size > 0) {
        array = (int*)__builtin_alloca(size * sizeof(int));
        
        /* Initialize array */
        for (int i = 0; i < size; ++i) {
            array[i] = i + 1;
        }
        
        /* OpenMP parallel region with multiple clauses */
        #pragma omp parallel for private(i) shared(array, size) reduction(+:sum) reduction(*:product) schedule(static, 4)
        for (int i = 0; i < size; ++i) {
            sum += array[i];
            product *= array[i];
        }
        
        /* Another OpenMP section with different clauses */
        int max_val = 0;
        int min_val = 1000000;
        
        #pragma omp parallel sections private(i) firstprivate(array, size) lastprivate(max_val, min_val)
        {
            #pragma omp section
            {
                for (int i = 0; i < size/2; ++i) {
                    if (array[i] > max_val) max_val = array[i];
                }
            }
            
            #pragma omp section
            {
                for (int i = size/2; i < size; ++i) {
                    if (array[i] < min_val) min_val = array[i];
                }
            }
        }
        
        /* OpenMP single directive */
        #pragma omp parallel
        {
            #pragma omp single nowait
            {
                sum += max_val - min_val;
            }
        }
    }
    
    return sum + product;
}

/* Pattern 2: Vector operations function */
__attribute__((noinline))
#ifdef __GNUC__
int vector_pattern(void) {
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c;
    
    /* Various vector operations */
    vec_c = vec_a + vec_b;
    vec_c = vec_c * vec_a;
    vec_c = vec_c - vec_b;
    
    v4sf fvec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec_b = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fvec_c = fvec_a * fvec_b;
    
    /* Use vectors in expressions */
    int result = 0;
    for (int i = 0; i < 4; ++i) {
        result += vec_c[i] + (int)fvec_c[i];
    }
    
    return result;
}
#else
int vector_pattern(void) {
    /* Fallback for non-GCC compilers */
    int vec_a[4] = {1, 2, 3, 4};
    int vec_b[4] = {5, 6, 7, 8};
    int result = 0;
    
    for (int i = 0; i < 4; ++i) {
        result += vec_a[i] + vec_b[i];
    }
    
    return result;
}
#endif

/* Pattern 3: C++ BINFO pattern function */
#ifdef __cplusplus
__attribute__((noinline))
int binfo_pattern(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Access through base pointer - involves BINFO */
    int value1 = base_ptr->get_value();
    
    /* Dynamic cast - involves BINFO */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    int value2 = derived_ptr ? derived_ptr->get_value() : 0;
    
    /* Multiple inheritance scenario */
    class AnotherBase {
    public:
        virtual int another_value() { return 200; }
    };
    
    class MultiDerived : public BaseClass, public AnotherBase {
    public:
        virtual int get_value() override { return 168; }
        virtual int another_value() override { return 300; }
    };
    
    MultiDerived multi;
    BaseClass* base1 = &multi;
    AnotherBase* base2 = &multi;
    
    int value3 = base1->get_value();
    int value4 = base2->another_value();
    
    return value1 + value2 + value3 + value4 + multi.extra_value;
}
#endif

/* Main function that combines all patterns */
int main(void) {
    volatile int checksum = 0;
    
    /* Use global variables (IDENTIFIER_NODE) in various ways */
    checksum += global_var1;
    checksum += (int)global_var2;
    checksum += global_var3;
    checksum += (int)global_var4;
    
    /* Take addresses of globals */
    int* ptr1 = &global_var1;
    float* ptr2 = &global_var2;
    
    /* Use sizeof on identifiers */
    checksum += sizeof(global_var1);
    checksum += sizeof(global_var3);
    
    /* Call SSA pattern functions */
    checksum += ssa_pattern1(10);
    checksum += (int)ssa_pattern2(5.0f, 8);
    
    /* Call block pattern */
    checksum += block_pattern(15);
    
    /* Call constructor pattern */
    checksum += constructor_pattern();
    
    /* Call vector pattern */
    checksum += vector_pattern();
    
    /* Call OpenMP pattern if supported */
    #ifdef _OPENMP
    checksum += omp_pattern(8);
    #endif
    
    /* Call C++ BINFO pattern if in C++ mode */
    #ifdef __cplusplus
    checksum += binfo_pattern();
    
    /* Additional C++ specific identifier usage */
    DerivedClass local_derived;
    checksum += local_derived.get_value();
    #endif
    
    /* Use all pointer variables to prevent optimization */
    checksum += *ptr1;
    checksum += (int)*ptr2;
    
    /* Final output to prevent dead code elimination */
    #ifdef __cplusplus
    std::cout << "Final checksum: " << checksum << std::endl;
    #else
    printf("Final checksum: %d\n", checksum);
    #endif
    
    return checksum > 0 ? 0 : 1;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif
