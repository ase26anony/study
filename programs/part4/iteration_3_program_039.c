/* test_targhooks.c
 * 
 * A minimal C program designed to trigger GCC's internal tree initialization
 * for compiler-generated declarations with specific flags:
 *   TREE_STATIC, TREE_PUBLIC, DECL_EXTERNAL, TREE_USED,
 *   TREE_THIS_VOLATILE, TREE_NOTHROW, DECL_ARTIFICIAL,
 *   DECL_IGNORED_P, DECL_VISIBILITY_SPECIFIED, VISIBILITY_HIDDEN
 *
 * Compile with: gcc -O3 -fPIC -ftls-model=initial-exec -fvisibility=hidden -fnon-call-exceptions test_targhooks.c -o test_targhooks
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <pthread.h>

/* External function to force external linkage context */
extern int external_helper(void) __attribute__((weak));

/* Volatile variable to prevent optimization */
volatile int volatile_trigger = 1;

/* Thread-local storage with dynamic initialization */
__thread int tls_var = 0;

/* Function with side effects for TLS initialization */
int init_tls(void) {
    return rand() % 100 + 1;
}

/* Nested function using setjmp/longjmp (GCC extension) */
void nested_function_with_jmp(void) {
    jmp_buf env;
    volatile int counter = 0;
    
    /* Define nested function using GCC's statement expression */
    auto void nested_helper(void) __attribute__((nothrow));
    
    void nested_helper(void) {
        if (counter++ < 3) {
            longjmp(env, 1);
        }
    }
    
    if (setjmp(env) == 0) {
        nested_helper();
    }
    
    /* Access TLS variable to force initialization */
    tls_var = init_tls();
}

/* Function marked nothrow containing the triggering constructs */
void triggering_function(void) __attribute__((nothrow));

void triggering_function(void) {
    static int static_with_side_effect = (printf("Initializing static...\n"), 42);
    
    /* Complex static initializer */
    static struct {
        int a;
        volatile int b;
    } complex_static = { .a = rand(), .b = volatile_trigger };
    
    /* Nested function context */
    nested_function_with_jmp();
    
    /* Use the statics to prevent dead code elimination */
    printf("Static values: %d, %d, %d\n", 
           static_with_side_effect, 
           complex_static.a, 
           complex_static.b);
}

/* Simulate multi-file compilation with weak external */
int external_helper(void) {
    return 0xDEADBEEF;
}

/* Main execution flow with setup and observable I/O */
int main(void) {
    /* Setup */
    srand(42);
    jmp_buf main_env;
    
    printf("Program start\n");
    
    /* Force TLS initialization */
    tls_var = init_tls();
    printf("TLS initialized: %d\n", tls_var);
    
    /* Execute the triggering function multiple times */
    for (int i = 0; i < 2; i++) {
        triggering_function();
        
        /* Use setjmp/longjmp in main to potentially trigger more internal functions */
        if (setjmp(main_env) == 0) {
            if (i == 0 && volatile_trigger) {
                longjmp(main_env, 1);
            }
        }
    }
    
    /* Call external function */
    int ext_result = external_helper();
    printf("External helper result: 0x%X\n", ext_result);
    
    /* Final observable output using all constructs */
    printf("Final TLS value: %d\n", tls_var);
    printf("Volatile trigger: %d\n", volatile_trigger);
    
    return 0;
}

/* Additional function to create more linkage context */
void __attribute__((visibility("default"))) public_function(void) {
    /* This creates contrast with hidden visibility declarations */
    printf("Public function\n");
}

/* Force generation of static initialization guard variables */
void __attribute__((constructor)) init(void) {
    printf("Constructor called\n");
}
