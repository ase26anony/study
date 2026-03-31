/* 
 * This program is designed to trigger GCC's internal tree initialization
 * for compiler-generated declarations with specific flags:
 * TREE_STATIC=1, TREE_PUBLIC=1, DECL_EXTERNAL=1, TREE_USED=1,
 * TREE_THIS_VOLATILE=1, TREE_NOTHROW=1, DECL_ARTIFICIAL=1,
 * DECL_IGNORED_P=1, DECL_VISIBILITY_SPECIFIED=1, VISIBILITY_HIDDEN
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function declaration to influence DECL_EXTERNAL handling */
extern int external_helper(void);
/* Weak attribute to simulate multi-file linking scenario */
extern int maybe_defined_elsewhere(void) __attribute__((weak));

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;
__thread volatile int tls_volatile = 0;

/* Function that will be marked nothrow */
static int nested_with_jump(void) __attribute__((nothrow));

/* Global jmp_buf for non-local jumps */
static jmp_buf jump_buffer;

/* Function with side effects for dynamic initialization */
static int init_with_side_effects(void) {
    volatile int counter = rand() % 100;
    return counter + 1;
}

/* Static variable with complex initializer */
static int complex_static = (printf("Initializing complex_static\n"), init_with_side_effects());

/* External helper simulation */
int external_helper(void) {
    return rand() % 50;
}

/* Weak function definition */
int maybe_defined_elsewhere(void) {
    return 42;
}

/* Nested function using setjmp/longjmp (GCC extension) */
static int nested_with_jump(void) {
    volatile int local_state = 0;
    
    /* GCC nested function using __extension__ */
    __extension__ int nested_func(int x) {
        if (x > 100) {
            /* Non-local jump - may trigger internal helper generation */
            longjmp(jump_buffer, 1);
        }
        return x * 2;
    };
    
    local_state = nested_func(complex_static);
    
    /* Access TLS with volatile qualifier */
    tls_volatile = local_state;
    
    return local_state;
}

/* Function containing OpenMP pragma for potential offloading stub generation */
static void omp_offload_section(void) {
    volatile int result = 0;
    
    #pragma omp parallel for reduction(+:result)
    for (int i = 0; i < 10; i++) {
        result += i;
    }
    
    tls_var += result;
}

int main(void) {
    int total = 0;
    volatile int should_jump = 0;
    
    /* Seed random for volatile behavior */
    srand(42);
    
    /* Initialize TLS with dynamic value */
    tls_var = external_helper();
    
    /* Set up jump point */
    if (setjmp(jump_buffer) == 0) {
        /* First execution path */
        
        /* Call nested function with jmp_buf usage */
        total += nested_with_jump();
        
        /* Force evaluation of weak symbol */
        if (&maybe_defined_elsewhere) {
            total += maybe_defined_elsewhere();
        }
        
        /* Complex static initialization with side effects */
        static int another_static = (printf("Initializing another_static\n"), 
                                     external_helper() + complex_static);
        total += another_static;
        
        /* OpenMP section that may generate internal functions */
        omp_offload_section();
        
        /* Access volatile TLS */
        should_jump = tls_volatile;
        
        /* Potentially trigger longjmp */
        if (should_jump > 50) {
            /* This will trigger the longjmp in nested_func if executed */
            __extension__ int trigger_jump(int x) {
                if (x > 0) longjmp(jump_buffer, 2);
                return 0;
            };
            trigger_jump(should_jump);
        }
    } else {
        /* longjmp was called */
        total += 1000;
    }
    
    /* Ensure all constructs are used and observable */
    printf("Result: %d\n", total + tls_var + tls_volatile);
    
    /* Additional volatile operations to prevent optimization */
    volatile int final_check = total;
    for (volatile int i = 0; i < 3; i++) {
        final_check += i;
    }
    
    return final_check % 256;
}
