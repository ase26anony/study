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
float global_var_2;
double global_var_3;
char global_var_4;

/* Function declarations to force identifier lookups */
extern int extern_func_1(int);
extern void extern_func_2(float);
extern double extern_func_3(double, int);

/* Use identifiers in various contexts */
void identifier_pattern(void) __attribute__((noinline));
void identifier_pattern(void) {
    /* Local variables */
    int local_var_1;
    float local_var_2;
    
    /* sizeof expressions with identifiers */
    volatile size_t s1 = sizeof(global_var_1);
    volatile size_t s2 = sizeof(local_var_1);
    
    /* Address-of operations */
    int *p1 = &global_var_1;
    float *p2 = &global_var_2;
    int *p3 = &local_var_1;
    
    /* Use in expressions */
    global_var_1 = 42;
    local_var_1 = global_var_1 * 2;
    global_var_2 = 3.14f * local_var_1;
    
    /* Prevent dead code elimination */
    (void)s1; (void)s2; (void)p1; (void)p2; (void)p3;
}

/* ========== TREE_VEC patterns ========== */
#ifdef __GNUC__
/* Vector type declaration */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

void vector_pattern(void) __attribute__((noinline));
void vector_pattern(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c;
    
    /* Vector arithmetic */
    c = a + b;
    c = a * b;
    c = a - b;
    
    /* Vector comparisons */
    v4si mask = a > b;
    
    /* Vector shuffle/permute */
    v4si d = __builtin_shuffle(a, b, (v4si){0, 1, 2, 3});
    
    /* Prevent optimization */
    volatile v4si *volatile ptr = &c;
    (void)mask; (void)d; (void)ptr;
}
#endif

/* ========== SSA_NAME patterns ========== */
void ssa_pattern(int n) __attribute__((noinline));
void ssa_pattern(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Multiple loops to create SSA variables */
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
    }
    
    for (int j = 0; j < n * 2; ++j) {
        z = z - j;
        x = x + z;
    }
    
    /* Conditional updates */
    for (int k = 0; k < n; ++k) {
        if (k % 2 == 0) {
            x = x * 2;
        } else {
            y = y / 2;
        }
    }
    
    /* Prevent dead code */
    volatile int result = x + y + z;
    (void)result;
}

/* ========== BLOCK patterns ========== */
void block_pattern(void) __attribute__((noinline));
void block_pattern(void) {
    /* Outer block with variables */
    int outer_var = 10;
    
    {
        /* Nested block 1 */
        int inner_var_1 = 20;
        
        {
            /* Nested block 2 */
            int inner_var_2 = 30;
            outer_var += inner_var_1 + inner_var_2;
        }
        
        /* Another nested block */
        {
            float float_var = 3.14f;
            outer_var += (int)float_var;
        }
    }
    
    /* GCC statement expression (creates a block) */
    int stmt_expr = ({
        int temp = 5;
        temp * temp;
    });
    
    /* Label address and goto (involves blocks) */
    void *label_ptr = &&my_label;
    
    if (outer_var > 50) {
        goto *label_ptr;
    }
    
    outer_var += 10;
    
my_label:
    /* Use computed goto target */
    volatile int final = outer_var + stmt_expr;
    (void)final;
    (void)label_ptr;
}

/* ========== CONSTRUCTOR patterns ========== */
void constructor_pattern(void) __attribute__((noinline));
void constructor_pattern(void) {
    /* Structure with designated initializer */
    struct S {
        int a;
        float b;
        double c;
        char d;
    };
    
    struct S s1 = { .a = 1, .b = 2.0f, .c = 3.0, .d = 'X' };
    struct S s2 = { .b = 4.5f, .a = 2, .d = 'Y', .c = 6.7 };
    
    /* Array initializer */
    int arr1[5] = {1, 2, 3, 4, 5};
    int arr2[3][2] = {{1, 2}, {3, 4}, {5, 6}};
    
    /* Compound literals */
    int *ptr1 = (int[]){10, 20, 30, 40};
    struct S *ptr2 = &(struct S){ .a = 100, .b = 200.0f, .c = 300.0, .d = 'Z' };
    
    /* Nested initializers */
    struct T {
        struct S inner;
        int extra;
    };
    
    struct T t1 = { .inner = { .a = 5, .b = 6.0f }, .extra = 7 };
    
    /* Prevent optimization */
    volatile struct S *vs = &s1;
    volatile int *va = arr1;
    (void)ptr1; (void)ptr2; (void)t1; (void)vs; (void)va;
}

