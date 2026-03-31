/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* ========== PATTERN 1: IDENTIFIER_NODE ========== */
/* Global variables to force identifier creation */
int global_var_1;
float global_var_2;
double global_var_3;
char global_var_4;

/* Function taking address of identifiers */
__attribute__((noinline))
int pattern_identifiers(void) {
    /* Local variables with distinct names */
    int local_identifier_1 = 1;
    float local_identifier_2 = 2.0f;
    double local_identifier_3 = 3.0;
    char local_identifier_4 = 'A';
    
    /* Operations that require identifier lookup */
    volatile int *addr1 = &global_var_1;
    volatile float *addr2 = &global_var_2;
    volatile double *addr3 = &global_var_3;
    volatile char *addr4 = &global_var_4;
    
    /* sizeof expressions with identifiers */
    size_t s1 = sizeof(global_var_1);
    size_t s2 = sizeof(local_identifier_1);
    
    /* Use in expressions */
    int result = local_identifier_1 + global_var_1;
    result += (int)local_identifier_2;
    result += (int)local_identifier_3;
    result += local_identifier_4;
    
    /* Prevent optimization */
    asm volatile("" : "+r"(result));
    return result;
}

/* ========== PATTERN 2: TREE_VEC ========== */
#ifdef __GNUC__
/* Vector type declarations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

__attribute__((noinline))
int pattern_vectors(void) {
    /* Vector variables */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Vector operations */
    v4si vec_sum = vec1 + vec2;
    v4si vec_mul = vec1 * vec2;
    v4sf fvec_sum = fvec1 + fvec2;
    
    /* Extract elements */
    int sum = vec_sum[0] + vec_sum[1] + vec_sum[2] + vec_sum[3];
    sum += vec_mul[0] + vec_mul[1];
    
    /* Vector comparisons */
    v4si cmp = vec1 > vec2;
    sum += cmp[0] + cmp[1] + cmp[2] + cmp[3];
    
    /* Prevent optimization */
    asm volatile("" : "+r"(sum));
    return sum;
}
#else
__attribute__((noinline))
int pattern_vectors(void) {
    /* Fallback for non-GCC compilers */
    int arr1[4] = {1, 2, 3, 4};
    int arr2[4] = {5, 6, 7, 8};
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += arr1[i] + arr2[i];
    }
    return sum;
}
#endif

/* ========== PATTERN 3: SSA_NAME ========== */
__attribute__((noinline))
int pattern_ssa(int n) {
    /* Variables that will get SSA names */
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Loop that forces SSA form */
    for (int i = 0; i < n; ++i) {
        x = x + i;      /* Creates phi nodes in SSA */
        y = y * (i + 1);
        if (i % 2 == 0) {
            z = z - i;
        } else {
            z = z + i;
        }
    }
    
    /* Another loop with different variable */
    int w = 100;
    for (int j = 0; j < n; ++j) {
        w = w - j;
        for (int k = 0; k < 3; ++k) {
            w = w + k;  /* Nested loop creates more SSA complexity */
        }
    }
    
    /* Complex control flow */
    int result = x;
    if (y > 0) {
        result += y;
    } else {
        result -= y;
    }
    
    result += z;
    result += w;
    
    /* Prevent optimization */
    asm volatile("" : "+r"(result));
    return result;
}

/* ========== PATTERN 4: BLOCK ========== */
__attribute__((noinline))
int pattern_blocks(int val) {
    int result = val;
    
    /* Level 1 block */
    {
        int block_var_1 = result * 2;
        
        /* Level 2 nested block */
        {
            int block_var_2 = block_var_1 + 10;
            
            /* Level 3 deeply nested block */
            {
                int block_var_3 = block_var_2 - 5;
                result = block_var_3;
                
                /* GCC statement expression (creates a block) */
                int stmt_expr = ({
                    int temp = block_var_3;
                    temp * temp;
                });
                result += stmt_expr;
            }
        }
    }
    
    /* Another block with label address */
    {
        void *label_ptr = &&my_label;
        goto *label_ptr;
        
        my_label:
        result += 100;
    }
    
    /* Switch with blocks in cases */
    switch (result % 3) {
        case 0: {
            int case_var = result + 1;
            result = case_var * 2;
            break;
        }
        case 1: {
            int case_var = result - 1;
            result = case_var / 2;
            break;
        }
        default: {
            int case_var = result * 3;
            result = case_var;
            break;
        }
    }
    
    /* Prevent optimization */
    asm volatile("" : "+r"(result));
    return result;
}

