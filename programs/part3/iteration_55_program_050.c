/* test_tree_nodes.cc - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global variables and operations */
int global_var1 = 10;
int global_var2 = 20;
float global_var3 = 3.14;
double global_var4 = 2.71828;
char global_var5 = 'A';

extern int external_func(int);  /* Forward declaration forces identifier lookup */

__attribute__((noinline))
int identifier_pattern() {
    /* Multiple local identifiers */
    int local1 = global_var1;
    int local2 = global_var2;
    float local3 = global_var3;
    double local4 = global_var4;
    char local5 = global_var5;
    
    /* Operations that require identifier lookup */
    int* ptr1 = &global_var1;
    int* ptr2 = &local1;
    size_t sz1 = sizeof(global_var2);
    size_t sz2 = sizeof(local2);
    
    /* Use in expressions with external function */
    int result = external_func(global_var1 + local1);
    
    /* Complex expression with multiple identifiers */
    return (global_var1 * local1) + (global_var2 / local2) + 
           (int)(global_var3 * local3) + (int)(global_var4 * local4) + 
           (int)global_var5 + (int)local5 + result;
}

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

__attribute__((noinline))
int vector_pattern() {
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    v4si vec5 = vec1 - vec2;
    
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fvec3 = fvec1 * fvec2;
    
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {0.25, 0.75};
    v2df dvec3 = dvec1 / dvec2;
    
    /* Use vectors in function-like context */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += vec3[i] + (int)fvec3[i];
    }
    sum += (int)dvec3[0] + (int)dvec3[1];
    
    return sum;
}
#else
__attribute__((noinline))
int vector_pattern() {
    /* Fallback for non-GCC compilers */
    int arr[4] = {1, 2, 3, 4};
    int sum = 0;
    for (int i = 0; i < 4; i++) sum += arr[i];
    return sum;
}
#endif

/* Pattern 3: SSA_NAME - Loops that force SSA form */
__attribute__((noinline))
int ssa_pattern(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Multiple loops creating SSA variables */
    for (int i = 0; i < n; ++i) {
        x = x + i;      /* Creates SSA_NAME for x */
        y = y * (i + 1); /* Creates SSA_NAME for y */
    }
    
    for (int j = 0; j < n * 2; ++j) {
        z = z - j;      /* Creates SSA_NAME for z */
        x = x + z;      /* Creates phi nodes */
    }
    
    /* Nested loop with complex flow */
    int w = 0;
    for (int k = 0; k < n; k++) {
        for (int l = 0; l < k; l++) {
            w = w + (k * l); /* More SSA complexity */
        }
    }
    
    return x + y + z + w;
}

/* Pattern 4: BLOCK - Nested blocks and statement expressions */
__attribute__((noinline))
int block_pattern(int val) {
    int result = 0;
    
    /* Level 1 block */
    {
        int a = val;
        
        /* Level 2 block */
        {
            int b = a * 2;
            
            /* Level 3 block with statement expression (GCC extension) */
            result = ({
                int c = b + 10;
                int d = c * 3;
                d;  /* Returns d */
            });
            
            /* Another statement expression */
            int e = ({
                int temp = result / 2;
                temp + 5;
            });
            
            result += e;
        }
        
        /* Label and goto creating block context */
        if (result > 100) {
            goto skip_block;
        }
        
        /* Another nested block */
        {
            int f = 42;
            result += f;
        }
    }
    
skip_block:
    /* Block with switch statement */
    switch (val) {
        case 1: {
            int g = 10;
            result += g;
            break;
        }
        case 2: {
            int h = 20;
            result += h;
            break;
        }
        default: {
            int i = 30;
            result += i;
            break;
        }
    }
    
    return result;
}

