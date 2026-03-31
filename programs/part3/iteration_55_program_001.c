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
    int extra_data;
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
    
    int z = x;
    for (int j = 0; j < n; ++j) {
        z = z - j;
        x = x + z;
    }
    
    return x + y;
}

__attribute__((noinline))
float ssa_pattern2(int n) {
    float a = 1.0f;
    float b = 2.0f;
    
    for (int i = 0; i < n; ++i) {
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
                d;
            });
            
            /* Another nested block */
            {
                volatile int temp = result;
                result = temp + 100;
            }
        }
    }
    
    /* Label and goto for additional block nodes */
    void* label_ptr = &&end_label;
    
    if (x > 100) {
        goto *label_ptr;
    }
    
    /* Another statement expression */
    result += ({
        int inner = 50;
        inner * 2;
    });
    
end_label:
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
        double extra;
    };
    
    struct ComplexStruct cs = {
        .id = 1001,
        .values = {1.1f, 2.2f, 3.3f, 4.4f},
        .name = "test_struct",
        .extra = 99.99
    };
    
    /* Array initializer */
    int matrix[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    
    /* Compound literal */
    int* dynamic_array = (int[5]){10, 20, 30, 40, 50};
    
    /* Nested structure initializer */
    struct {
        struct {
            int x, y;
        } point;
        int data[2];
    } nested = {
        .point = {.x = 5, .y = 10},
        .data = {100, 200}
    };
    
    return cs.id + matrix[1][1] + dynamic_array[2] + nested.point.x;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int product = 1;
    int* arr = 0;
    
    if (size > 0) {
        arr = (int*)__builtin_alloca(size * sizeof(int));
        for (int i = 0; i < size; i++) {
            arr[i] = i + 1;
        }
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(size) shared(arr) reduction(+:sum) if(size > 100)
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP directive with different clauses */
    #pragma omp parallel sections private(product) firstprivate(size)
    {
        #pragma omp section
        {
            product = 1;
            for (int i = 1; i <= size; i++) {
                product *= i;
            }
        }
        
        #pragma omp section
        {
            int local_sum = 0;
            #pragma omp parallel for reduction(+:local_sum)
            for (int i = 0; i < size; i++) {
                local_sum += arr[i] * 2;
            }
            sum += local_sum;
        }
    }
    
    /* OpenMP critical section */
    #pragma omp critical
    {
        sum = sum * 2;
    }
    
    return sum + product;
}

/* Pattern 2: TREE_VEC - Vector operations function */
#ifdef __GNUC__
__attribute__((noinline))
v4si vector_pattern(v4si a, v4si b) {
    v4si result;
    
    /* Various vector operations */
    result = a + b;
    result = result * a;
    result = result - b;
    
    /* Vector comparisons */
    v4si mask = a > b;
    result = result & mask;
    
    /* Vector shuffling */
    result = __builtin_shuffle(result, result);
    
    return result;
}

__attribute__((noinline))
v4sf float_vector_pattern(v4sf a, v4sf b) {
    v4sf result = a * b + a / b;
    return result;
}
#endif

/* Pattern 3: TREE_BINFO - C++ polymorphism */
#ifdef __cplusplus
__attribute__((noinline))
int binfo_pattern(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Virtual function call through base pointer */
    int value = base_ptr->get_value();
    
    /* Dynamic cast */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        value += derived_ptr->get_value();
    }
    
    /* Array of base pointers */
    BaseClass* objects[3];
    objects[0] = &derived;
    
    return value;
}
#endif

/* Main function that combines all patterns */
int main(void) {
    volatile int checksum = 0;
    
    /* Use IDENTIFIER_NODE patterns */
    checksum += global_var1;
    checksum += (int)global_var2;
    checksum += global_var3;
    checksum += (int)global_var4;
    
    /* Take addresses to force identifier lookups */
    void* addr1 = &global_var1;
    void* addr2 = &global_var2;
    (void)addr1;
    (void)addr2;
    
    /* Use sizeof on identifiers */
    checksum += sizeof(global_var3);
    checksum += sizeof(global_var4);
    
    /* Call SSA pattern functions */
    checksum += ssa_pattern1(50);
    checksum += (int)ssa_pattern2(25);
    
    /* Call block pattern */
    checksum += block_pattern(75);
    
    /* Call constructor pattern */
    checksum += constructor_pattern();
    
    /* Call OpenMP pattern */
    checksum += omp_pattern(100);
    
    #ifdef __GNUC__
    /* Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec_result = vector_pattern(vec1, vec2);
    
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {2.0f, 3.0f, 4.0f, 5.0f};
    v4sf fvec_result = float_vector_pattern(fvec1, fvec2);
    
    /* Extract elements from vectors */
    for (int i = 0; i < 4; i++) {
        checksum += vec_result[i];
        checksum += (int)fvec_result[i];
    }
    #endif
    
    #ifdef __cplusplus
    /* C++ BINFO pattern */
    checksum += binfo_pattern();
    #endif
    
    /* Final output to prevent optimization */
    #ifdef __cplusplus
    std::cout << "Checksum: " << checksum << std::endl;
    #else
    printf("Checksum: %d\n", checksum);
    #endif
    
    return 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
