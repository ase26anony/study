#include <stdio.h>

// Declarations from other files
extern int helper_function(void);
extern int another_helper(void);
extern void assembly_function(void);

int main() {
    printf("Main function starting\n");
    
    int result = helper_function();
    printf("Helper returned: %d\n", result);
    
    another_helper();
    assembly_function();
    
    printf("Main function completed\n");
    return 0;
}
