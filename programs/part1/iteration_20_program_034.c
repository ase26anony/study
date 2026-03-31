if (x == 1) {
    // bb1
    y = 1;
} else {
    // bb2
    y = 0;
}
// bb3 (merge point)
z = y;  // PHI node for y
if (z == 1)  // This conditional might be optimized
