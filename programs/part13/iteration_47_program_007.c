/* Test program to exercise specific tree node types in GCC's tree.cc */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void opaque_external_function(int *);

/* Global identifiers for IDENTIFIER_NODE coverage */
static int static_global_counter = 0;
extern int external_global_data;
volatile int volatile_global = 1;

/* Complex struct for CONSTRUCTOR nodes */
struct ComplexData {
    int values[4];
    struct {
        double x, y;
    } point;
    char *name;
};

/* Another struct with designated initializers */
struct DesignatedInit {
    int a;
    int b;
    int c;
    int d;
};

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexData recursive_struct_builder(int depth, int base) {
    struct ComplexData result;
    
    /* Array initializer with partial elements */
    int local_array[4] = {base, base * 2, [3] = base * 3};
    
    for (int i = 0; i < 4; i++) {
        result.values[i] = local_array[i] + depth;
    }
    
    result.point.x = (double)depth;
    result.point.y = (double)(depth * 2);
    
    static char buffer[32];
    result.name = buffer;
    
    if (depth > 0) {
        struct ComplexData inner = recursive_struct_builder(depth - 1, base + 1);
        /* Combine results */
        for (int i = 0; i < 4; i++) {
            result.values[i] += inner.values[i];
        }
    }
    
    return result;
}

#ifdef __cplusplus
/* C++ specific code for TREE_BINFO nodes */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int virtual_method(int x) {
        return x * 2;
    }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int virtual_method(int x) override {
        return x * 3 + base_data;
    }
    int derived_data;
};

class SecondDerived : public DerivedClass {
public:
    virtual int virtual_method(int x) override {
        return x * 4 + base_data + derived_data;
    }
};

template<typename T>
class TemplateClass {
public:
    T data;
    TemplateClass(T val) : data(val) {}
    T process() { return data * 2; }
};

/* Function using templates (TREE_VEC) */
void template_usage() {
    TemplateClass<int> int_template(42);
    TemplateClass<double> double_template(3.14);
    volatile int result1 = int_template.process();
    volatile double result2 = double_template.process();
    
    /* Use different identifiers */
    BaseClass* poly1 = new DerivedClass();
    BaseClass* poly2 = new SecondDerived();
    poly1->base_data = 10;
    ((DerivedClass*)poly1)->derived_data = 20;
    
    /* Dynamic cast for BINFO usage */
    DerivedClass* derived = dynamic_cast<DerivedClass*>(poly2);
    if (derived) {
        derived->derived_data = 30;
    }
    
    int vcall1 = poly1->virtual_method(5);
    int vcall2 = poly2->virtual_method(5);
    
    delete poly1;
    delete poly2;
}
#endif

/* Function with complex control flow for SSA_NAME and BLOCK nodes */
int complex_control_flow(int n, int* output) {
    int result = 0;
    
    /* Outer block with local variable */
    {
        int block_local = n * 2;
        
        /* Nested block with goto */
        {
            int hidden = block_local + 10;
            if (n % 3 == 0) {
                goto skip_part;
            }
            
            /* This part might be skipped */
            hidden *= 2;
            
        skip_part:
            /* Use the variable to keep it alive */
            result += hidden;
        }
    }
    
    /* Loop with conditional assignment for SSA_NAME */
    for (int i = 0; i < n; i++) {
        int temp;
        if (i % 2 == 0) {
            temp = i * 3;
        } else {
            temp = i * 5;
        }
        
        /* Multiple assignments to same variable */
        if (i % 3 == 0) {
            temp += volatile_global;
        }
        
        /* Use temp to prevent elimination */
        output[i] = temp;
        result += temp;
    }
    
    /* Another block with switch */
    {
        volatile int switch_var = n % 5;
        int case_result;
        
        switch (switch_var) {
            case 0:
                case_result = 100;
                break;
            case 1:
                case_result = 200;
                break;
            case 2:
                case_result = 300;
                break;
            default:
                case_result = 400;
                break;
        }
        
        result += case_result;
    }
    
    return result;
}

/* OpenMP function with multiple clauses */
void openmp_test(int size, double* data) {
    int i, j;
    double sum = 0.0;
    volatile double checksum = 0.0;
    
    /* Multi-dimensional array for collapse clause */
    #pragma omp parallel for private(i, j) firstprivate(size) \
            shared(data) reduction(+:sum) collapse(2) \
            schedule(dynamic, 4)
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            int index = i * size + j;
            double val = (i + j) * 0.5;
            data[index] = val;
            sum += val;
        }
    }
    
    checksum = sum;
    
    /* Another OpenMP region with different clauses */
    #pragma omp parallel
    {
        #pragma omp single
        {
            volatile int thread_count = 0;
            #pragma omp atomic
            thread_count++;
        }
        
        #pragma omp for nowait
        for (i = 0; i < size * 2; i++) {
            data[i % size] += 0.1;
        }
    }
}

int main(int argc, char** argv) {
    /* Use argc to prevent compile-time optimization */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    /* CONSTRUCTOR: Struct with designated initializer */
    struct DesignatedInit di = {
        .a = 1,
        .c = 3,
        .d = 4,
        .b = 2  /* Out of order */
    };
    
    /* CONSTRUCTOR: Array with designators */
    int sparse_array[10] = {[2] = 20, [5] = 50, [8] = 80, 0};
    
    /* Call recursive function */
    struct ComplexData cd = recursive_struct_builder(3, 1);
    
    /* BLOCK: Local block with goto */
    int block_result = 0;
    {
        int x = 10;
        int y = 20;
        
        if (argc > 2) {
            goto middle;
        }
        
        x = 30;
        
    middle:
        y = 40;
        block_result = x + y;
    }
    
    /* SSA_NAME: Complex control flow */
    int* dynamic_array = (int*)malloc(iterations * sizeof(int));
    int control_result = complex_control_flow(iterations, dynamic_array);
    
    /* OpenMP with multiple clauses */
    double* omp_data = (double*)malloc(iterations * iterations * sizeof(double));
    openmp_test(iterations, omp_data);
    
    #ifdef __cplusplus
    /* C++ specific: Templates and inheritance */
    template_usage();
    
    /* More template usage for TREE_VEC */
    TemplateClass<long> long_template(1000L);
    volatile long lt_result = long_template.process();
    #endif
    
    /* Use various identifiers */
    opaque_external_function(&static_global_counter);
    opaque_external_function(dynamic_array);
    
    /* Calculate final checksum */
    long final_checksum = 0;
    final_checksum += di.a + di.b + di.c + di.d;
    final_checksum += sparse_array[2] + sparse_array[5] + sparse_array[8];
    final_checksum += cd.values[0] + cd.values[1] + cd.values[2] + cd.values[3];
    final_checksum += block_result;
    final_checksum += control_result;
    
    for (int i = 0; i < iterations && i < 10; i++) {
        final_checksum += dynamic_array[i];
    }
    
    for (int i = 0; i < iterations && i < 5; i++) {
        for (int j = 0; j < iterations && j < 5; j++) {
            final_checksum += (long)omp_data[i * iterations + j];
        }
    }
    
    printf("Final checksum: %ld\n", final_checksum);
    
    free(dynamic_array);
    free(omp_data);
    
    return (final_checksum > 0) ? 0 : 1;
}

/* Dummy external function definition to satisfy linker */
void opaque_external_function(int* ptr) {
    if (ptr) {
        *ptr += volatile_global;
    }
}
