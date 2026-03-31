/* main.c - Main program that calls functions from other files */
#include <stdio.h>

/* External declarations */
extern int helper_function(void);
extern int compute_value(int x);
extern void assembly_function(void);

/* Function with deliberate error in another file */
extern void problematic_function(void);

int main(void) {
    printf("Test program for GCC driver coverage\n");
    
    /* Call functions from other files */
    int result = helper_function();
    printf("Helper returned: %d\n", result);
    
    int computed = compute_value(42);
    printf("Computed value: %d\n", computed);
    
    /* Call assembly function */
    assembly_function();
    
    /* This will cause linking error if error.c is included */
    /* problematic_function(); */
    
    return 0;
}
