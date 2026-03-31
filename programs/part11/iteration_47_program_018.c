x₀ = 0
for i = 0; i < n; i++:
    x₁ = φ(x₀, x₄)  // Phi node at loop entry
    if (x₁ == 0):
        x₂ = 1
    else:
        x₃ = 0
    x₄ = φ(x₂, x₃)  // Phi node after if-else
