/* main.c - Main program that calls functions from other modules */
#include <stdio.h>

/* External declarations */
extern int helper_function(void);
extern int asm_function(void);
extern void problematic_function(void);

int main(void) {
    printf("Main program starting\n");
    
    int result1 = helper_function();
    printf("Helper function returned: %d\n", result1);
    
    int result2 = asm_function();
    printf("Assembly function returned: %d\n", result2);
    
    /* This will cause a link error if problematic_function is called */
    /* problematic_function(); */
    
    printf("Main program completed successfully\n");
    return 0;
}
