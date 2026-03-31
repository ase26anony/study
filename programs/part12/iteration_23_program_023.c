/* Combined test program to trigger compiler-generated artificial symbols
   with specific attributes (ARTIFICIAL, IGNORED, HIDDEN visibility, etc.) */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/* Pattern 1: AddressSanitizer triggers */
#ifdef __SANITIZE_ADDRESS__
void asan_stack_overflow() {
    int arr[10];
    /* Potential stack buffer overflow - ASan will instrument this */
    arr[15] = 0;  /* Out-of-bounds write */
}

void asan_use_after_return() {
    static int* ptr;
    {
        int local = 42;
        ptr = &local;  /* Use-after-return potential */
    }
    /* Don't actually use ptr to avoid crash without ASan */
}
#endif

/* Pattern 2: ThreadSanitizer triggers */
volatile int shared_counter = 0;

void* thread_func(void* arg) {
    for (int i = 0; i < 1000; i++) {
        shared_counter++;  /* Data race potential */
    }
    return NULL;
}

void tsan_data_race() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_func, NULL);
    pthread_create(&t2, NULL, thread_func, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Shared counter: %d\n", shared_counter);
}

/* Pattern 3: OpenMP internal symbols */
#ifdef _OPENMP
#include <omp.h>
void openmp_reduction() {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 1000; ++i) {
        sum += i;
    }
    printf("OpenMP reduction sum: %d\n", sum);
}

void openmp_private_vars() {
    int n = 1000;
    #pragma omp parallel for private(n)
    for (int i = 0; i < 100; ++i) {
        n = i * 2;
        /* n is private to each thread */
    }
}
#endif

/* Pattern 4: Complex inline assembly */
void inline_assembly_clobbers() {
    int x = 42, y = 0;
    
    /* Extended asm with multiple clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (y)
        : "r" (x)
        : "%eax", "%ebx", "memory", "cc"
    );
    
    printf("Inline assembly result: %d\n", y);
}

/* Pattern 5: C++ features (compile as C++ for these) */
#ifdef __cplusplus
#include <typeinfo>

/* Static local with potential guard variable */
int get_singleton() {
    static int singleton = 12345;
    return singleton;
}

/* Virtual function for RTTI */
class Base {
public:
    virtual ~Base() {}
    virtual void foo() { printf("Base::foo\n"); }
};

class Derived : public Base {
public:
    virtual void foo() override { printf("Derived::foo\n"); }
};

void cpp_rtti_test() {
    Base* b = new Derived();
    
    /* Use typeid which requires typeinfo */
    printf("Type: %s\n", typeid(*b).name());
    
    /* Use dynamic_cast */
    Derived* d = dynamic_cast<Derived*>(b);
    if (d) {
        d->foo();
    }
    
    delete b;
}

/* Exception handling */
void exception_test() {
    try {
        throw 42;
    } catch (int e) {
        printf("Caught exception: %d\n", e);
    }
}
#endif

/* Pattern 6: Profile-guided optimization counters */
void pgo_hot_function() {
    /* Loop that should trigger PGO instrumentation */
    volatile int result = 0;
    for (int i = 0; i < 10000; i++) {
        result += i * i;
    }
    printf("PGO function result: %d\n", result);
}

/* Main orchestrator */
int main(int argc, char** argv) {
    int run_sanitizer_tests = 0;
    
    /* Check if we should run sanitizer-triggering code */
    if (argc > 1) {
        run_sanitizer_tests = atoi(argv[1]);
    }
    
    printf("=== Starting artificial symbol generation tests ===\n");
    
    /* Always run safe tests */
    printf("\n1. Running OpenMP tests...\n");
    #ifdef _OPENMP
    openmp_reduction();
    openmp_private_vars();
    #else
    printf("OpenMP not supported\n");
    #endif
    
    printf("\n2. Running inline assembly test...\n");
    inline_assembly_clobbers();
    
    printf("\n3. Running PGO hot function...\n");
    pgo_hot_function();
    
    printf("\n4. Running ThreadSanitizer test...\n");
    tsan_data_race();
    
    #ifdef __cplusplus
    printf("\n5. Running C++ feature tests...\n");
    printf("Singleton value: %d\n", get_singleton());
    cpp_rtti_test();
    exception_test();
    #endif
    
    /* Only run potentially dangerous ASan tests if requested */
    if (run_sanitizer_tests) {
        printf("\n6. Running AddressSanitizer tests...\n");
        #ifdef __SANITIZE_ADDRESS__
        asan_stack_overflow();
        asan_use_after_return();
        #else
        printf("ASan not enabled at compile time\n");
        #endif
    } else {
        printf("\n6. Skipping ASan tests (run with './program 1' to enable)\n");
    }
    
    printf("\n=== Tests completed ===\n");
    return 0;
}
