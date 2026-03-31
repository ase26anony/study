if (cond) {
    // x is in [0, 100]
    x = some_value % 100;
} else {
    // x is in [200, 300]
    x = 200 + (some_value % 100);
}
int y = x & 0xFC; // AND after merging paths
