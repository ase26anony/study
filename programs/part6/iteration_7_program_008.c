int a, b, c;
int original_a = a;  // Store original value if needed

if (a > 0) {
    b = 10;
    // Don't modify 'a' here
} else {
    b = 20;
}

// If you need to set a = 5 when a > 0, do it here:
if (original_a > 0) {
    a = 5;
}
