x₀ = 0
for (int i = 0; i < n; i++) {
    // Phi node needed here
    x₁ = φ(x₀, x₃)  // At loop entry: x₁ = x₀ (first iteration), x₃ (subsequent iterations)
    if (x₁ == 0) {
        x₂ = 1;
    } else {
        x₃ = 0;
    }
    // Need another phi to merge x₂ and x₃ for next iteration
    x₄ = φ(x₂, x₃)  // This becomes x₃ for the next iteration's φ
}
