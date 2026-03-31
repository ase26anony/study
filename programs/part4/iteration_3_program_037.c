/* 
 * This program is designed to trigger GCC's internal tree initialization
 * for compiler-generated artificial functions with hidden visibility.
 * It combines multiple patterns that force GCC to create internal declarations:
 * 1. Thread-local storage with dynamic initialization
 * 2. Nested functions with non-local jumps using setjmp/longjmp
 * 3. Complex static initializers with side effects
 * 4. External linkage and volatile variables
 * 5. Exception handling context via nothrow attribute
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <time.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void);

/* Weak attribute to simulate multi-file compilation */
int external_helper(void) __attribute__((weak));

/* Thread-local storage with non-constant initializer */
__thread int tls_var = 0;

/* Function to dynamically initialize TLS */
int init_tls(void) {
    return rand() % 100 + 1;
}

/* Volatile variable to prevent optimization */
volatile int volatile_counter = 0;

/* Complex static initializer with side effects */
static int static_with_side_effect = (printf("Initializing static...\n"), 42);

/* Nested function using setjmp/longjmp - GCC extension */
void nested_function_with_jmpbuf(jmp_buf env) {
    /* This nested function may cause GCC to generate artificial helpers */
    auto void nested_helper(void) __attribute__((nothrow));
    
    void nested_helper(void) {
        volatile_counter++;
        if (volatile_counter < 3) {
            longjmp(env, 1);
        }
    }
    
    nested_helper();
}

/* Function marked nothrow to provide context for TREE_NOTHROW flag */
void process_with_exception_context(void) __attribute__((nothrow));

void process_with_exception_context(void) {
    jmp_buf jump_buffer;
    volatile int retry_count = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First entry - initialize TLS with dynamic value */
        tls_var = init_tls();
        printf("TLS initialized to: %d\n", tls_var);
        
        /* Call nested function that may trigger internal declaration */
        nested_function_with_jmpbuf(jump_buffer);
    } else {
        /* Longjmp return path */
        retry_count++;
        printf("Longjmp return, retry: %d\n", retry_count);
        
        if (retry_count < 2) {
            /* Modify TLS and jump again */
            tls_var += external_helper();
            longjmp(jump_buffer, 1);
        }
    }
}

/* Simulated external helper function */
int external_helper(void) {
    /* This might be defined in another compilation unit */
    static int counter = 0;
    return ++counter;
}

/* Main function with observable I/O */
int main(void) {
    /* Setup */
    srand(time(NULL));
    printf("Program start\n");
    
    /* Initialize TLS before use */
    tls_var = init_tls();
    
    /* Loop to ensure execution of critical paths */
    for (int i = 0; i < 2; i++) {
        printf("Iteration %d:\n", i + 1);
        
        /* Execute the construct designed to trigger internal declaration generation */
        process_with_exception_context();
        
        /* Access TLS variable to ensure it's used */
        printf("TLS value: %d\n", tls_var);
        
        /* Use static variable with side effect */
        printf("Static value: %d\n", static_with_side_effect + i);
        
        /* Call external function to influence linkage */
        int ext_result = external_helper();
        printf("External helper result: %d\n", ext_result);
    }
    
    /* Final observable output */
    printf("Final volatile counter: %d\n", volatile_counter);
    printf("Program completed successfully\n");
    
    return 0;
}
