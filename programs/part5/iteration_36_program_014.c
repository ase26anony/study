/* First test function */
int compute1(int x) { 
    return x * 2; 
}

int helper1(int a, int b) {
    return a + b + compute1(a);
}
