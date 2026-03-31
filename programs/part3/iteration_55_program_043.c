/* test_tree_nodes.cc - Comprehensive test to trigger tree_kind coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#endif

/* Pattern 1: IDENTIFIER_NODE - Global variables and operations */
int global_var_1 = 10;
float global_var_2 = 20.5;
char global_var_3 = 'A';
double global_var_4 = 30.75;

extern int extern_func_1(int);
extern float extern_func_2(float);
extern void extern_func_3(void);

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#else
/* Fallback for non-GCC compilers */
typedef struct { int data[4]; } v4si;
typedef struct { float data[4]; } v4sf;
#endif

/* Pattern 3: TREE_BINFO - C++ class inheritance (C++ only) */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int get_value() { return 42; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int get_value() override { return 84; }
    int derived_data;
};

class AnotherDerived : public BaseClass {
public:
    virtual int get_value() override { return 168; }
    int another_data;
};
#endif

/* Pattern 4: SSA_NAME - Functions with loops for SSA generation */
__attribute__((noinline))
int ssa_pattern_1(int n) {
    int x = 0;
    int y = 1;
    
    /* Multiple loops to create SSA_NAME nodes */
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
    }
    
    for (int j = n; j > 0; --j) {
        x = x - j;
        y = y / (j + 1);
    }
    
    int z = x;
    for (int k = 0; k < 10; ++k) {
        z = z * 2 + k;
    }
    
    return x + y + z;
}

__attribute__((noinline))
float ssa_pattern_2(float start) {
    float a = start;
    float b = start * 2.0f;
    
    /* Nested loops for complex SSA */
    for (int i = 0; i < 100; i++) {
        a = a + i * 0.1f;
        for (int j = 0; j < 10; j++) {
            b = b - j * 0.01f;
        }
    }
    
    return a + b;
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
            
            /* Another block with label */
            {
                void* ptr = &&my_label;
                goto *ptr;
my_label:
                result += 100;
            }
        }
        
        /* Block with switch */
        switch (a) {
            case 10: {
                int inner = 50;
                result += inner;
                break;
            }
            default: {
                result -= 10;
                break;
            }
        }
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
        struct {
            char c;
            short s;
        } nested;
    };
    
    /* Multiple constructor patterns */
    struct ComplexStruct cs1 = {
        .id = 1,
        .values = {1.1f, 2.2f, 3.3f, 4.4f},
        .nested = {.c = 'X', .s = 100}
    };
    
    struct ComplexStruct cs2 = {
        .id = 2,
        .values = {5.5f, 6.6f, 7.7f, 8.8f},
        .nested.c = 'Y',
        .nested.s = 200
    };
    
    /* Array initializers */
    int arr1[5] = {1, 2, 3, 4, 5};
    int arr2[3][2] = {{1, 2}, {3, 4}, {5, 6}};
    
    /* Compound literals */
    int* ptr1 = (int[]){10, 20, 30, 40};
    struct ComplexStruct* ptr2 = &(struct ComplexStruct){
        .id = 99,
        .values = {9.9f, 8.8f},
        .nested = {.c = 'Z', .s = 300}
    };
    
    /* Nested compound literal */
    float sum = ((float[2]){cs1.values[0], cs2.values[0]})[0] +
                ((float[2]){cs1.values[1], cs2.values[1]})[1];
    
    return cs1.id + cs2.id + arr1[0] + arr2[0][0] + ptr1[0] + ptr2->id + (int)sum;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int product = 1;
    float float_sum = 0.0f;
    
    /* Create array for OpenMP operations */
    int* arr = (int*)__builtin_alloca(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        arr[i] = i + 1;
    }
    
    /* Multiple OpenMP directives with various clauses */
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel for private(size) shared(arr) reduction(+:sum) schedule(static, 4)
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    /* Another parallel region with different clauses */
    #pragma omp parallel sections private(product) firstprivate(size) reduction(*:product)
    {
        #pragma omp section
        {
            product = 1;
            for (int i = 1; i <= size/2; i++) {
                product *= i;
            }
        }
        
        #pragma omp section
        {
            int temp = 1;
            for (int i = size/2 + 1; i <= size; i++) {
                temp *= i;
            }
            #pragma omp atomic
            product *= temp;
        }
    }
    
    /* Parallel with collapse clause */
    #pragma omp parallel for collapse(2) reduction(+:float_sum) ordered
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            float_sum += i * 0.1f + j * 0.01f;
            #pragma omp ordered
            {
                /* Empty ordered block */
            }
        }
    }
    
    /* Single directive with copyprivate */
    int master_value = 0;
    #pragma omp parallel
    {
        #pragma omp single copyprivate(master_value)
        {
            master_value = 42;
        }
        
        #pragma omp barrier
        
        #pragma omp critical
        {
            sum += master_value;
        }
    }
    
    return sum + product + (int)float_sum;
}

