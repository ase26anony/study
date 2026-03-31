// Safe comparison pattern
if (isunordered(a, b)) {
    // Handle NaN case
} else if (a > b) {
    // Safe comparison - no NaN present
}

// Using unordered comparison functions
if (isgreater(a, b)) {  // No exception even if a or b is NaN
    // a > b and neither is NaN
}
