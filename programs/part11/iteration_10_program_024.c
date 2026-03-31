/* test_tree.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(int);
extern void external_func2(void);
extern volatile int external_var;

#ifdef __cplusplus
extern "C" {
#endif

/* Force creation of various tree nodes */
static volatile int checksum = 0;

/* Helper to prevent optimization */
static void use(void *p) {
    checksum += (int)(long)p;
}

/* ========== IDENTIFIER_NODE tests ========== */
void test_identifiers(void) {
    /* Deeply nested scopes with same variable names */
    {
        int x = 1;
        checksum += x;
        
        {
            /* Different x in inner scope */
            float x = 2.0f;
            checksum += (int)x;
            
            {
                /* Another x */
                volatile int x = 3;
                checksum += x;
                
                {
                    /* Pointer to outer x */
                    extern int x;
                    volatile int y = x;
                    checksum += y;
                }
            }
        }
    }
    
    /* Multiple declarations in loops */
    for (int i = 0; i < 3; i++) {
        /* Same name in each iteration */
        volatile int counter = i;
        checksum += counter;
        
        {
            /* Different counter in nested block */
            float counter = i * 2.0f;
            checksum += (int)counter;
        }
    }
    
    /* Function parameter shadowing */
    {
        auto int shadow_test(int x) {
            {
                float x = x * 1.5f;  /* Different x */
                return (int)x;
            }
        }
        checksum += shadow_test(10);
    }
}

/* ========== TREE_VEC tests ========== */
void test_tree_vec(void) {
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    /* Use vector elements */
    for (int i = 0; i < 4; i++) {
        checksum += c[i];
        checksum += d[i];
    }
    
    /* Array compound literals */
    int *arr1 = (int[]){1, 2, 3, 4, 5};
    int *arr2 = (int[]){[0] = 10, [2] = 20, [4] = 30};
    
    for (int i = 0; i < 5; i++) {
        checksum += arr1[i];
        checksum += arr2[i];
    }
    
    /* Nested compound literals */
    struct Point { int x; int y; };
    struct Point *points = (struct Point[]){
        {1, 2}, {3, 4}, {5, 6}
    };
    
    for (int i = 0; i < 3; i++) {
        checksum += points[i].x + points[i].y;
    }
}

/* ========== SSA_NAME tests ========== */
int test_ssa_names(int n) {
    /* Complex control flow with many assignments */
    int x = 0, y = 1, z = 2;
    volatile int result = 0;
    
    for (int i = 0; i < n; i++) {
        if (i & 1) {
            x = y + z;
            y = x * 2;
        } else {
            z = x - y;
            x = z / 2;
        }
        
        switch (i % 4) {
            case 0: x += i; break;
            case 1: y -= i; break;
            case 2: z *= i; break;
            case 3: x = y = z = i; break;
        }
        
        result += x + y + z;
    }
    
    /* Nested loops with phi nodes */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if ((i + j) & 1) {
                sum += i * j;
            } else {
                sum -= i + j;
            }
        }
    }
    
    return result + sum;
}

/* ========== BLOCK tests ========== */
void test_blocks(void) {
    /* Deeply nested blocks with labels and gotos */
    {
        int a = 0;
    label1:
        a++;
        {
            int b = a * 2;
        label2:
            b += 5;
            {
                volatile int c = b + 10;
                checksum += c;
                if (a < 3) goto label1;
            }
            if (b < 20) goto label2;
        }
    }
    
    /* Switch statement with blocks */
    int val = 5;
    switch (val) {
        case 1: {
            int block_var1 = 10;
            checksum += block_var1;
            break;
        }
        case 5: {
            int block_var2 = 20;
            {
                int inner_var = block_var2 * 2;
                checksum += inner_var;
            }
            break;
        }
        default: {
            int block_var3 = 30;
            checksum += block_var3;
        }
    }
}

