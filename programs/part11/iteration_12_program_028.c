/* main.c - Main program that calls functions from other files */
#include <stdio.h>

/* External declarations */
extern int helper_function(void);
extern int compute_value(int);
extern void assembly_function(void);

/* Function with deliberate error in another file */
extern void problematic_function(void);

int main(void) {
    printf("Test program starting...\n");
    
    /* Call helper from helper.c */
    int result = helper_function();
    printf("Helper returned: %d\n", result);
    
    /* Call compute from compute.c */
    int computed = compute_value(5);
    printf("Computed value: %d\n", computed);
    
    /* Call assembly function */
    assembly_function();
    
    /* This will cause a link error if problematic_function is not defined */
    /* problematic_function(); */
    
    return 0;
}
