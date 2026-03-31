/* Combined test program to trigger artificial symbol generation in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifdef __cplusplus
#include <typeinfo>
#endif

/* Pattern 1: AddressSanitizer triggers */
void asan_stack_overflow() {
    volatile int arr[10];
    arr[15] = 0;  /* Potential stack buffer overflow */
}

int* use_after_return() {
    int local = 42;
    return &local;  /* Use after return warning */
}

/* Pattern 2: ThreadSanitizer triggers */
int shared_var = 0;

void* thread_func(void* arg) {
    for (int i = 0; i < 1000; i++) {
        shared_var++;  /* Data race */
    }
    return NULL;
}

void create_data_race() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_func, NULL);
    pthread_create(&t2, NULL, thread_func, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Shared var: %d\n", shared_var);
}

/* Pattern 3: OpenMP artificial symbols */
#ifdef _OPENMP
#include <omp.h>
void openmp_reduction() {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 1000; ++i) {
        sum += i;
    }
    printf("OpenMP sum: %d\n", sum);
    
    /* Thread-private variables */
    #pragma omp parallel
    {
        static int thread_specific = 0;
        #pragma omp threadprivate(thread_specific)
        thread_specific++;
    }
}
#endif

/* Pattern 4: Complex inline assembly */
void inline_assembly_test() {
    int x = 42, y = 0;
    
    /* Multiple clobbered registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (y)
        : "r" (x)
        : "%eax", "%ebx", "%ecx", "memory"
    );
    
    printf("Assembly result: %d\n", y);
    
    /* asm goto */
    asm goto (
        "cmpl $100, %0\n\t"
        "jg %l[label]\n\t"
        : : "r" (y) : "memory" : label
    );
    
    printf("Didn't jump\n");
    return;
    
label:
    printf("Jumped!\n");
}

#ifdef __cplusplus
/* Pattern 5: C++ features generating artificial symbols */

/* Static local with thread-safe init */
inline int static_local_func() {
    static int counter = 0;
    return ++counter;
}

/* Virtual functions and RTTI */
class Base {
public:
    virtual ~Base() {}
    virtual void foo() { printf("Base::foo\n"); }
};

class Derived : public Base {
public:
    virtual void foo() override { printf("Derived::foo\n"); }
};

void cpp_exceptions() {
    try {
        throw 42;
    } catch (int e) {
        printf("Caught exception: %d\n", e);
    }
}

void cpp_rtti_test() {
    Base* b = new Derived();
    
    /* Trigger typeinfo generation */
    const std::type_info& ti = typeid(*b);
    printf("Type: %s\n", ti.name());
    
    /* dynamic_cast with RTTI */
    Derived* d = dynamic_cast<Derived*>(b);
    if (d) {
        d->foo();
    }
    
    delete b;
}
#endif

/* Pattern 6: Profile-guided optimization triggers */
void pgo_hot_function() {
    volatile int counter = 0;
    for (int i = 0; i < 10000; i++) {
        counter += i * i;
        if (counter % 7 == 0) {
            counter /= 3;
        }
    }
    printf("PGO counter: %d\n", counter);
}

/* Main orchestrator */
int main(int argc, char** argv) {
    int run_sanitizer_tests = 0;
    
    /* Check if we should run sanitizer tests */
    if (argc > 1 && strcmp(argv[1], "--safe") != 0) {
        run_sanitizer_tests = 1;
    }
    
    printf("Starting artificial symbol generation test...\n");
    
    /* Always run safe patterns */
    printf("\n1. Running OpenMP patterns...\n");
    #ifdef _OPENMP
    openmp_reduction();
    #endif
    
    printf("\n2. Running inline assembly patterns...\n");
    inline_assembly_test();
    
    printf("\n3. Running PGO hot function...\n");
    pgo_hot_function();
    
    #ifdef __cplusplus
    printf("\n4. Running C++ patterns...\n");
    for (int i = 0; i < 5; i++) {
        printf("Static local: %d\n", static_local_func());
    }
    cpp_exceptions();
    cpp_rtti_test();
    #endif
    
    /* Conditionally run sanitizer patterns */
    if (run_sanitizer_tests) {
        printf("\n5. Running sanitizer patterns (may trigger errors)...\n");
        asan_stack_overflow();
        use_after_return();
        create_data_race();
    } else {
        printf("\n5. Skipping sanitizer patterns (use --unsafe to enable)\n");
    }
    
    printf("\nTest completed.\n");
    return 0;
}
