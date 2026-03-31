/* test_tree_coverage.cc - Comprehensive test for GCC tree node coverage */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* ==================== IDENTIFIER_NODE generation ==================== */
/* Many distinct identifiers at different scopes */
int global_var_1;
static int static_var_1;
extern int extern_var_1;

typedef struct TypeName1 {
    int member1;
    float member2;
} TypeName1;

typedef union UnionType {
    int as_int;
    float as_float;
    char as_char;
} UnionType;

enum EnumType {
    ENUM_VALUE_1,
    ENUM_VALUE_2,
    ENUM_VALUE_3
};

/* Function with many parameters for TREE_VEC */
void __attribute__((format(printf, 1, 2)))
multi_param_func(const char* fmt, int a, double b, char c, 
                 long d, short e, float f, void* g) {
    printf(fmt, a, b, c, d, e, f, g);
}

/* Another function with complex attributes */
int __attribute__((noinline, cold))
attributed_func(int x, int y, int z) __attribute__((returns_twice)) {
    return x + y + z;
}

/* ==================== C++ Classes for TREE_BINFO ==================== */
#ifdef __cplusplus

class BaseClass {
public:
    BaseClass() : base_data(42) {}
    virtual ~BaseClass() {}
    virtual void virtual_method() { printf("BaseClass::virtual_method\n"); }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    DerivedClass() : derived_data(100) {}
    virtual void virtual_method() override { 
        printf("DerivedClass::virtual_method, base_data=%d\n", base_data); 
    }
    int derived_data;
};

class AnotherDerived : public BaseClass {
public:
    AnotherDerived() : another_data(200) {}
    virtual void virtual_method() override {
        printf("AnotherDerived::virtual_method\n");
    }
    int another_data;
};

template<typename T>
class TemplateClass {
public:
    TemplateClass(T val) : data(val) {}
    T get_data() const { return data; }
private:
    T data;
};

void test_cpp_classes() {
    BaseClass* base_ptr;
    DerivedClass derived;
    AnotherDerived another;
    TemplateClass<int> t_int(123);
    TemplateClass<double> t_double(456.789);
    
    base_ptr = &derived;
    base_ptr->virtual_method();
    
    base_ptr = &another;
    base_ptr->virtual_method();
    
    printf("Template int: %d\n", t_int.get_data());
    printf("Template double: %f\n", t_double.get_data());
}

#endif

/* ==================== SSA_NAME generation ==================== */
int generate_ssa_names(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Complex conditional assignments in loop */
    for (int i = 0; i < n; i++) {
        /* Multiple assignments to x along different paths */
        if (i % 3 == 0) {
            x = y + z;
        } else if (i % 3 == 1) {
            x = y - z;
        } else {
            x = y * z;
        }
        
        /* Nested conditionals for more SSA complexity */
        for (int j = 0; j < 5; j++) {
            int temp = (j > 2) ? x : y;
            z = (temp > 0) ? temp : 0;
        }
        
        y = x + i;
    }
    
    /* Switch with multiple cases for SSA */
    int result = 0;
    switch (x % 4) {
        case 0: result = x + y; break;
        case 1: result = x - y; break;
        case 2: result = x * y; break;
        case 3: result = x / (y ? y : 1); break;
    }
    
    return result;
}

/* ==================== BLOCK node generation ==================== */
int test_blocks(int val) {
    /* Outer block */
    int a = val;
    
    {
        /* Nested block 1 */
        int b = a * 2;
        {
            /* Nested block 2 */
            int c = b + 3;
            a += c;
        }
        
        /* Statement expression creates a block */
        int d = ({
            int temp = a;
            temp *= 2;
            temp + 5;
        });
        a = d;
    }
    
    /* Another block with different variables */
    {
        int e = 10;
        int f = 20;
        while (e < f) {
            /* Loop creates another block */
            int g = e * f;
            e += g % 3;
        }
        a += e;
    }
    
    return a;
}

/* ==================== CONSTRUCTOR node generation ==================== */
struct ComplexStruct {
    int ints[5];
    float floats[3];
    struct {
        char c;
        short s;
    } nested;
};

union ComplexUnion {
    struct {
        int a, b;
    } s;
    long long ll;
    double d;
};

/* Global constructors */
struct ComplexStruct global_struct = {
    .ints = {1, 2, 3, 4, 5},
    .floats = {1.1f, 2.2f, 3.3f},
    .nested = {.c = 'A', .s = 100}
};

int array_global[][3] = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9}
};

void test_constructors() {
    /* Local struct with designated initializer */
    struct ComplexStruct local_struct = {
        .ints = {10, 20, 30, 40, 50},
        .floats = {10.1f, 20.2f, 30.3f},
        .nested = {.c = 'B', .s = 200}
    };
    
    /* Array constructor */
    int local_array[4] = {100, 200, 300, 400};
    
    /* Compound literal in expression */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += ((int[]){1, 2, 3, 4})[i];
    }
    
    /* Nested compound literals */
    struct {
        int x;
        int y;
        int z;
    } point = (struct {int x; int y; int z;}){1, 2, 3};
    
    /* Union constructor */
    union ComplexUnion u = {.s = {.a = 42, .b = 24}};
    
    printf("Constructor test: sum=%d, point.x=%d, u.s.a=%d\n", 
           sum, point.x, u.s.a);
}

/* ==================== OMP_CLAUSE node generation ==================== */
void test_openmp(int size) {
    int* arr = (int*)malloc(size * sizeof(int));
    int sum = 0;
    int i;
    
    if (!arr) return;
    
    /* Initialize array */
    for (i = 0; i < size; i++) {
        arr[i] = i + 1;
    }
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) \
        schedule(static, 16) if(size > 1000)
    for (i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    printf("OpenMP sum: %d\n", sum);
    
    /* Task with depend clause */
    int x = 0, y = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: x)
        {
            x = 42;
        }
        
        #pragma omp task depend(in: x) depend(out: y)
        {
            y = x * 2;
        }
        
        #pragma omp task depend(in: y)
        {
            printf("Task result: y = %d\n", y);
        }
    }
    
    /* SIMD with linear clause */
    int results[100];
    #pragma omp simd linear(j:1) simdlen(8)
    for (int j = 0; j < 100; j++) {
        results[j] = j * j;
    }
    
    /* Parallel sections */
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            arr[0] *= 2;
        }
        
        #pragma omp section
        {
            arr[1] *= 3;
        }
        
        #pragma omp section
        {
            arr[2] *= 4;
        }
    }
    
    free(arr);
}

/* ==================== Main function ==================== */
int main() {
    int checksum = 0;
    
    printf("=== Starting tree node coverage test ===\n");
    
    /* Test IDENTIFIER_NODE and TREE_VEC */
    multi_param_func("Format test: %d %f %c %ld %hd %f %p\n",
                     42, 3.14, 'X', 123456L, (short)99, 2.718f, NULL);
    
    checksum += attributed_func(1, 2, 3);
    
    /* Test C++ classes for TREE_BINFO */
    #ifdef __cplusplus
    test_cpp_classes();
    #endif
    
    /* Test SSA_NAME generation */
    checksum += generate_ssa_names(50);
    
    /* Test BLOCK nodes */
    checksum += test_blocks(100);
    
    /* Test CONSTRUCTOR nodes */
    test_constructors();
    
    /* Test OMP_CLAUSE nodes */
    test_openmp(1000);
    
    printf("=== Final checksum: %d ===\n", checksum);
    
    return 0;
}