/* ========== CONSTRUCTOR tests ========== */
void test_constructors(void) {
    /* Designated initializers */
    struct Complex {
        int a;
        int b[4];
        struct {
            int x;
            int y;
        } point;
    };
    
    struct Complex c1 = {
        .a = 1,
        .b = {[0] = 10, [2] = 20, [3] = 30},
        .point = {.x = 100, .y = 200}
    };
    
    checksum += c1.a;
    for (int i = 0; i < 4; i++) {
        checksum += c1.b[i];
    }
    checksum += c1.point.x + c1.point.y;
    
    /* Partial array initialization */
    int arr[10] = {[0] = 1, [5] = 2, [9] = 3};
    for (int i = 0; i < 10; i++) {
        checksum += arr[i];
    }
    
    /* Nested designated initializers */
    struct Outer {
        struct Inner {
            int a;
            int b;
        } inner;
        int c;
    };
    
    struct Outer o = {
        .inner = {.a = 5, .b = 6},
        .c = 7
    };
    checksum += o.inner.a + o.inner.b + o.c;
    
    /* Union initializers */
    union Data {
        int i;
        float f;
        char str[20];
    };
    
    union Data d1 = {.i = 42};
    union Data d2 = {.f = 3.14f};
    union Data d3 = {.str = "test"};
    
    checksum += d1.i + (int)d2.f + (int)d3.str[0];
}

/* ========== OpenMP tests ========== */
void test_omp_clauses(void) {
    int i, sum = 0;
    int array[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        array[i] = i + 1;
    }
    
    /* Multiple OpenMP pragmas with various clauses */
    #pragma omp parallel for private(i) shared(array, sum) reduction(+:sum) schedule(dynamic, 5) if(100 > 50)
    for (i = 0; i < 100; i++) {
        sum += array[i];
    }
    checksum += sum;
    
    /* Another OpenMP construct with different clauses */
    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val) collapse(2) ordered
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            int val = x * 10 + y;
            #pragma omp ordered
            {
                if (val > max_val) max_val = val;
            }
        }
    }
    checksum += max_val;
    
    /* Sections with private/firstprivate */
    int section_var = 100;
    #pragma omp parallel sections private(section_var) firstprivate(checksum)
    {
        #pragma omp section
        {
            section_var = 1;
            checksum += section_var;
        }
        #pragma omp section
        {
            section_var = 2;
            checksum += section_var;
        }
    }
}

#ifdef __cplusplus
/* ========== C++ specific tests (TREE_BINFO) ========== */
class Base {
public:
    virtual void vfunc() { checksum += 1; }
    int base_data;
    Base() : base_data(10) {}
};

class Derived : public Base {
public:
    virtual void vfunc() override { checksum += 2; }
    int derived_data;
    Derived() : derived_data(20) {}
};

class Derived2 : public Derived {
public:
    virtual void vfunc() override { checksum += 3; }
    int derived2_data;
    Derived2() : derived2_data(30) {}
};

void test_binfo(void) {
    Derived d;
    Derived2 d2;
    
    /* Access base class members */
    d.base_data = 100;
    d2.base_data = 200;
    
    /* Virtual function calls */
    Base* b1 = &d;
    Base* b2 = &d2;
    
    b1->vfunc();  /* Should call Derived::vfunc */
    b2->vfunc();  /* Should call Derived2::vfunc */
    
    /* Multiple inheritance-like access */
    checksum += d.base_data + d.derived_data;
    checksum += d2.base_data + d2.derived_data + d2.derived2_data;
    
    /* Casts that require BINFO lookups */
    Derived* pd = static_cast<Derived*>(b1);
    checksum += pd->derived_data;
}
#endif

/* ========== Main test driver ========== */
int main(void) {
    printf("Starting tree node coverage tests...\n");
    
    /* Run all tests */
    test_identifiers();
    printf("  IDENTIFIER_NODE tests complete\n");
    
    test_tree_vec();
    printf("  TREE_VEC tests complete\n");
    
    checksum += test_ssa_names(20);
    printf("  SSA_NAME tests complete\n");
    
    test_blocks();
    printf("  BLOCK tests complete\n");
    
    test_constructors();
    printf("  CONSTRUCTOR tests complete\n");
    
    test_omp_clauses();
    printf("  OMP_CLAUSE tests complete\n");
    
#ifdef __cplusplus
    test_binfo();
    printf("  TREE_BINFO tests complete\n");
#endif
    
    /* Use external identifiers */
    checksum += external_var;
    external_func1(checksum);
    external_func2();
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
