/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global variables and operations */
int global_var_1 = 10;
int global_var_2 = 20;
float global_var_3 = 30.5;
double global_var_4 = 40.7;
char global_var_5 = 'A';

/* Function using identifiers in various ways */
__attribute__((noinline))
int identifier_pattern(void) {
    /* Local identifiers */
    int local_var_1 = 5;
    int local_var_2 = 15;
    static int static_var = 100;
    
    /* Operations that create identifier nodes */
    int *ptr1 = &global_var_1;
    int *ptr2 = &local_var_1;
    size_t sz1 = sizeof(global_var_2);
    size_t sz2 = sizeof(local_var_2);
    
    /* Complex expressions with identifiers */
    int result = global_var_1 + local_var_1 * global_var_2 - local_var_2;
    result += (int)global_var_3 + (int)global_var_4;
    result += global_var_5;
    
    /* Function calls with identifiers */
    extern int dummy_extern_func(int);
    result += dummy_extern_func(static_var);
    
    return result;
}

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

__attribute__((noinline))
int vector_pattern(void) {
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c;
    
    /* Vector operations */
    vec_c = vec_a + vec_b;
    vec_c = vec_a * vec_b;
    vec_c = vec_a - vec_b;
    
    /* Mixed vector types */
    v4sf vec_f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_f2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec_f3 = vec_f1 + vec_f2;
    
    /* Vector in function argument */
    extern v4si dummy_vector_func(v4si);
    vec_c = dummy_vector_func(vec_c);
    
    /* Extract elements */
    int sum = vec_c[0] + vec_c[1] + vec_c[2] + vec_c[3];
    sum += (int)vec_f3[0];
    
    return sum;
}
#else
__attribute__((noinline))
int vector_pattern(void) {
    /* Fallback for non-GCC compilers */
    int arr[4] = {1, 2, 3, 4};
    return arr[0] + arr[1] + arr[2] + arr[3];
}
#endif

/* Pattern 3: SSA_NAME - Loops and variable modifications */
__attribute__((noinline))
int ssa_pattern(int n) {
    int x = 0;
    int y = 10;
    int z = 20;
    
    /* Multiple loops creating SSA names */
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
    }
    
    for (int j = n; j > 0; --j) {
        z = z - j;
        x = x + z;
    }
    
    /* Conditional updates */
    if (x > 0) {
        y = y + x;
    } else {
        y = y - x;
    }
    
    /* Nested loops */
    for (int k = 0; k < 5; ++k) {
        for (int l = 0; l < 3; ++l) {
            z = z + k * l;
        }
    }
    
    return x + y + z;
}

/* Pattern 4: BLOCK - Nested blocks and statement expressions */
__attribute__((noinline))
int block_pattern(void) {
    int result = 0;
    
    /* Level 1 block */
    {
        int a = 10;
        result += a;
        
        /* Level 2 block */
        {
            int b = 20;
            result += b;
            
            /* Level 3 block */
            {
                int c = 30;
                result += c;
            }
        }
    }
    
    /* GCC statement expression (creates a block) */
    result += ({
        int temp = 100;
        temp * 2;
    });
    
    /* More complex nested blocks with variables */
    {
        int x = 5;
        {
            int y = x * 2;
            {
                int z = y + 10;
                result += z;
            }
        }
    }
    
    /* Labels and gotos (implicitly use blocks) */
    void *label_ptr = &&my_label;
    goto *label_ptr;
    
my_label:
    result += 50;
    
    return result;
}

/* Pattern 5: CONSTRUCTOR - Structure and array initializers */
__attribute__((noinline))
int constructor_pattern(void) {
    /* Structure with designated initializer */
    struct ComplexStruct {
        int int_field;
        float float_field;
        double double_field;
        char char_field;
        int array_field[3];
    };
    
    struct ComplexStruct s1 = {
        .int_field = 42,
        .float_field = 3.14f,
        .double_field = 2.71828,
        .char_field = 'X',
        .array_field = {1, 2, 3}
    };
    
    /* Array initializer */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Compound literal */
    int *ptr = (int[4]){100, 200, 300, 400};
    
    /* Nested structure initializer */
    struct Inner {
        int a;
        int b;
    };
    
    struct Outer {
        struct Inner inner;
        int extra;
    };
    
    struct Outer o1 = {
        .inner = {.a = 1, .b = 2},
        .extra = 3
    };
    
    /* Multiple constructors in expressions */
    int sum = s1.int_field + arr[2] + ptr[1] + o1.inner.a;
    
    /* Union initializer */
    union MyUnion {
        int i;
        float f;
    } u1 = {.i = 123};
    
    sum += u1.i;
    
    return sum;
}

/* Pattern 6: OMP_CLAUSE - OpenMP directives */
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
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static, 10)
    for (int i = 0; i < size; i++) {
        sum += arr[i % 100];
    }
    
    /* Another OpenMP directive with different clauses */
    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val) collapse(2) if(size > 1000)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int val = i * 10 + j;
            if (val > max_val) {
                max_val = val;
            }
        }
    }
    
    /* OpenMP sections */
    int section_result = 0;
    #pragma omp parallel sections private(i) firstprivate(section_result)
    {
        #pragma omp section
        {
            for (int i = 0; i < 5; i++) {
                section_result += i;
            }
        }
        
        #pragma omp section
        {
            for (int i = 5; i < 10; i++) {
                section_result += i * 2;
            }
        }
    }
    
    return sum + max_val + section_result;
}
#else
__attribute__((noinline))
int omp_pattern(int size) {
    /* Fallback without OpenMP */
    int sum = 0;
    for (int i = 0; i < size && i < 100; i++) {
        sum += i;
    }
    return sum;
}
#endif

#ifdef __cplusplus
} /* extern "C" */

/* Pattern 7: TREE_BINFO - C++ class inheritance */
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
    virtual int get_value() const override { 
        return BaseClass::get_value() + derived_value; 
    }
    virtual void set_value(int v) override { 
        BaseClass::set_value(v);
        derived_value = v * 2;
    }
    
private:
    int derived_value = 50;
};

class SecondDerived : public DerivedClass {
public:
    virtual int get_value() const override {
        return DerivedClass::get_value() + 25;
    }
};

__attribute__((noinline))
int binfo_pattern(void) {
    DerivedClass derived_obj;
    BaseClass* base_ptr = &derived_obj;
    
    /* Virtual function calls through base pointer */
    base_ptr->set_value(42);
    int val1 = base_ptr->get_value();
    
    /* Multiple inheritance levels */
    SecondDerived second_derived;
    BaseClass* base_ptr2 = &second_derived;
    int val2 = base_ptr2->get_value();
    
    /* Array of base pointers */
    BaseClass* poly_array[3];
    poly_array[0] = new BaseClass();
    poly_array[1] = new DerivedClass();
    poly_array[2] = new SecondDerived();
    
    int total = 0;
    for (int i = 0; i < 3; i++) {
        poly_array[i]->set_value(i * 10);
        total += poly_array[i]->get_value();
        delete poly_array[i];
    }
    
    return val1 + val2 + total;
}
#endif

/* Main function that calls all patterns */
int main(void) {
    volatile int total = 0; /* volatile to prevent optimization */
    
    /* Call all pattern functions */
    total += identifier_pattern();
    total += vector_pattern();
    total += ssa_pattern(100);
    total += block_pattern();
    total += constructor_pattern();
    total += omp_pattern(1000);
    
#ifdef __cplusplus
    total += binfo_pattern();
    std::cout << "Total result: " << total << std::endl;
#else
    printf("Total result: %d\n", total);
#endif
    
    return total != 0 ? 0 : 1;
}
