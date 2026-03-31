/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* ========== IDENTIFIER_NODE patterns ========== */
/* Global variables to force identifier creation */
int global_var_1;
float global_var_2;
double global_var_3;
char global_var_4;

/* Function declarations using identifiers */
extern int external_func_1(int);
extern void external_func_2(float);
extern double external_func_3(void);

/* Use identifiers in various contexts */
__attribute__((noinline))
int identifier_pattern(void) {
    /* Local variables */
    int local_var_1 = 10;
    float local_var_2 = 20.5f;
    
    /* Taking addresses of identifiers */
    int *ptr1 = &global_var_1;
    float *ptr2 = &local_var_2;
    
    /* sizeof expressions with identifiers */
    size_t s1 = sizeof(global_var_3);
    size_t s2 = sizeof(local_var_1);
    
    /* Complex expressions with identifiers */
    int result = global_var_1 + local_var_1 * 2;
    result += (int)(global_var_2 * local_var_2);
    
    /* Use in conditional expressions */
    if (global_var_4 || local_var_1) {
        result += 100;
    }
    
    /* Function-like macro with identifiers */
#define USE_IDENT(x) ((x) * 2)
    result += USE_IDENT(local_var_1);
    
    return result;
}

/* ========== TREE_VEC patterns ========== */
#ifdef __GNUC__
/* Vector type declarations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

__attribute__((noinline))
int vector_pattern(void) {
    /* Vector variable declarations and initializations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Vector operations */
    v4si vec_sum = vec1 + vec2;
    v4si vec_diff = vec1 - vec2;
    v4si vec_prod = vec1 * vec2;
    v4sf fvec_sum = fvec1 + fvec2;
    
    /* Vector comparisons */
    v4si mask = vec1 > vec2;
    
    /* Vector shuffles/extracts */
    int first_elem = vec1[0];
    vec1[3] = 99;
    
    /* Use vectors in function arguments */
    v4si vec_result = vec_sum + vec_diff;
    
    /* Complex vector expression */
    v4si final_vec = (vec1 * 2) + (vec2 / 2);
    
    /* Return sum of all elements */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += vec_result[i];
        sum += final_vec[i];
    }
    
    return sum + first_elem;
}
#else
__attribute__((noinline))
int vector_pattern(void) {
    /* Fallback for non-GCC compilers */
    int arr[4] = {1, 2, 3, 4};
    return arr[0] + arr[1] + arr[2] + arr[3];
}
#endif

/* ========== SSA_NAME patterns ========== */
__attribute__((noinline))
int ssa_pattern(int n) {
    /* Variables that will get SSA names */
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Loop with variable modification - forces SSA */
    for (int i = 0; i < n; ++i) {
        x = x + i;      /* Creates phi nodes in SSA */
        y = y * 2 + x;  /* Complex dependency */
    }
    
    /* Nested loop with more SSA variables */
    for (int j = 0; j < 10; ++j) {
        for (int k = 0; k < 5; ++k) {
            z = z + j * k;
            x = x - 1;
        }
        y = y + z;
    }
    
    /* Conditional with SSA */
    int w = 0;
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            w = w + i;
        } else {
            w = w - i;
        }
    }
    
    /* Complex expression with multiple assignments */
    int result = x;
    for (int i = 0; i < 100; i++) {
        result = result * 1103515245 + 12345;
        result = (result / 65536) % 32768;
    }
    
    return x + y + z + w + result;
}

/* ========== BLOCK patterns ========== */
__attribute__((noinline))
int block_pattern(void) {
    int result = 0;
    
    /* Level 1 block */
    {
        int a = 10;
        result += a;
        
        /* Level 2 nested block */
        {
            int b = 20;
            result += b;
            
            /* Level 3 nested block */
            {
                int c = 30;
                result += c;
                
                /* GCC statement expression - creates a block */
                int d = ({
                    int temp = 40;
                    temp * 2;
                });
                result += d;
            }
        }
    }
    
    /* Another block with different variables */
    {
        float f1 = 1.5f;
        double d1 = 2.5;
        result += (int)(f1 + d1);
    }
    
    /* Block with gotos and labels */
    {
        void *label_ptr;
        
        /* Taking address of label */
        label_ptr = &&my_label;
        
        /* Use in computed goto */
        goto *label_ptr;
        
    my_label:
        result += 100;
    }
    
    /* Switch statement with blocks in cases */
    switch (result % 3) {
        case 0: {
            int case_var = 111;
            result += case_var;
            break;
        }
        case 1: {
            int case_var = 222;
            result += case_var;
            break;
        }
        case 2: {
            int case_var = 333;
            result += case_var;
            break;
        }
    }
    
    return result;
}

