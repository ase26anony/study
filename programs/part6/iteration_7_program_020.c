int a, b, c;
int original_a = a;  // Store original value if needed
if (a > 0) {
    b = 10;
    // Don't modify 'a' here
} else {
    b = 20;
}
// Modify 'a' here if necessary, with clear intent
a = 5;  // Only if this should always happen
