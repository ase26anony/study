#include <stdio.h>

extern int helper(void);
extern int problematic(void);

int main() {
    printf("Helper returned: %d\n", helper());
    printf("Problematic returned: %d\n", problematic());
    return 0;
}