/* ========== PATTERN 5: CONSTRUCTOR ========== */
__attribute__((noinline))
int pattern_constructors(void) {
    /* Structure with designated initializer */
    struct ComplexStruct {
        int a;
        float b;
        double c;
        char d;
        int e[3];
    };
    
    struct ComplexStruct s1 = {
        .a = 42,
        .b = 3.14f,
        .c = 2.71828,
        .d = 'X',
        .e = {1, 2, 3}
    };
    
    /* Array with initializer */
    int arr_init[5] = {10, 20, 30, 40, 50};
    
    /* Compound literals */
    int *ptr = (int[3]){100, 200, 300};
    struct ComplexStruct *s2 = &(struct ComplexStruct){
        .a = 99,
        .b = 1.5f,
        .c = 3.0,
        .d = 'Y',
        .e = {4, 5, 6}
    };
    
    /* Nested initializers */
    struct Outer {
        struct Inner {
            int x;
            int y;
        } inner;
        int z;
    } outer = {
        .inner = {.x = 7, .y = 8},
        .z = 9
    };
    
    /* Compute result using all constructors */
    int result = s1.a + (int)s1.b + (int)s1.c + s1.d;
    for (int i = 0; i < 3; i++) {
        result += s1.e[i];
    }
    
    for (int i = 0; i < 5; i++) {
        result += arr_init[i];
    }
    
    result += ptr[0] + ptr[1] + ptr[2];
    result += s2->a + (int)s2->b;
    result += outer.inner.x + outer.inner.y + outer.z;
    
    /* Prevent optimization */
    asm volatile("" : "+r"(result));
    return result;
}

/* ========== PATTERN 6: OMP_CLAUSE ========== */
#ifdef _OPENMP
__attribute__((noinline))
int pattern_openmp(int size) {
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP parallel for with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static, 10)
    for (int i = 0; i < size && i < 100; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP section with different clauses */
    int max_val = 0;
    int min_val = 1000;
    
    #pragma omp parallel sections private(i) reduction(max:max_val) reduction(min:min_val)
    {
        #pragma omp section
        {
            for (int i = 0; i < size/2 && i < 100; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
        }
        
        #pragma omp section
        {
            for (int i = size/2; i < size && i < 100; i++) {
                if (arr[i] < min_val) min_val = arr[i];
            }
        }
    }
    
    /* OpenMP parallel with firstprivate and lastprivate */
    int thread_sum = 0;
    #pragma omp parallel firstprivate(thread_sum) lastprivate(thread_sum)
    {
        #pragma omp for
        for (int i = 0; i < size && i < 100; i++) {
            thread_sum += arr[i];
        }
    }
    
    int result = sum + max_val + min_val + thread_sum;
    
    /* Prevent optimization */
    asm volatile("" : "+r"(result));
    return result;
}
#else
__attribute__((noinline))
int pattern_openmp(int size) {
    /* Fallback without OpenMP */
    int sum = 0;
    for (int i = 0; i < size && i < 100; i++) {
        sum += i + 1;
    }
    return sum;
}
#endif

/* ========== C++ SECTION FOR TREE_BINFO ========== */
#ifdef __cplusplus
}

/* Base and derived classes for BINFO nodes */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int get_value() const { return base_value; }
    virtual void set_value(int v) { base_value = v; }
    
private:
    int base_value = 100;
};

class DerivedClass : public BaseClass {
public:
    virtual int get_value() const override { return derived_value; }
    virtual void set_value(int v) override { derived_value = v; }
    
    int get_double() const { return derived_value * 2; }
    
private:
    int derived_value = 200;
};

class AnotherDerived : public BaseClass {
public:
    virtual int get_value() const override { return another_value; }
    
private:
    int another_value = 300;
};

__attribute__((noinline))
int pattern_binfo(void) {
    DerivedClass derived_obj;
    AnotherDerived another_obj;
    
    /* Use base class pointers (involves BINFO for dynamic dispatch) */
    BaseClass* base_ptr1 = &derived_obj;
    BaseClass* base_ptr2 = &another_obj;
    
    /* Virtual function calls */
    int val1 = base_ptr1->get_value();
    base_ptr1->set_value(250);
    val1 = base_ptr1->get_value();
    
    int val2 = base_ptr2->get_value();
    
    /* Casts that involve BINFO */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr1);
    if (derived_ptr) {
        val1 += derived_ptr->get_double();
    }
    
    /* Array of base pointers */
    BaseClass* poly_array[3];
    poly_array[0] = &derived_obj;
    poly_array[1] = &another_obj;
    poly_array[2] = base_ptr1;
    
    int result = 0;
    for (int i = 0; i < 3; i++) {
        result += poly_array[i]->get_value();
    }
    
    result += val1 + val2;
    
    /* Prevent optimization */
    asm volatile("" : "+r"(result));
    return result;
}

extern "C" {
#endif

/* ========== MAIN FUNCTION ========== */
int main(void) {
    int checksum = 0;
    
    /* Call all pattern functions */
    checksum += pattern_identifiers();
    checksum += pattern_vectors();
    checksum += pattern_ssa(50);
    checksum += pattern_blocks(42);
    checksum += pattern_constructors();
    checksum += pattern_openmp(75);
    
#ifdef __cplusplus
    checksum += pattern_binfo();
#endif
    
    /* Volatile output to prevent dead code elimination */
    volatile int final_result = checksum;
    
#ifdef __cplusplus
    std::cout << "Result: " << final_result << std::endl;
#else
    printf("Result: %d\n", final_result);
#endif
    
    return 0;
}

#ifdef __cplusplus
}
#endif
