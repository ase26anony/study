// Option 1: Use a different variable
int x = 5;
if (x > 0) {
    x = 10;  // Still modifies x, but at least it's clear
}

// Option 2: Use a temporary variable
int x = 5;
int result = x;
if (x > 0) {
    result = 10;
}
// Use result instead of x

// Option 3: Ternary operator (for simple cases)
int x = 5;
x = (x > 0) ? 10 : x;
