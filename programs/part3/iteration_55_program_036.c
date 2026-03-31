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

/* Pattern 3: TREE_BINFO - C++ class hierarchy (C++ only) */
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
        b = b + a;
        if (i % 2 == 0) {
            a = a - 0.5f;
        } else {
            b = b * 0.9f;
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
            
            /* Level 3 block with statement expression */
            result = ({
                int c = 30;
                int d = 40;
                /* Another nested block inside statement expression */
                {
                    int e = a + b + c + d;
                    e * 2;
                }
            });
            
            /* Label and goto for additional block nodes */
            void* label_ptr = &&my_label;
            goto *label_ptr;
            
            my_label:
            result += 100;
        }
    }
    
    /* Additional block with local variables */
    {
        int x = 5, y = 7, z = 9;
        result += x * y * z;
    }
    
    return result;
}

/* Pattern 6: CONSTRUCTOR - Structure and array initializers */
__attribute__((noinline))
int constructor_pattern() {
    /* Structure with designated initializers */
    struct ComplexStruct {
        int int_field;
        float float_field;
        double double_field;
        char char_field;
    };
    
    /* Multiple constructors */
    struct ComplexStruct s1 = { .int_field = 1, .float_field = 2.0f, 
                                .double_field = 3.0, .char_field = 'X' };
    
    struct ComplexStruct s2 = { 10, 20.5f, 30.75, 'Y' };
    
    /* Array initializers */
    int arr1[5] = {1, 2, 3, 4, 5};
    int arr2[3][2] = {{1, 2}, {3, 4}, {5, 6}};
    
    /* Compound literals */
    int* ptr1 = (int[]){10, 20, 30, 40};
    struct ComplexStruct* ptr2 = &(struct ComplexStruct){ 
        .int_field = 100, .float_field = 200.0f 
    };
    
    /* Nested initializers */
    struct Outer {
        struct Inner {
            int a;
            int b;
        } inner;
        int c;
    } outer = { .inner = { .a = 1, .b = 2 }, .c = 3 };
    
    return s1.int_field + arr1[2] + ptr1[1] + outer.inner.a;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int product = 1;
    int* arr = 0;
    
    /* Allocate array for OpenMP operations */
    if (size > 0) {
        arr = (int*)__builtin_alloca(size * sizeof(int));
        for (int i = 0; i < size; i++) {
            arr[i] = i + 1;
        }
    }
    
    /* Multiple OpenMP pragmas with various clauses */
    #pragma omp parallel for private(size) shared(arr) reduction(+:sum) if(size > 100)
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    #pragma omp parallel sections private(product)
    {
        #pragma omp section
        {
            product = 1;
            for (int i = 1; i <= 10; i++) {
                product *= i;
            }
        }
        
        #pragma omp section
        {
            int local_sum = 0;
            #pragma omp parallel for reduction(+:local_sum)
            for (int i = 0; i < size && i < 50; i++) {
                local_sum += arr[i];
            }
            sum += local_sum;
        }
    }
    
    /* OpenMP task with dependencies */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: arr[0]) firstprivate(size)
            {
                if (size > 0) arr[0] = 100;
            }
            
            #pragma omp task depend(in: arr[0])
            {
                sum += arr[0];
            }
        }
    }
    
    return sum + product;
}

/* Pattern 2: TREE_VEC - Vector operations function */
__attribute__((noinline))
#ifdef __GNUC__
int vector_pattern() {
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {0};
    
    /* Various vector operations */
    vec_c = vec_a + vec_b;
    vec_c = vec_c * vec_a;
    vec_c = vec_b - vec_a;
    
    v4sf vec_f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_f2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf vec_f3 = vec_f1 * vec_f2;
    
    /* Vector comparisons and blends */
    v4si mask = vec_a > vec_b;
    vec_c = __builtin_shuffle(vec_a, vec_b, (v4si){0, 1, 4, 5});
    
    /* Extract elements */
    int result = vec_c[0] + vec_c[1] + vec_c[2] + vec_c[3];
    result += (int)vec_f3[0];
    
    return result;
}
#else
int vector_pattern() {
    /* Fallback for non-GCC compilers */
    int arr[4] = {1, 2, 3, 4};
    return arr[0] + arr[1] + arr[2] + arr[3];
}
#endif

/* Pattern 3: TREE_BINFO - C++ polymorphism (C++ only) */
#ifdef __cplusplus
__attribute__((noinline))
int binfo_pattern() {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Virtual call through base pointer */
    int result = base_ptr->method();
    
    /* Dynamic cast */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        result += derived_ptr->extra_method();
    }
    
    /* Multiple inheritance-like access */
    BaseClass& base_ref = derived;
    result += base_ref.method();
    
    return result;
}
#endif

/* Main function that combines all patterns */
int main() {
    volatile int checksum = 0;
    
    /* Use IDENTIFIER_NODES through global variables */
    checksum += global_var1;
    checksum += (int)global_var2;
    checksum += global_var3;
    checksum += (int)global_var4;
    
    /* Take addresses and use sizeof for more identifier operations */
    int* ptr1 = &global_var1;
    float* ptr2 = &global_var2;
    size_t s1 = sizeof(global_var3);
    size_t s2 = sizeof(global_var4);
    checksum += *ptr1 + (int)*ptr2 + (int)s1 + (int)s2;
    
    /* Call pattern functions */
    checksum += ssa_pattern1(100);
    checksum += (int)ssa_pattern2(50);
    checksum += block_pattern();
    checksum += constructor_pattern();
    checksum += vector_pattern();
    
    /* OpenMP pattern */
    #ifdef _OPENMP
    checksum += omp_pattern(200);
    #else
    checksum += 999; /* Fallback value if OpenMP not available */
    #endif
    
    /* C++ specific patterns */
    #ifdef __cplusplus
    checksum += binfo_pattern();
    #endif
    
    /* Prevent dead code elimination */
    volatile int output = checksum;
    
    #ifdef __cplusplus
    std::cout << "Result: " << output << std::endl;
    #else
    printf("Result: %d\n", output);
    #endif
    
    return 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
