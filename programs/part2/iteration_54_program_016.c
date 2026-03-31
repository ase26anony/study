// Option 1: Use a different variable
int x = 5;
if (x > 0) {
    x = 10;  // Still modifies x, but clearer
}

// Option 2: Extract to a variable
int x = 5;
bool shouldUpdate = (x > 0);
if (shouldUpdate) {
    x = 10;
}

// Option 3: Use a temporary variable
int x = 5;
int newValue = x;
if (x > 0) {
    newValue = 10;
}
x = newValue;
