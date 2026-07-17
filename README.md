# CoEngine ⚡

A high-performance, multi-threaded procedural generation engine designed for custom Geode mod layers in Geometry Dash.

## 🛠️ Architecture Highlights
* **Thread-Safe Core:** Asynchronous, frame-budgeted execution pipelines via `TaskManager`.
* **Multi-Mode Generation:** Dynamic support for Linear Horizontal tracks, Spatial Platformer environments, and Perlin-noise terrain maps.
* **Decoupled Memory:** Highly optimized object recycling framework utilizing a localized `ObjectPool`.

## 🚀 Building the Project
Ensure you have the [Geode CLI](https://geode-sdk.org/) installed and configured.

```bash
# Clone the repository
git clone [https://github.com/yourusername/CoEngine.git](https://github.com/yourusername/CoEngine.git)
cd CoEngine

# Build the geode profile
geode profile build
