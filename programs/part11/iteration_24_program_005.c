/* Simple test program to generate coverage data files */
int main() {
    int x = 0;
    
    /* Create some basic control flow for coverage */
    if (x == 0) {
        x = 1;
    } else {
        x = 2;
    }
    
    for (int i = 0; i < 3; i++) {
        x += i;
    }
    
    return x > 0 ? 0 : 1;
}
