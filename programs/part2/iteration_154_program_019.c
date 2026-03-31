void test() {
    int local = 5;
    int g_copy = g;  // Take a snapshot
    if (g_copy > local) {
        g = local;
    }
}
