if (x > 0) {
    // Can't optimize if any instruction here modifies 'x'
    // (except labels, notes, or debug info)
    x = 10;  // This would make the function return false
    // ... other code
}
