/* main.c - Main program that calls functions from other files */
#include <stdio.h>

/* External declarations */
extern int helper_function(void);
extern int compute_value(int x);
extern void assembly_function(void);
extern int problematic_function(void);  /* Will cause link error */

int main(void) {
    printf("Test program for GCC driver coverage\n");
    
    /* Call helper from helper.c */
    int result = helper_function();
    printf("helper_function() returned: %d\n", result);
    
    /* Call compute from compute.c */
    int computed = compute_value(42);
    printf("compute_value(42) returned: %d\n", computed);
    
    /* Call assembly function */
    assembly_function();
    
    /* This will cause a link error when error.c is included */
    /* int problem = problematic_function(); */
    
    return 0;
}
