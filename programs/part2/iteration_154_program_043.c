void test() {
    int local = 5;
    if (g > local) {
        g = local;  // Thread 1 writes
        // Thread 2 might read a partially updated value
        // or overwrite Thread 1's write
    }
}
