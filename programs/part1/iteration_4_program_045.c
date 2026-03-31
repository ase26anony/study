if (x > 0) {
    // This loop checks that 'x' isn't modified here
    // so we can safely optimize based on (x > 0)
    y = x * 2;
}
