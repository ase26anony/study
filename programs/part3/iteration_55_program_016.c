/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* ========== IDENTIFIER_NODE patterns ========== */
/* Global variables for identifier creation */
int global_var_1;
double global_var_2;
char global_var_3;

/* Function declarations that require identifier lookup */
extern int external_func_1(int);
extern void external_func_2(double);
extern char* external_func_3(void);

/* Use identifiers in various contexts */
void identifier_pattern(void) __attribute__((noinline));
void identifier_pattern(void) {
    /* Local variables with distinct names */
    int local_identifier_a;
    float local_identifier_b;
    long local_identifier_c;
    
    /* Operations that create identifier nodes */
    int *ptr_a = &local_identifier_a;
    float *ptr_b = &local_identifier_b;
    long *ptr_c = &local_identifier_c;
    
    /* sizeof expressions with identifiers */
    size_t size_a = sizeof(local_identifier_a);
    size_t size_b = sizeof(local_identifier_b);
    size_t size_c = sizeof(local_identifier_c);
    
    /* Use global identifiers */
    global_var_1 = 42;
    global_var_2 = 3.14159;
    global_var_3 = 'X';
    
    /* Address-of globals */
    int *gptr1 = &global_var_1;
    double *gptr2 = &global_var_2;
    char *gptr3 = &global_var_3;
    
    /* Prevent optimization */
    volatile int dummy = size_a + size_b + size_c;
    (void)dummy;
}

/* ========== TREE_VEC patterns ========== */
#ifdef __GNUC__
/* Vector type declarations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

void vector_pattern(void) __attribute__((noinline));
void vector_pattern(void) {
    /* Vector variables */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4sf vec_c = {1.0f, 2.0f, 3.0f, 4.0f};
    v8hi vec_d = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Vector operations */
    v4si vec_sum = vec_a + vec_b;
    v4si vec_mul = vec_a * vec_b;
    v4si vec_sub = vec_a - vec_b;
    
    /* Vector comparisons */
    v4si mask = vec_a > vec_b;
    
    /* Vector shuffles/extracts */
    int first = __builtin_shuffle(vec_a, vec_b, (v4si){0, 1, 2, 3})[0];
    
    /* Prevent optimization */
    volatile int result = first + vec_sum[0] + vec_mul[1] + mask[2];
    (void)result;
}
#else
/* Fallback for non-GCC compilers */
void vector_pattern(void) __attribute__((noinline));
void vector_pattern(void) {
    /* Simple array operations */
    int arr[4] = {1, 2, 3, 4};
    volatile int sum = arr[0] + arr[1] + arr[2] + arr[3];
    (void)sum;
}
#endif

/* ========== SSA_NAME patterns ========== */
void ssa_pattern(int n) __attribute__((noinline));
void ssa_pattern(int n) {
    /* Variables that will become SSA_NAMEs */
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Loop with variable modification - forces SSA form */
    for (int i = 0; i < n; ++i) {
        x = x + i;      /* Creates phi nodes in SSA */
        y = y * (i + 1);
        if (i % 2 == 0) {
            z = z - i;  /* Conditional assignment */
        } else {
            z = z + i;  /* Different assignment in else branch */
        }
    }
    
    /* Another loop with different variable */
    int w = 100;
    for (int j = 0; j < n; ++j) {
        w = w ^ j;      /* Bitwise operation */
    }
    
    /* Nested loops for more complex SSA */
    int total = 0;
    for (int outer = 0; outer < 5; ++outer) {
        for (int inner = 0; inner < 3; ++inner) {
            total = total + outer * inner;
        }
    }
    
    /* Prevent optimization */
    volatile int final = x + y + z + w + total;
    (void)final;
}

/* ========== BLOCK patterns ========== */
void block_pattern(void) __attribute__((noinline));
void block_pattern(void) {
    /* Multiple nested blocks */
    {
        int block_var_1 = 10;
        {
            int block_var_2 = 20;
            {
                int block_var_3 = 30;
                volatile int sum = block_var_1 + block_var_2 + block_var_3;
                (void)sum;
            }
        }
    }
    
    /* GCC statement expression (creates a block) */
    int result = ({
        int temp_a = 5;
        int temp_b = 10;
        temp_a * temp_b;  /* Returns value from block */
    });
    
    /* Labels and goto (involves block nodes) */
    void *label_ptr = &&my_label;
    
    if (result > 0) {
        goto *label_ptr;
    }
    
    /* This should be skipped if goto is taken */
    result = -1;
    
my_label:
    /* Another block with local variables */
    {
        int post_label_var = 99;
        volatile int dummy = post_label_var + result;
        (void)dummy;
    }
    
    /* Switch statement with blocks in cases */
    switch (result) {
        case 50: {
            int case_var = 100;
            volatile int x = case_var;
            (void)x;
            break;
        }
        default: {
            int default_var = 200;
            volatile int y = default_var;
            (void)y;
            break;
        }
    }
}

