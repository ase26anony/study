#include <stdio.h>

/* Declare functions from other files */
extern int helper_function(void);
extern int compute_value(int x);
extern void assembly_function(void);

/* Function that will be called from assembly */
void c_function_from_asm(void) {
    printf("Called from assembly\n");
}

int main(void) {
    printf("Main program starting\n");
    
    /* Call helper from helper.c */
    int result = helper_function();
    printf("Helper returned: %d\n", result);
    
    /* Call compute from compute.c */
    int computed = compute_value(42);
    printf("Computed value: %d\n", computed);
    
    /* Call assembly function */
    assembly_function();
    
    printf("Main program ending\n");
    return 0;
}
