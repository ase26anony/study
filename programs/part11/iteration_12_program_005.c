#include <stdio.h>

// Declarations for functions defined in other files
extern int helper_function(void);
extern int assembly_function(void);
extern void problematic_function(void);

int main(void) {
    printf("Main program starting...\n");
    
    int result1 = helper_function();
    printf("Helper function returned: %d\n", result1);
    
    int result2 = assembly_function();
    printf("Assembly function returned: %d\n", result2);
    
    // This will cause a link error if error.c is included
    // problematic_function();
    
    printf("Main program completed successfully.\n");
    return 0;
}
