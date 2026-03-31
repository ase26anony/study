#include <stdio.h>

extern int helper(void);
extern int problematic(void);

int main() {
    printf("Main program started\n");
    int result = helper();
    printf("Helper returned: %d\n", result);
    
    // This will cause a link error if problematic.o is compiled
    // int bad = problematic();
    
    return 0;
}