/* Pattern 2 implementation: Vector operations */
__attribute__((noinline))
int vector_pattern(void) {
#ifdef __GNUC__
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    
    /* Various vector operations */
    v4si result1 = vec1 + vec2;
    v4si result2 = vec1 * vec3;
    v4si result3 = vec2 - vec1;
    
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fresult = fvec1 * fvec2;
    
    /* Vector comparisons */
    v4si cmp = vec1 > vec2;
    
    /* Vector shuffle-like operations */
    v4si shuffled = __builtin_shuffle(vec1, vec2, (v4si){0, 4, 1, 5});
    
    return result1[0] + result2[1] + result3[2] + (int)fresult[3] + cmp[0] + shuffled[2];
#else
    return 42;
#endif
}

/* Pattern 3 implementation: C++ polymorphism */
#ifdef __cplusplus
__attribute__((noinline))
int binfo_pattern(void) {
    DerivedClass derived;
    AnotherDerived another;
    BaseClass* base1 = &derived;
    BaseClass* base2 = &another;
    
    /* Use polymorphism to trigger BINFO lookups */
    int sum = base1->get_value() + base2->get_value();
    
    /* Cast operations that involve BINFO */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base1);
    AnotherDerived* another_ptr = dynamic_cast<AnotherDerived*>(base2);
    
    if (derived_ptr && another_ptr) {
        sum += derived_ptr->derived_data + another_ptr->another_data;
    }
    
    /* Array of base pointers */
    BaseClass* bases[3];
    bases[0] = &derived;
    bases[1] = &another;
    bases[2] = nullptr;
    
    for (int i = 0; i < 2; i++) {
        sum += bases[i]->get_value();
    }
    
    return sum;
}
#endif

/* Main function that combines all patterns */
int main(void) {
    volatile int checksum = 0;
    
    /* Use IDENTIFIER_NODE patterns */
    checksum += global_var_1;
    checksum += (int)global_var_2;
    checksum += global_var_3;
    checksum += (int)global_var_4;
    
    /* Take addresses and use sizeof */
    void* addr1 = &global_var_1;
    void* addr2 = &global_var_2;
    checksum += sizeof(global_var_3);
    checksum += sizeof(global_var_4);
    
    /* Call SSA pattern functions */
    checksum += ssa_pattern_1(100);
    checksum += (int)ssa_pattern_2(5.0f);
    
    /* Call BLOCK pattern */
    checksum += block_pattern();
    
    /* Call CONSTRUCTOR pattern */
    checksum += constructor_pattern();
    
    /* Call VECTOR pattern */
    checksum += vector_pattern();
    
    /* Call OMP pattern */
    checksum += omp_pattern(50);
    
#ifdef __cplusplus
    /* Call BINFO pattern (C++ only) */
    checksum += binfo_pattern();
#endif
    
    /* Prevent dead code elimination */
    if (checksum > 0) {
        /* Use checksum to compute final result */
        int result = checksum % 1000;
        
#ifdef __cplusplus
        std::cout << "Result: " << result << std::endl;
#else
        /* Simple output for C */
        printf("Result: %d\n", result);
#endif
        
        return result;
    }
    
    return 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
