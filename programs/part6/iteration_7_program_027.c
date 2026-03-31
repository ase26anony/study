int a, b, c;
int original_a = a;  // Store original if needed
if (a > 0) {
    b = 10;
    // Don't modify 'a' here
} else {
    b = 20;
}
a = 5;  // Modify 'a' outside the conditional if needed