/* ========== CONSTRUCTOR patterns ========== */
__attribute__((noinline))
int constructor_pattern(void) {
    int result = 0;
    
    /* Structure with designated initializer */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Point p1 = { .x = 1, .y = 2, .z = 3 };
    result += p1.x + p1.y + p1.z;
    
    /* Array initializer */
    int arr1[5] = { 10, 20, 30, 40, 50 };
    for (int i = 0; i < 5; i++) {
        result += arr1[i];
    }
    
    /* Nested structure initializer */
    struct Nested {
        struct Point p;
        float f;
        char c;
    };
    
    struct Nested n1 = { 
        .p = { .x = 100, .y = 200, .z = 300 },
        .f = 3.14f,
        .c = 'A'
    };
    result += n1.p.x + n1.p.y + (int)n1.f;
    
    /* Compound literal - creates CONSTRUCTOR node */
    result += ((int[3]){1, 2, 3})[0];
    result += ((int[3]){1, 2, 3})[1];
    result += ((int[3]){1, 2, 3})[2];
    
    /* Union with initializer */
    union Data {
        int i;
        float f;
        char str[20];
    };
    
    union Data d1 = { .i = 999 };
    result += d1.i;
    
    /* Complex initializer with designators */
    struct Complex {
        int a[3];
        struct {
            float x;
            float y;
        } point;
        char name[10];
    };
    
    struct Complex c1 = {
        .a = {1, 2, 3},
        .point = { .x = 1.5f, .y = 2.5f },
        .name = "test"
    };
    
    for (int i = 0; i < 3; i++) {
        result += c1.a[i];
    }
    
    return result;
}

/* ========== OMP_CLAUSE patterns ========== */
#ifdef _OPENMP
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int product = 1;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(size) shared(arr) reduction(+:sum) schedule(static, 10)
    for (int i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP directive with different clauses */
    #pragma omp parallel sections private(product) firstprivate(size) num_threads(2)
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
            #pragma omp parallel for reduction(+:local_sum) if(size > 50)
            for (int i = 0; i < 50; i++) {
                local_sum += arr[i];
            }
            sum += local_sum;
        }
    }
    
    /* OpenMP critical section */
    #pragma omp parallel
    {
        #pragma omp critical
        {
            sum += 1000;
        }
    }
    
    /* OpenMP atomic operation */
    #pragma omp parallel for
    for (int i = 0; i < 100; i++) {
        #pragma omp atomic
        sum += 1;
    }
    
    return sum + product;
}
#else
__attribute__((noinline))
int omp_pattern(int size) {
    /* Fallback without OpenMP */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += i;
    }
    return sum;
}
#endif

} /* extern "C" block ends here */

#ifdef __cplusplus
/* ========== TREE_BINFO patterns (C++ only) ========== */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int virtual_method() { return 42; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int virtual_method() override { return 84; }
    int derived_data;
};

class AnotherDerived : public BaseClass {
public:
    virtual int virtual_method() override { return 168; }
    int another_data;
};

__attribute__((noinline))
int binfo_pattern(void) {
    DerivedClass derived_obj;
    AnotherDerived another_obj;
    
    BaseClass* base_ptr1 = &derived_obj;
    BaseClass* base_ptr2 = &another_obj;
    
    /* Use virtual methods - involves BINFO lookups */
    int result = base_ptr1->virtual_method();
    result += base_ptr2->virtual_method();
    
    /* Dynamic cast - involves BINFO */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr1);
    if (derived_ptr) {
        result += derived_ptr->derived_data;
    }
    
    /* Multiple inheritance scenario */
    class MultiBase1 {
    public:
        virtual ~MultiBase1() {}
        int data1;
    };
    
    class MultiBase2 {
    public:
        virtual ~MultiBase2() {}
        int data2;
    };
    
    class MultiDerived : public MultiBase1, public MultiBase2 {
    public:
        int data3;
    };
    
    MultiDerived multi_obj;
    MultiBase1* mb1_ptr = &multi_obj;
    MultiBase2* mb2_ptr = &multi_obj;
    
    /* Casting through inheritance hierarchy */
    MultiDerived* md_ptr = dynamic_cast<MultiDerived*>(mb1_ptr);
    if (md_ptr) {
        result += 1000;
    }
    
    return result;
}
#endif

/* ========== Main function ========== */
int main(void) {
    volatile int final_result = 0;
    
    /* Initialize global variables */
    global_var_1 = 1;
    global_var_2 = 2.0f;
    global_var_3 = 3.0;
    global_var_4 = 'X';
    
    /* Call all pattern functions */
    final_result += identifier_pattern();
    final_result += vector_pattern();
    final_result += ssa_pattern(100);
    final_result += block_pattern();
    final_result += constructor_pattern();
    final_result += omp_pattern(100);
    
#ifdef __cplusplus
    final_result += binfo_pattern();
#endif
    
    /* Use result to prevent optimization */
#ifdef __cplusplus
    std::cout << "Final result: " << final_result << std::endl;
#else
    printf("Final result: %d\n", final_result);
#endif
    
    return final_result != 0 ? 0 : 1;
}
