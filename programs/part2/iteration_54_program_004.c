// Option 1: Use a temporary variable
int x = 5;
int original_x = x;
if (original_x > 0) {
    x = 10;
}

// Option 2: Restructure if modification is conditional
int x = 5;
if (x > 0) {
    // Perform operations without modifying x in condition checks
    int new_value = 10;
    x = new_value;
}
