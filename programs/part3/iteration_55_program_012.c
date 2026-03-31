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

/* Pattern 3: TREE_BINFO - C++ class hierarchy (if compiled as C++) */
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
        sum += i;
        prod *= i;
        if (sum > 1000) {
            sum = sum / 2;
        }
    }
    return sum * prod;
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
            
            /* Level 3 block */
            {
                int c = 30;
                result = a + b + c;
            }
        }
    }
    
    /* GCC statement expression */
    result += ({
        int x = 5;
        int y = 10;
        x * y;
    });
    
    /* Label and goto */
    void* label_ptr = &&my_label;
    goto *label_ptr;
    
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
        char name[32];
    };
    
    struct ComplexStruct cs = {
        .id = 1001,
        .values = {1.1f, 2.2f, 3.3f, 4.4f},
        .name = "Test Struct"
    };
    
    /* Array initializer */
    int matrix[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    
    /* Compound literal */
    int* arr = (int[5]){10, 20, 30, 40, 50};
    
    return cs.id + matrix[1][1] + arr[2];
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
#ifdef _OPENMP
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int* data = (int*)__builtin_alloca(size * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < size; i++) {
        data[i] = i + 1;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(i) shared(data) reduction(+:sum) schedule(static)
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    
    /* Another OpenMP directive with different clauses */
    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val) collapse(2)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int val = i * 10 + j;
            if (val > max_val) {
                max_val = val;
            }
        }
    }
    
    return sum + max_val;
}
#endif

/* Pattern 2: Vector operations function */
#ifdef __GNUC__
__attribute__((noinline))
int vector_pattern(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf f3 = f1 * f2;
    
    /* Extract elements to ensure computation */
    int result = c[0] + c[1] + c[2] + c[3];
    result += d[0] + d[1];
    
    return result;
}
#endif

/* Pattern 3: C++ polymorphism function */
#ifdef __cplusplus
__attribute__((noinline))
int cpp_binfo_pattern(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Access through base pointer to use vtable */
    int value1 = base_ptr->get_value();
    
    /* Create another derived object */
    DerivedClass* derived_ptr = new DerivedClass();
    int value2 = derived_ptr->get_value();
    
    delete derived_ptr;
    
    return value1 + value2;
}
#endif

/* Main function that calls all patterns */
int main(void) {
    volatile int checksum = 0;  /* volatile to prevent optimization */
    
    /* Use IDENTIFIER_NODE patterns */
    checksum += global_var1;
    checksum += (int)global_var2;
    checksum += global_var3;
    checksum += (int)global_var4;
    
    /* Take addresses to force identifier lookups */
    void* addr1 = &global_var1;
    void* addr2 = &global_var2;
    void* addr3 = &global_var3;
    void* addr4 = &global_var4;
    
    /* Use sizeof on identifiers */
    checksum += sizeof(global_var1);
    checksum += sizeof(global_var2);
    
    /* Call SSA pattern functions */
    checksum += ssa_pattern1(100);
    checksum += ssa_pattern2(50);
    
    /* Call block pattern */
    checksum += block_pattern();
    
    /* Call constructor pattern */
    checksum += constructor_pattern();
    
#ifdef __GNUC__
    /* Call vector pattern */
    checksum += vector_pattern();
#endif
    
#ifdef __cplusplus
    /* Call C++ binfo pattern */
    checksum += cpp_binfo_pattern();
#endif
    
#ifdef _OPENMP
    /* Call OpenMP pattern */
    checksum += omp_pattern(100);
#endif
    
    /* Print result to prevent dead code elimination */
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
