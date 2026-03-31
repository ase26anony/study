/* Test program to exercise specific tree node types in GCC's tree.cc */
#ifdef __cplusplus
#include <vector>
#include <iostream>
extern "C" {
#endif

/* External function declarations to prevent optimization */
extern void use_int(int x);
extern void use_ptr(void *p);
extern int get_random(void);

/* Global identifiers (IDENTIFIER_NODE) */
static int static_counter = 0;
extern int external_var;
int global_var = 42;

/* Complex struct for CONSTRUCTOR nodes */
struct ComplexData {
    int id;
    double values[4];
    struct {
        short x, y;
    } coord;
};

/* Another struct for nested constructors */
struct Nested {
    struct ComplexData data;
    char tag;
    int flags[3];
};

#ifdef __cplusplus
/* C++ classes for TREE_BINFO nodes */
class Base {
public:
    virtual int method() { return 1; }
    virtual ~Base() {}
    int base_data;
};

class Derived : public Base {
public:
    virtual int method() override { return 2; }
    int derived_data;
};

class DeepDerived : public Derived {
public:
    virtual int method() override { return 3; }
    int deep_data;
};
#endif

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexData build_data(int depth, int seed) {
    struct ComplexData result;
    result.id = seed;
    
    /* Use volatile to prevent optimization */
    volatile int i;
    for (i = 0; i < 4; i++) {
        result.values[i] = (seed + i) * 1.5;
    }
    
    result.coord.x = seed % 100;
    result.coord.y = (seed * 3) % 100;
    
    if (depth > 0) {
        /* Recursive call */
        struct ComplexData temp = build_data(depth - 1, seed + 1);
        result.values[0] += temp.values[0];
    }
    
    return result;  /* CONSTRUCTOR node for return value */
}

/* Function with SSA_NAME generation */
int ssa_test(int iterations, int threshold) {
    int x = 0;  /* Will become SSA_NAME */
    int y = 0;  /* Will become SSA_NAME */
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Conditional assignment creating phi nodes */
        if (i % 3 == 0) {
            x = i * 2;
        } else if (i % 3 == 1) {
            x = i + threshold;
        } else {
            x = threshold - i;
        }
        
        /* Another conditional for y */
        if (x > threshold) {
            y = x / 2;
        } else {
            y = x * 3;
        }
        
        /* Use both to prevent elimination */
        result += x + y;
        
        /* Nested block with local variable (BLOCK node) */
        {
            int hidden = i * 7;  /* BLOCK local */
            if (hidden % 5 == 0) {
                goto skip_hidden;  /* goto for BLOCK stress */
            }
            result += hidden % 3;
skip_hidden:
            /* Empty target for goto */
            ;
        }
    }
    
    /* Complex expression with multiple SSA_NAME uses */
    return result > 0 ? result : -result;
}

int main(int argc, char *argv[]) {
    /* Use argc to prevent compile-time optimization */
    int iterations = (argc > 1) ? 10 : 20;
    int threshold = (argc > 2) ? 100 : 200;
    
    /* Array initializer with designators (TREE_VEC in C, template in C++) */
    int arr[10] = {[0] = 1, [2] = 3, [5] = get_random(), [9] = 9};
    
    /* Struct initialization (CONSTRUCTOR nodes) */
    struct Nested nested = {
        .data = {
            .id = 1001,
            .values = {1.1, 2.2, 3.3, 4.4},
            .coord = {.x = 10, .y = 20}
        },
        .tag = 'A',
        .flags = {0x1, 0x2, 0x4}
    };
    
    /* Another constructor with partial initialization */
    struct ComplexData data2 = {.id = 2002, .coord = {.y = 50}};
    
    /* Call recursive constructor-returning function */
    struct ComplexData built = build_data(3, 42);
    use_int(built.id);
    
    /* SSA test */
    int ssa_result = ssa_test(iterations, threshold);
    use_int(ssa_result);
    
    /* OpenMP region with multiple clauses (OMP_CLAUSE nodes) */
    int sum = 0;
    int matrix[10][20];
    
    #pragma omp parallel for private(iterations) firstprivate(threshold) \
            shared(matrix) reduction(+:sum) collapse(2) schedule(dynamic)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            matrix[i][j] = i * j + threshold;
            sum += matrix[i][j];
            
            /* Nested OpenMP directive */
            #pragma omp atomic
            static_counter++;
        }
    }
    
    /* Another OpenMP region with different clauses */
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            use_int(sum);
        }
        
        #pragma omp for ordered
        for (int i = 0; i < 5; i++) {
            #pragma omp ordered
            {
                use_int(i);
            }
        }
    }
    
    /* Complex block structure with goto (BLOCK nodes) */
    {
        int block_var1 = 100;
        goto middle;
        
        {
            int block_var2 = 200;  /* This won't be initialized due to goto */
            use_int(block_var2);
        }
        
middle:
        {
            int block_var3 = 300;
            use_int(block_var3 + block_var1);
        }
        
        /* Switch with computed goto-like behavior */
        switch (ssa_result % 4) {
            case 0: goto end_block;
            case 1: use_int(1); break;
            case 2: use_int(2); break;
            default: use_int(3);
        }
        
        int block_var4 = 400;
        use_int(block_var4);
        
end_block:
        use_int(999);
    }
    
#ifdef __cplusplus
    /* C++ specific code for TREE_BINFO nodes */
    std::vector<Base*> objects;  /* Template instantiation (TREE_VEC) */
    
    Derived d1;
    DeepDerived d2;
    Base b;
    
    objects.push_back(&d1);
    objects.push_back(&d2);
    objects.push_back(&b);
    
    /* Virtual calls through base pointer */
    for (Base* obj : objects) {
        use_int(obj->method());
        
        /* dynamic_cast for BINFO usage */
        if (Derived* derived = dynamic_cast<Derived*>(obj)) {
            use_int(derived->derived_data);
        }
    }
    
    /* Template with multiple parameters */
    std::vector<std::vector<int>> matrix_vec(10, std::vector<int>(20));
    for (auto& row : matrix_vec) {
        for (int val : row) {
            sum += val;
        }
    }
#endif
    
    /* Use all computed results to prevent optimization */
    int final_result = sum + ssa_result + nested.data.id + built.id;
    final_result += arr[0] + arr[5];
    
    /* Call external function with various identifiers */
    use_int(final_result);
    use_ptr(&global_var);
    use_ptr(&static_counter);
    
    return final_result % 256;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif
