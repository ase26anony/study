void test() {
    int local = 5;
    int current_g = g;  // Read once
    if (current_g > local) {
        g = local;      // Write based on captured value
    }
}