/* ========== CONSTRUCTOR patterns ========== */
void constructor_pattern(void) __attribute__((noinline));
void constructor_pattern(void) {
    /* Structure with designated initializer */
    struct ComplexStruct {
        int id;
        float value;
        char name[20];
        double data[4];
    };
    
    struct ComplexStruct s1 = {
        .id = 1,
        .value = 3.14f,
        .name = "test",
        .data = {1.1, 2.2, 3.3, 4.4}
    };
    
    /* Array initializer */
    int matrix[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    
    /* Compound literals */
    int *arr_ptr = (int[]){10, 20, 30, 40, 50};
    struct ComplexStruct *s2 = &(struct ComplexStruct){
        .id = 2,
        .value = 2.718f,
        .name = "literal",
        .data = {5.5, 6.6, 7.7, 8.8}
    };
    
    /* Nested initializers */
    struct Outer {
        struct Inner {
            int a;
            int b;
        } inner;
        int outer_val;
    } outer = {
        .inner = {.a = 100, .b = 200},
        .outer_val = 300
    };
    
    /* Union initializer */
    union MyUnion {
        int as_int;
        float as_float;
        char as_char[4];
    } u1 = {.as_int = 0x12345678};
    
    /* Prevent optimization */
    volatile int check = s1.id + matrix[1][1] + arr_ptr[2] + s2->id + outer.inner.a + u1.as_int;
    (void)check;
}

/* ========== OpenMP patterns (OMP_CLAUSE) ========== */
#ifdef _OPENMP
#include <omp.h>

void omp_pattern(int size) __attribute__((noinline));
void omp_pattern(int size) {
    int i;
    int sum = 0;
    int *arr = (int*)malloc(size * sizeof(int));
    
    if (!arr) return;
    
    /* Initialize array */
    for (i = 0; i < size; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP parallel for with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static, 4)
    for (i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP directive with different clauses */
    int max_val = 0;
    #pragma omp parallel for private(i) shared(arr) reduction(max:max_val) num_threads(2)
    for (i = 0; i < size; i++) {
        if (arr[i] > max_val) {
            max_val = arr[i];
        }
    }
    
    /* OpenMP sections */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            int section_sum = 0;
            for (i = 0; i < size/2; i++) {
                section_sum += arr[i];
            }
            #pragma omp atomic
            sum += section_sum;
        }
        
        #pragma omp section
        {
            int section_max = 0;
            for (i = size/2; i < size; i++) {
                if (arr[i] > section_max) {
                    section_max = arr[i];
                }
            }
            #pragma omp critical
            {
                if (section_max > max_val) {
                    max_val = section_max;
                }
            }
        }
    }
    
    /* OpenMP task */
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (i = 0; i < 10; i++) {
                #pragma omp task firstprivate(i) shared(sum)
                {
                    #pragma omp atomic
                    sum += i;
                }
            }
        }
    }
    
    free(arr);
    
    /* Prevent optimization */
    volatile int result = sum + max_val;
    (void)result;
}
#else
/* Fallback for non-OpenMP compilation */
void omp_pattern(int size) __attribute__((noinline));
void omp_pattern(int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += i + 1;
    }
    volatile int result = sum;
    (void)result;
}
#endif

/* ========== Main function ========== */
int main(int argc, char **argv) {
    int iterations = 100;
    
    /* Call all pattern functions */
    identifier_pattern();
    vector_pattern();
    ssa_pattern(iterations);
    block_pattern();
    constructor_pattern();
    omp_pattern(iterations);
    
    /* Compute a checksum to prevent dead code elimination */
    volatile int final_result = 
        global_var_1 + 
        (int)global_var_2 + 
        global_var_3;
    
#ifdef __cplusplus
    std::cout << "Test completed. Result: " << final_result << std::endl;
#else
    printf("Test completed. Result: %d\n", final_result);
#endif
    
    return 0;
}

#ifdef __cplusplus
} /* extern "C" */

/* ========== C++ specific patterns (TREE_BINFO) ========== */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int get_value() const { return base_value; }
    virtual void set_value(int v) { base_value = v; }
    
protected:
    int base_value;
};

class DerivedClass : public BaseClass {
public:
    DerivedClass() : derived_value(0) {}
    virtual int get_value() const override { return base_value + derived_value; }
    virtual void set_value(int v) override { 
        base_value = v / 2;
        derived_value = v - base_value;
    }
    
private:
    int derived_value;
};

class AnotherDerived : public BaseClass {
public:
    AnotherDerived() : extra(0.0f) {}
    virtual int get_value() const override { return base_value * 2; }
    virtual void set_value(int v) override { 
        base_value = v;
        extra = v * 0.5f;
    }
    
private:
    float extra;
};

void cpp_binfo_pattern(void) __attribute__((noinline));
void cpp_binfo_pattern(void) {
    /* Create objects of different types */
    DerivedClass derived_obj;
    AnotherDerived another_obj;
    
    /* Use base class pointers (involves BINFO for dynamic dispatch) */
    BaseClass* ptr1 = &derived_obj;
    BaseClass* ptr2 = &another_obj;
    
    /* Virtual function calls */
    ptr1->set_value(42);
    ptr2->set_value(100);
    
    int val1 = ptr1->get_value();
    int val2 = ptr2->get_value();
    
    /* Dynamic cast (requires BINFO for type checking) */
    DerivedClass* casted = dynamic_cast<DerivedClass*>(ptr1);
    if (casted) {
        casted->set_value(val1 + 10);
    }
    
    /* Array of base pointers */
    BaseClass* poly_array[3];
    poly_array[0] = &derived_obj;
    poly_array[1] = &another_obj;
    poly_array[2] = new DerivedClass();
    
    /* Polymorphic iteration */
    int total = 0;
    for (int i = 0; i < 3; i++) {
        poly_array[i]->set_value(i * 20);
        total += poly_array[i]->get_value();
    }
    
    delete poly_array[2];
    
    /* Prevent optimization */
    volatile int result = val1 + val2 + total;
    (void)result;
}

/* C++ main that includes BINFO pattern */
int cpp_main(int argc, char **argv) {
    /* Call original C patterns */
    main(argc, argv);
    
    /* Call C++ specific pattern */
    cpp_binfo_pattern();
    
    return 0;
}

/* Override main for C++ compilation */
#ifndef TEST_C_ONLY
#define main cpp_main
#endif

#endif /* __cplusplus */