/* Pattern 5: CONSTRUCTOR - Structure and array initializers */
__attribute__((noinline))
int constructor_pattern() {
    /* Structure with designated initializer */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Point p1 = { .x = 1, .y = 2, .z = 3 };
    struct Point p2 = { .x = 4, .y = 5, .z = 6 };
    
    /* Array initializer */
    int arr1[5] = {10, 20, 30, 40, 50};
    int arr2[] = {1, 2, 3, 4, 5, 6};
    
    /* Compound literals */
    int sum = ((int[3]){p1.x, p1.y, p1.z})[0] +
              ((int[3]){p2.x, p2.y, p2.z})[1];
    
    /* Nested structure initializer */
    struct Rectangle {
        struct Point top_left;
        struct Point bottom_right;
    };
    
    struct Rectangle rect = {
        .top_left = {.x = 0, .y = 10},
        .bottom_right = {.x = 20, .y = 0}
    };
    
    sum += rect.top_left.x + rect.bottom_right.y;
    
    /* Initialize with compound literal in expression */
    sum += ((struct Point){.x = 100, .y = 200, .z = 300}).x;
    
    return sum;
}

/* Pattern 6: OpenMP clauses - Multiple pragmas with various clauses */
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel for with multiple clauses */
    #pragma omp parallel for private(size) shared(arr) reduction(+:sum) schedule(static, 10)
    for (int i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    int max_val = 0;
    int min_val = 1000;
    
    /* OpenMP parallel with sections */
    #pragma omp parallel sections private(size) shared(arr, max_val, min_val)
    {
        #pragma omp section
        {
            #pragma omp parallel for reduction(max:max_val)
            for (int i = 0; i < 100; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
        }
        
        #pragma omp section
        {
            #pragma omp parallel for reduction(min:min_val)
            for (int i = 0; i < 100; i++) {
                if (arr[i] < min_val) min_val = arr[i];
            }
        }
    }
    
    /* OpenMP task with if clause */
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < 10; i++) {
                #pragma omp task firstprivate(i) if(i > 5)
                {
                    arr[i] *= 2;
                }
            }
        }
    }
    
    return sum + max_val + min_val;
}

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
    
    int get_double() const { return derived_value * 2; }
    
private:
    int derived_value = 50;
};

class SecondDerived : public DerivedClass {
public:
    virtual int get_value() const override {
        return DerivedClass::get_value() + extra_value;
    }
    
private:
    int extra_value = 25;
};

__attribute__((noinline))
int binfo_pattern() {
    DerivedClass derived;
    SecondDerived second_derived;
    
    BaseClass* base_ptr1 = &derived;
    BaseClass* base_ptr2 = &second_derived;
    
    /* Virtual calls through base pointers */
    base_ptr1->set_value(42);
    base_ptr2->set_value(84);
    
    int result = base_ptr1->get_value() + base_ptr2->get_value();
    
    /* Dynamic casting (requires RTTI) */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr1);
    if (derived_ptr) {
        result += derived_ptr->get_double();
    }
    
    /* Array of base pointers */
    BaseClass* poly_array[3];
    poly_array[0] = &derived;
    poly_array[1] = &second_derived;
    poly_array[2] = new DerivedClass();
    
    for (int i = 0; i < 3; i++) {
        result += poly_array[i]->get_value();
    }
    
    delete poly_array[2];
    
    return result;
}
#endif

/* External function declaration for identifier pattern */
int external_func(int x) {
    return x * 2;
}

int main() {
    volatile int total = 0;  /* volatile to prevent optimization */
    
    /* Call all pattern functions */
    total += identifier_pattern();
    total += vector_pattern();
    total += ssa_pattern(50);
    total += block_pattern(25);
    total += constructor_pattern();
    total += omp_pattern(100);
    
    #ifdef __cplusplus
    total += binfo_pattern();
    #endif
    
    /* Use result to prevent dead code elimination */
    #ifdef __cplusplus
    std::cout << "Total checksum: " << total << std::endl;
    #else
    printf("Total checksum: %d\n", total);
    #endif
    
    return total > 0 ? 0 : 1;
}
