#include <stdio.h>

extern int helper(void);
extern int another_helper(void);

int main(void) {
    printf("Helper returned: %d\n", helper());
    printf("Another helper returned: %d\n", another_helper());
    return 0;
}
