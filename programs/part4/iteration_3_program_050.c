#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void);

/* Weak symbol to simulate multi-file compilation */
int __attribute__((weak)) weak_symbol = 0;

/* Function with nothrow attribute to provide context for TREE_NOTHROW */
void __attribute__((nothrow)) nothrow_function(void) {
    volatile int dummy = 0;
    (void)dummy;
}

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;
__thread int tls_init = (printf("TLS init side effect\n"), 42);

/* Complex static initializer with side effects */
static int static_complex = (printf("Static init side effect\n"), 100);

/* Function that uses setjmp/longjmp for non-local jumps */
int setjmp_test(jmp_buf *env) {
    volatile int counter = 0;
    
    /* Nested function using GCC extension */
    __extension__ int nested_helper(int x) {
        counter++;
        if (x > 10) {
            longjmp(*env, x);
        }
        return x * 2;
    }
    
    int result = 0;
    for (int i = 0; i < 5; i++) {
        result += nested_helper(i);
    }
    
    return result;
}

/* Function with OpenMP pragma to generate offloading stubs */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
void omp_target_function(void) {
    volatile int arr[10];
    
    /* OpenMP target region - may generate internal stub functions */
    #pragma omp target map(tofrom: arr[0:10])
    {
        for (int i = 0; i < 10; i++) {
            arr[i] = i * i;
        }
    }
    
    /* Use the result to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    printf("OpenMP array sum: %d\n", sum);
}
#pragma GCC diagnostic pop

/* External function definition (simulates multi-file) */
int external_helper(void) {
    return rand() % 100;
}

int main(void) {
    jmp_buf env;
    volatile int result = 0;
    
    /* Seed random for external function */
    srand(42);
    
    /* Call nothrow function */
    nothrow_function();
    
    /* Use thread-local storage */
    tls_var = external_helper();
    printf("TLS var: %d\n", tls_var);
    printf("TLS init: %d\n", tls_init);
    
    /* Use static variable with complex initializer */
    printf("Static complex: %d\n", static_complex + 10);
    
    /* Setjmp/longjmp with nested function */
    volatile int setjmp_result = setjmp(env);
    if (setjmp_result == 0) {
        result = setjmp_test(&env);
        printf("Setjmp test result: %d\n", result);
    } else {
        printf("Longjmp caught with value: %d\n", setjmp_result);
    }
    
    /* OpenMP target region (if supported) */
    #ifdef _OPENMP
    omp_target_function();
    #else
    printf("OpenMP not enabled at compile time\n");
    #endif
    
    /* Use weak symbol */
    weak_symbol = 1;
    printf("Weak symbol: %d\n", weak_symbol);
    
    /* Force volatile operations */
    volatile int *volatile_ptr = (volatile int*)malloc(sizeof(int));
    if (volatile_ptr) {
        *volatile_ptr = 12345;
        printf("Volatile value: %d\n", *volatile_ptr);
        free((void*)volatile_ptr);
    }
    
    /* Complex expression with side effects */
    static int counter = 0;
    int complex_expr = (counter++, printf("Counter: %d\n", counter), 
                       external_helper() + tls_var + static_complex);
    
    printf("Final complex expression: %d\n", complex_expr);
    
    return 0;
}
