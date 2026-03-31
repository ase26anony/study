/* main.c - Main program that calls helper functions */
#include <stdio.h>

/* External declarations */
extern int helper_function(void);
extern int asm_helper(void);
extern void problematic_function(void);

int main(void) {
    int result1 = helper_function();
    int result2 = asm_helper();
    
    printf("Helper function returned: %d\n", result1);
    printf("Assembly helper returned: %d\n", result2);
    
    /* This will cause a link error if error.c is included */
    /* problematic_function(); */
    
    return 0;
}
