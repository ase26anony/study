int a, b, c;
int original_a = a;  // Store original if needed
if (a > 0) {
    b = 10;
    // Use a different variable if you need to store a new value
    int new_value = 5;
    // Or if you must update a, do it consciously:
    a = 5;  // But be aware this changes a for subsequent code
} else {
    b = 20;
}