/* ========== OMP_CLAUSE patterns ========== */
#ifdef _OPENMP
void omp_pattern(int n) __attribute__((noinline));
void omp_pattern(int n) {
    int i;
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel for with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static, 10)
    for (i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* OpenMP parallel region with clauses */
    #pragma omp parallel num_threads(4) default(none) shared(sum, arr)
    {
        #pragma omp for nowait
        for (i = 0; i < 50; i++) {
            arr[i] *= 2;
        }
    }
    
    /* OpenMP sections */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < 25; i++) arr[i] += 1;
        }
        
        #pragma omp section
        {
            for (i = 25; i < 50; i++) arr[i] -= 1;
        }
    }
    
    /* Prevent dead code */
    volatile int final_sum = sum;
    (void)final_sum;
}
#endif

/* ========== Main driver ========== */
int main(void) {
    int checksum = 0;
    
    /* Call all pattern functions */
    identifier_pattern();
    checksum += 1;
    
#ifdef __GNUC__
    vector_pattern();
    checksum += 2;
#endif
    
    ssa_pattern(100);
    checksum += 4;
    
    block_pattern();
    checksum += 8;
    
    constructor_pattern();
    checksum += 16;
    
#ifdef _OPENMP
    omp_pattern(100);
    checksum += 32;
#endif
    
    /* Final output to prevent optimization */
    volatile int result = checksum;
    
#ifdef __cplusplus
    std::cout << "Result: " << result << std::endl;
#else
    printf("Result: %d\n", result);
#endif
    
    return 0;
}

#ifdef __cplusplus
} /* extern "C" */

/* ========== C++ TREE_BINFO patterns ========== */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int get_value() const { return base_value; }
    virtual void set_value(int v) { base_value = v; }
    
private:
    int base_value = 42;
};

class DerivedClass : public BaseClass {
public:
    virtual int get_value() const override { return derived_value; }
    virtual void set_value(int v) override { derived_value = v * 2; }
    
    int get_double() const { return derived_value * 2; }
    
private:
    int derived_value = 84;
};

class SecondDerived : public DerivedClass {
public:
    virtual int get_value() const override { return second_value; }
    
private:
    int second_value = 168;
};

void cpp_binfo_pattern(void) __attribute__((noinline));
void cpp_binfo_pattern(void) {
    DerivedClass derived_obj;
    SecondDerived second_obj;
    
    BaseClass* base_ptr1 = &derived_obj;
    BaseClass* base_ptr2 = &second_obj;
    
    /* Virtual function calls (involve BINFO lookups) */
    base_ptr1->set_value(100);
    int val1 = base_ptr1->get_value();
    
    base_ptr2->set_value(200);
    int val2 = base_ptr2->get_value();
    
    /* Downcast using dynamic_cast (requires BINFO) */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr1);
    if (derived_ptr) {
        val1 += derived_ptr->get_double();
    }
    
    /* Array of base pointers */
    BaseClass* poly_array[3];
    poly_array[0] = &derived_obj;
    poly_array[1] = &second_obj;
    poly_array[2] = new DerivedClass();
    
    for (int i = 0; i < 3; i++) {
        poly_array[i]->set_value(i * 10);
    }
    
    delete poly_array[2];
    
    /* Prevent optimization */
    volatile int result = val1 + val2;
    (void)result;
}

/* C++ main */
int cpp_main(void) {
    main();  /* Call C main first */
    cpp_binfo_pattern();
    return 0;
}
#endif
