#include <stdio.h>

// Declarations from other files
extern int helper_function(void);
extern int error_function(void);

int main(void) {
    printf("Main program starting\n");
    
    int result = helper_function();
    printf("Helper returned: %d\n", result);
    
    // This will cause linking error if error_function is undefined
    // int error_result = error_function();
    // printf("Error function returned: %d\n", error_result);
    
    return 0;
}
