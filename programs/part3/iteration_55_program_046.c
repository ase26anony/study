/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global variables with various uses */
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
    int a = 1, b = 2, c = 3;
    for (int i = 0; i < n; ++i) {
        a = b + c;
        b = c - a;
        c = a * b;
    }
    return a + b + c;
}

/* Pattern 5: BLOCK - Nested blocks and statement expressions */
__attribute__((noinline))
int block_pattern(void) {
    int result = 0;
    
    /* Level 1 block */
    {
        int inner1 = 10;
        
        /* Level 2 block */
        {
            int inner2 = 20;
            
            /* Level 3 block */
            {
                int inner3 = 30;
                result = inner1 + inner2 + inner3;
            }
        }
    }
    
    /* GCC statement expression */
    result += ({
        int temp = 5;
        temp * 2;
    });
    
    /* Labels and goto for additional block nodes */
    void* label_ptr = &&my_label;
    goto *label_ptr;
    
my_label:
    return result;
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
    
    /* Compound literal */
    int sum = 0;
    sum += ((int[3]){1, 2, 3})[0];
    sum += ((int[3]){4, 5, 6})[1];
    
    /* Nested initializer */
    struct Nested {
        struct Point p;
        int id;
    } nested = { .p = {10, 20, 30.0}, .id = 100 };
    
    return p1.x + p2.y + arr1[0] + arr2[1] + sum + nested.id;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
#ifdef _OPENMP
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel for with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static)
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP directive with different clauses */
    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val) collapse(2)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int idx = i * 10 + j;
            if (arr[idx] > max_val) {
                max_val = arr[idx];
            }
        }
    }
    
    /* OpenMP parallel region */
    #pragma omp parallel num_threads(4) default(none) shared(arr, sum)
    {
        #pragma omp single
        {
            sum += 1000;
        }
    }
    
    return sum + max_val;
}
#endif

/* Pattern 2: TREE_VEC - Vector operations function */
#ifdef __GNUC__
__attribute__((noinline))
int vector_pattern(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    v4sf e = {1.0, 2.0, 3.0, 4.0};
    v4sf f = {5.0, 6.0, 7.0, 8.0};
    v4sf g = e + f;
    
    /* Extract elements to force usage */
    int result = 0;
    for (int i = 0; i < 4; i++) {
        result += c[i] + d[i] + (int)g[i];
    }
    
    return result;
}
#endif

/* Pattern 3: TREE_BINFO - C++ polymorphism function */
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
    
    /* Array of base pointers */
    BaseClass* base_array[3];
    base_array[0] = &derived;
    
    return result;
}
#endif

/* Pattern 1: IDENTIFIER_NODE - Function to use identifiers in various ways */
__attribute__((noinline))
int identifier_pattern(void) {
    /* Local variables with distinct names */
    int local_counter = 0;
    float local_ratio = 1.5;
    double local_precision = 0.0001;
    char local_char = 'Z';
    
    /* Use sizeof on identifiers */
    local_counter += sizeof(global_var1);
    local_counter += sizeof(local_ratio);
    
    /* Take addresses */
    int* ptr1 = &global_var2;
    float* ptr2 = &local_ratio;
    
    /* Use in expressions */
    local_counter += *ptr1 + (int)*ptr2;
    
    /* Declare external function to force identifier lookup */
    extern int external_func(int);
    
    /* Use all global identifiers */
    local_counter += global_var1 + (int)global_var3 + (int)global_var4 + global_var5;
    
    return local_counter;
}

/* Main function that calls all patterns */
int main(void) {
    volatile int total = 0;  /* volatile to prevent optimization */
    
    /* Pattern 1: IDENTIFIER_NODE */
    total += identifier_pattern();
    
    /* Pattern 2: TREE_VEC */
#ifdef __GNUC__
    total += vector_pattern();
#endif
    
    /* Pattern 3: TREE_BINFO */
#ifdef __cplusplus
    total += binfo_pattern();
#endif
    
    /* Pattern 4: SSA_NAME */
    total += ssa_pattern1(100);
    total += ssa_pattern2(50);
    
    /* Pattern 5: BLOCK */
    total += block_pattern();
    
    /* Pattern 6: CONSTRUCTOR */
    total += constructor_pattern();
    
    /* Pattern 7: OMP_CLAUSE */
#ifdef _OPENMP
    total += omp_pattern(100);
#endif
    
    /* Use all global variables to ensure they're not optimized away */
    total += global_var1 + global_var2 + (int)global_var3 + (int)global_var4 + global_var5;
    
#ifdef __cplusplus
    std::cout << "Result: " << total << std::endl;
#else
    printf("Result: %d\n", total);
#endif
    
    return 0;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif
