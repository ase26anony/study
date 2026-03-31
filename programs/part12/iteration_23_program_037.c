/* test_targhooks.c - Combined triggers for compiler-generated artificial symbols */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#ifdef __cplusplus
#include <typeinfo>
#endif

/* Pattern 1: AddressSanitizer triggers */
#ifdef __SANITIZE_ADDRESS__
void asan_stack_overflow() {
    int arr[10];
    arr[15] = 0;  /* Stack buffer overflow - ASan will instrument */
}

void asan_use_after_return() {
    static int* ptr;
    {
        int local = 42;
        ptr = &local;  /* Use after return - ASan may create artificial vars */
    }
    /* Don't actually use ptr to avoid crash without ASan */
}
#endif

/* Pattern 2: ThreadSanitizer triggers */
volatile int shared_counter = 0;

void* thread_func(void* arg) {
    for (int i = 0; i < 1000; i++) {
        shared_counter++;  /* Data race - TSan will instrument */
    }
    return NULL;
}

void tsan_data_race() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_func, NULL);
    pthread_create(&t2, NULL, thread_func, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
}

/* Pattern 3: OpenMP artificial symbols */
#ifdef _OPENMP
void omp_reduction() {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 1000; ++i) {
        sum += i;  /* OpenMP creates internal reduction symbols */
    }
    printf("OpenMP sum: %d\n", sum);
}

void omp_private_vars() {
    int n = 100;
    #pragma omp parallel private(n)
    {
        n = omp_get_thread_num();
        /* Thread-private variable handling creates artificial symbols */
    }
}
#endif

/* Pattern 4: Complex inline assembly */
void complex_asm() {
    int x = 42, y = 0;
    
    /* Extended asm with multiple clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (y)
        : "r" (x)
        : "%eax", "%ebx", "memory"
    );
    
    printf("Assembly result: %d\n", y);
}

/* Pattern 5: C++ specific triggers (compiled as C++ only) */
#ifdef __cplusplus
class Base {
public:
    virtual void foo() { printf("Base::foo\n"); }
    virtual ~Base() {}
};

class Derived : public Base {
public:
    virtual void foo() override { printf("Derived::foo\n"); }
};

/* Static local in inline function - may create guard variables */
inline int get_static_value() {
    static int counter = 0;
    return ++counter;
}

void cpp_rtti_and_exceptions() {
    /* RTTI usage */
    Base* b = new Derived();
    printf("Type: %s\n", typeid(*b).name());
    
    /* Exception handling */
    try {
        if (dynamic_cast<Derived*>(b)) {
            throw std::bad_cast();  /* Personality routine symbols */
        }
    } catch (const std::exception& e) {
        printf("Exception caught: %s\n", e.what());
    }
    
    /* Static local variable */
    printf("Static value: %d\n", get_static_value());
    
    delete b;
}
#endif

/* Pattern 6: Profile-guided optimization triggers */
void pgo_hot_function() {
    /* Hot loop for PGO instrumentation */
    volatile int result = 0;
    for (int i = 0; i < 10000; i++) {
        result += i * i;
    }
    printf("PGO result: %d\n", result);
}

/* Pattern 7: Builtin usage */
void builtin_usage() {
    /* Use GCC builtins that may create artificial symbols */
    int x = 42;
    printf("Builtin abs: %d\n", __builtin_abs(x));
    printf("Builtin popcount: %d\n", __builtin_popcount(0xFF));
    
    /* Memory builtins */
    char src[100], dst[100];
    __builtin_memcpy(dst, src, sizeof(src));
    __builtin_memset(src, 0, sizeof(src));
}

/* Main orchestrator */
int main(int argc, char** argv) {
    int run_sanitizer_tests = 0;
    
    /* Check if we should run sanitizer tests */
    if (argc > 1 && strcmp(argv[1], "--run-sanitizers") == 0) {
        run_sanitizer_tests = 1;
    }
    
    printf("=== Starting compiler symbol trigger tests ===\n");
    
    /* Always run these (safe without sanitizers) */
    pgo_hot_function();
    builtin_usage();
    
#ifdef _OPENMP
    omp_reduction();
    omp_private_vars();
#endif
    
    complex_asm();
    
#ifdef __cplusplus
    cpp_rtti_and_exceptions();
#endif
    
    /* Conditionally run sanitizer-triggering code */
    if (run_sanitizer_tests) {
#ifdef __SANITIZE_ADDRESS__
        printf("\nRunning ASan tests...\n");
        asan_stack_overflow();
        asan_use_after_return();
#endif
        
        printf("\nRunning TSan test...\n");
        tsan_data_race();
    } else {
        printf("\nSanitizer tests skipped (use --run-sanitizers to enable)\n");
    }
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
