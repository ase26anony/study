// If some_condition(i) always returns true:
int val = 0;
for (int i = 0; i < n; ++i) {
    val = 1;  // Path A always taken
    // val is always 1, so if (val == 0) is always false
    // The entire if block can be eliminated
}
