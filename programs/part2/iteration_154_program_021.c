void test() {
    int local = 5;
    // Acquire lock/mutex
    if (g > local) {
        g = local;
    }
    // Release lock/mutex
}
