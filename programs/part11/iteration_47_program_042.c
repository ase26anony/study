x₀ = 0
for (int i = 0; i < n; i++) {
    // Phi node needed here
    x₁ = φ(x₀, x₃)  // At loop entry: x₁ = x₀ (first iteration), x₁ = x₃ (subsequent iterations)
    if (x₁ == 0) {
        x₂ = 1;
    } else {
        x₃ = 0;
    }
    // Need another phi to merge x₂ and x₃
    x₄ = φ(x₂, x₃)  // This becomes the value for next iteration
}
