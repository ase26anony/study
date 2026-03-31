/* main.c - Main program that calls functions from other modules */
#include <stdio.h>

/* External declarations */
extern int helper_function(void);
extern int another_helper(void);
extern void assembly_function(void);
extern int problematic_function(void);

int main(void) {
    printf("Main program starting...\n");
    
    int result1 = helper_function();
    printf("Helper function returned: %d\n", result1);
    
    int result2 = another_helper();
    printf("Another helper returned: %d\n", result2);
    
    assembly_function();
    
    /* This will fail to link if problematic_function is called */
    /* int bad_result = problematic_function(); */
    
    printf("Main program completed successfully.\n");
    return 0;
}
