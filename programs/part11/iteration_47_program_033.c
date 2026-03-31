x₀ = 0
for (int i = 0; i < n; i++) {
    x₁ = φ(x₀, x₄)  // Phi node: x₁ gets x₀ on first iteration, x₄ on subsequent iterations
    if (x₁ == 0) {
        x₂ = 1
    } else {
        x₃ = 0
    }
    x₄ = φ(x₂, x₃)  // Another phi to merge the two possible values
}
