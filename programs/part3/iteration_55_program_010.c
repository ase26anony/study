/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global variables with various uses */
int global_var_1 = 10;
float global_var_2 = 20.5;
char global_var_3 = 'A';
double global_var_4 = 30.75;

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
    return x + y;
}

__attribute__((noinline))
int ssa_pattern_2(int n) {
    int a = 0, b = 1, c = 2;
    for (int i = 0; i < n; ++i) {
        a = a + b;
        b = b + c;
        c = c + a;
        if (i % 2 == 0) {
            a = a * 2;
        }
    }
    return a + b + c;
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
        }
    }
    
    /* GCC statement expression */
    result += ({
        int temp = 100;
        temp * 2;
    });
    
    /* Labels and goto for block creation */
    void* label_ptr = &&my_label;
    goto *label_ptr;
    
my_label:
    return result;
}

/* Pattern 6: CONSTRUCTOR - Structure and array initializers */
__attribute__((noinline))
int constructor_pattern() {
    /* Structure with designated initializers */
    struct ComplexStruct {
        int id;
        float values[3];
        char name[20];
    };
    
    struct ComplexStruct cs = {
        .id = 1001,
        .values = {1.1f, 2.2f, 3.3f},
        .name = "test_struct"
    };
    
    /* Array initializer */
    int matrix[2][3] = {
        {1, 2, 3},
        {4, 5, 6}
    };
    
    /* Compound literal */
    int* arr = (int[5]){10, 20, 30, 40, 50};
    
    /* Nested initializer */
    struct Nested {
        struct {
            int x, y;
        } point;
        int data[2];
    } nested = {
        .point = {.x = 1, .y = 2},
        .data = {100, 200}
    };
    
    return cs.id + matrix[1][2] + arr[3] + nested.point.x;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int product = 1;
    
    #pragma omp parallel for private(size) shared(sum) reduction(+:sum) if(size > 100)
    for (int i = 0; i < size; ++i) {
        sum += i;
    }
    
    int arr[100];
    #pragma omp parallel for simd private(product) shared(arr) reduction(*:product) collapse(2)
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            arr[i * 10 + j] = i * j;
            product *= (i + j + 1);
        }
    }
    
    #pragma omp task depend(inout: sum) firstprivate(product)
    {
        sum += product;
    }
    
    return sum;
}

/* Vector operations function */
__attribute__((noinline))
int vector_pattern() {
#ifdef __GNUC__
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fvec3 = fvec1 * fvec2;
    
    return vec3[0] + vec4[1] + (int)fvec3[2];
#else
    return 0;
#endif
}

/* C++ polymorphism function */
#ifdef __cplusplus
__attribute__((noinline))
int cpp_binfo_pattern() {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* This should involve TREE_BINFO nodes */
    int value1 = base_ptr->get_value();
    int value2 = dynamic_cast<DerivedClass*>(base_ptr)->get_value();
    
    return value1 + value2;
}
#endif

/* Main function that combines all patterns */
int main() {
    volatile int checksum = 0;
    
    /* Use IDENTIFIER_NODE patterns */
    checksum += global_var_1;
    checksum += (int)global_var_2;
    checksum += global_var_3;
    checksum += (int)global_var_4;
    
    /* Take addresses to force identifier lookups */
    void* addr1 = (void*)&global_var_1;
    void* addr2 = (void*)&global_var_2;
    
    /* Use sizeof on identifiers */
    checksum += sizeof(global_var_3);
    
    /* Call SSA pattern functions */
    checksum += ssa_pattern_1(100);
    checksum += ssa_pattern_2(50);
    
    /* Call block pattern */
    checksum += block_pattern();
    
    /* Call constructor pattern */
    checksum += constructor_pattern();
    
    /* Call vector pattern */
    checksum += vector_pattern();
    
#ifdef __cplusplus
    /* Call C++ BINFO pattern */
    checksum += cpp_binfo_pattern();
#endif
    
    /* Call OpenMP pattern */
    checksum += omp_pattern(200);
    
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
