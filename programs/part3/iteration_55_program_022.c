/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global variables and operations */
int global_var_1 = 10;
float global_var_2 = 20.5;
double global_var_3 = 30.7;
char global_var_4 = 'A';

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Pattern 3: TREE_BINFO - C++ class hierarchy (only in C++ mode) */
#ifdef __cplusplus
class BaseClass {
public:
    virtual int get_value() { return 100; }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
public:
    virtual int get_value() override { return 200; }
};
#endif

/* Pattern 4: SSA_NAME - Functions with loops for SSA generation */
__attribute__((noinline)) 
int ssa_pattern_1(int n) {
    int x = 0;
    int y = 1;
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
    }
    
    int z = 0;
    for (int j = 0; j < n; ++j) {
        z = z + x + y;
        x = x - 1;
    }
    
    return x + y + z;
}

__attribute__((noinline))
float ssa_pattern_2(float start, int iterations) {
    float a = start;
    float b = start * 2.0f;
    
    for (int i = 0; i < iterations; ++i) {
        a = a + b * 0.5f;
        b = b - a * 0.1f;
        
        for (int j = 0; j < 5; ++j) {
            a = a + (float)j;
        }
    }
    
    return a + b;
}

/* Pattern 5: BLOCK - Nested blocks and statement expressions */
__attribute__((noinline))
int block_pattern() {
    int result = 0;
    
    /* Level 1 block */
    {
        int a = 10;
        
        /* Level 2 block */
        {
            int b = 20;
            
            /* Level 3 block */
            {
                int c = 30;
                result = a + b + c;
            }
            
            /* GCC statement expression */
            result += ({ 
                int temp = a * b;
                temp + 5;
            });
        }
        
        /* Another nested block with different variables */
        {
            float x = 3.14f;
            double y = 2.71828;
            result += (int)(x + y);
        }
    }
    
    /* Label address and goto (creates BLOCK nodes) */
    void* label_addr = &&my_label;
    goto *label_addr;
    
my_label:
    return result;
}

/* Pattern 6: CONSTRUCTOR - Structure and array initializers */
__attribute__((noinline))
int constructor_pattern() {
    /* Structure with designated initializers */
    struct ComplexStruct {
        int id;
        float values[4];
        char name[16];
        double extra;
    };
    
    struct ComplexStruct cs = {
        .id = 42,
        .values = {1.1f, 2.2f, 3.3f, 4.4f},
        .name = "test",
        .extra = 99.99
    };
    
    /* Array initializer */
    int matrix[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    
    /* Compound literals */
    int* arr = (int[5]){10, 20, 30, 40, 50};
    float* floats = (float[]){1.5f, 2.5f, 3.5f};
    
    /* Nested compound literal */
    struct Point {
        int x, y;
    };
    
    struct Point* points = (struct Point[3]){
        {.x = 1, .y = 2},
        {.x = 3, .y = 4},
        {.x = 5, .y = 6}
    };
    
    return cs.id + matrix[1][1] + arr[2] + (int)floats[1] + points[0].x;
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
    #pragma omp parallel sections private(i) reduction(max:max_val)
    {
        #pragma omp section
        {
            for (int i = 0; i < size/2; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
        }
        
        #pragma omp section
        {
            for (int i = size/2; i < size; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
        }
    }
    
    /* OpenMP parallel region with firstprivate and lastprivate */
    int thread_sum = 0;
    #pragma omp parallel firstprivate(thread_sum) lastprivate(thread_sum)
    {
        #pragma omp for
        for (int i = 0; i < size; i++) {
            thread_sum += arr[i];
        }
    }
    
    return sum + max_val + thread_sum;
}
#endif

/* Pattern 2: TREE_VEC - Vector operations function */
#ifdef __GNUC__
__attribute__((noinline))
int vector_pattern() {
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {0};
    
    /* Vector arithmetic */
    vec_c = vec_a + vec_b;
    vec_c = vec_c * vec_a;
    
    v4sf fvec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec_b = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fvec_c = fvec_a * fvec_b;
    
    /* Vector comparisons and blending */
    v4si mask = vec_a > (v4si){2, 2, 2, 2};
    vec_c = __builtin_shuffle(vec_a, vec_b, (v4si){0, 4, 1, 5});
    
    return vec_c[0] + vec_c[1] + vec_c[2] + vec_c[3] + 
           (int)fvec_c[0] + (int)fvec_c[1];
}
#endif

/* Pattern 3: TREE_BINFO - C++ polymorphism function */
#ifdef __cplusplus
__attribute__((noinline))
int binfo_pattern() {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Virtual function call - involves BINFO lookup */
    int value1 = base_ptr->get_value();
    
    /* Dynamic cast - involves BINFO */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    int value2 = derived_ptr ? derived_ptr->get_value() : 0;
    
    /* Multiple inheritance scenario */
    class Base2 {
    public:
        virtual int get_value2() { return 300; }
    };
    
    class MultiDerived : public BaseClass, public Base2 {
    public:
        virtual int get_value() override { return 400; }
        virtual int get_value2() override { return 500; }
    };
    
    MultiDerived md;
    BaseClass* bc_ptr = &md;
    Base2* b2_ptr = &md;
    
    int value3 = bc_ptr->get_value();
    int value4 = b2_ptr->get_value2();
    
    return value1 + value2 + value3 + value4;
}
#endif

/* Main function that combines all patterns */
int main() {
    volatile int checksum = 0;
    
    /* Use IDENTIFIER_NODE patterns */
    checksum += global_var_1;
    checksum += (int)global_var_2;
    checksum += (int)global_var_3;
    checksum += global_var_4;
    
    /* Take addresses and use sizeof */
    void* addr1 = &global_var_1;
    void* addr2 = &global_var_2;
    checksum += sizeof(global_var_3);
    checksum += sizeof(global_var_4);
    
    /* Call SSA pattern functions */
    checksum += ssa_pattern_1(100);
    checksum += (int)ssa_pattern_2(10.0f, 50);
    
    /* Call BLOCK pattern */
    checksum += block_pattern();
    
    /* Call CONSTRUCTOR pattern */
    checksum += constructor_pattern();
    
#ifdef __GNUC__
    /* Call VECTOR pattern */
    checksum += vector_pattern();
#endif
    
#ifdef __cplusplus
    /* Call BINFO pattern */
    checksum += binfo_pattern();
#endif
    
#ifdef _OPENMP
    /* Call OMP pattern */
    checksum += omp_pattern(1000);
#endif
    
    /* Prevent dead code elimination */
#ifdef __cplusplus
    std::cout << "Final checksum: " << checksum << std::endl;
#else
    printf("Final checksum: %d\n", checksum);
#endif
    
    return checksum != 0 ? 0 : 1;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
