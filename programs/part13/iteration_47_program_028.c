/* Compile with: g++ -O2 -fopenmp -std=c++11 -fdump-tree-all tree_test.cc -o tree_test */
/* Or for C: gcc -O2 -fopenmp -fdump-tree-all tree_test.c -o tree_test */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization - generates IDENTIFIER_NODE */
extern void opaque_external_function(int, double, char*);

/* Global variables with various linkages - generate IDENTIFIER_NODES */
int global_var = 42;
static int static_var = 100;
extern int extern_var;

/* Complex struct for CONSTRUCTOR nodes */
struct ComplexData {
    int values[5];
    double factor;
    char tag;
};

/* Nested struct for more complex CONSTRUCTOR */
struct Nested {
    struct ComplexData data;
    int counter;
    float ratios[3];
};

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexData build_data(int depth, int base) {
    struct ComplexData result;
    
    /* Complex initializer - CONSTRUCTOR node */
    struct ComplexData temp = {
        .values = {base, base+1, base+2, base+3, base+4},
        .factor = depth * 1.5,
        .tag = 'A' + (depth % 26)
    };
    
    if (depth > 0) {
        /* Recursive call */
        struct ComplexData child = build_data(depth - 1, base * 2);
        
        /* Combine data - creates more tree nodes */
        for (int i = 0; i < 5; i++) {
            temp.values[i] += child.values[i];
        }
        temp.factor += child.factor;
    }
    
    return temp;
}

/* Function with complex control flow for SSA_NAME and BLOCK nodes */
int process_value(int x, int y) {
    int result;
    
    /* Start a new block - BLOCK node */
    {
        int local_block_var = x * y;
        
        /* Conditional with multiple assignments - creates SSA_NAME nodes */
        if (x > y) {
            result = local_block_var + x;
        } else if (x < y) {
            result = local_block_var - y;
        } else {
            result = local_block_var * 2;
        }
        
        /* Another block with goto - stresses BLOCK handling */
        {
            int hidden = 10;
            if (result > 100) {
                goto skip_part;
            }
            hidden = 20;
skip_part:
            result += hidden;
        }
    }
    
    /* Loop with phi nodes for SSA */
    int sum = 0;
    for (int i = 0; i < x; i++) {
        /* This creates phi nodes in SSA form */
        if (i % 2 == 0) {
            sum += i * y;
        } else {
            sum += i * x;
        }
    }
    
    return result + sum;
}

/* OpenMP function with multiple clauses - generates OMP_CLAUSE nodes */
void openmp_computation(int size, double* array) {
    double total = 0.0;
    int i, j;
    
    /* Complex OpenMP pragma with multiple clauses */
    #pragma omp parallel for private(i, j) firstprivate(size) shared(array) reduction(+:total) collapse(2) schedule(dynamic)
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            int index = i * size + j;
            array[index] = (i + j) * 0.5;
            total += array[index];
        }
    }
    
    /* Another OpenMP region with different clauses */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < size; i++) {
                array[i] *= 2.0;
            }
        }
        
        #pragma omp section
        {
            for (i = size/2; i < size; i++) {
                array[i] += total;
            }
        }
    }
}

/* C++ specific code for TREE_BINFO nodes */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int process(int x) = 0;
    virtual void identify() { printf("Base\n"); }
};

class DerivedClass : public BaseClass {
private:
    int value;
public:
    DerivedClass(int v) : value(v) {}
    virtual int process(int x) override {
        return x * value;
    }
    virtual void identify() override {
        printf("Derived: %d\n", value);
    }
};

class SecondDerived : public DerivedClass {
public:
    SecondDerived(int v) : DerivedClass(v) {}
    virtual int process(int x) override {
        return x + DerivedClass::process(x);
    }
};

void test_cpp_features() {
    DerivedClass* obj1 = new DerivedClass(10);
    SecondDerived* obj2 = new SecondDerived(20);
    BaseClass* base1 = obj1;
    BaseClass* base2 = obj2;
    
    /* Virtual calls generate BINFO lookups */
    base1->identify();
    base2->identify();
    
    /* dynamic_cast uses BINFO */
    DerivedClass* casted = dynamic_cast<DerivedClass*>(base2);
    if (casted) {
        casted->identify();
    }
    
    delete obj1;
    delete obj2;
}
#endif

/* Main function with diverse tree node generation */
int main(int argc, char** argv) {
    /* Use argc to prevent optimization */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    int seed = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Complex array initializer - TREE_VEC and CONSTRUCTOR nodes */
    int matrix[3][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    };
    
    /* Designated initializer - more complex CONSTRUCTOR */
    struct Nested nested = {
        .data = {
            .values = {100, 200, 300, 400, 500},
            .factor = 3.14159,
            .tag = 'Z'
        },
        .counter = 999,
        .ratios = {1.1, 2.2, 3.3}
    };
    
    /* Call recursive function - generates CONSTRUCTOR returns */
    struct ComplexData built = build_data(3, seed);
    
    /* Process with complex control flow - SSA_NAME and BLOCK nodes */
    int processed = 0;
    for (int i = 0; i < iterations; i++) {
        processed += process_value(i, seed + i);
        
        /* Nested block with local variable - BLOCK node */
        {
            int block_local = i * 10;
            if (block_local % 7 == 0) {
                goto special_case;
            }
            processed += block_local;
            continue;
            
special_case:
            processed -= block_local;
        }
    }
    
    /* OpenMP computation - OMP_CLAUSE nodes */
    double* big_array = (double*)malloc(100 * 100 * sizeof(double));
    openmp_computation(100, big_array);
    
    /* Call external function with various identifiers */
    opaque_external_function(processed, built.factor, "test_string");
    
    /* C++ specific code if compiled as C++ */
    #ifdef __cplusplus
    test_cpp_features();
    
    /* Template instantiation - TREE_VEC nodes */
    std::vector<std::pair<int, double>> template_vec;
    for (int i = 0; i < 10; i++) {
        template_vec.push_back(std::make_pair(i, i * 0.5));
    }
    #endif
    
    /* Final computation to ensure all code is live */
    double final_result = 0.0;
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            final_result += big_array[i * 100 + j];
        }
    }
    
    final_result += processed + built.values[0] + nested.counter;
    
    printf("Final result: %f\n", final_result);
    
    free(big_array);
    return (int)final_result % 256;
}
