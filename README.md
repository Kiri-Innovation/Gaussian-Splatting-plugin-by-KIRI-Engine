<img width="1280" height="720" alt="1 0 main" src="https://github.com/user-attachments/assets/9260c0f4-15ce-4848-9b0d-24e65a168c7f" />


# Gaussian Splatting plugin by KIRI Engine

Render, style, and animate Gaussian Splat scenes directly inside Adobe After Effects.

The Gaussian Splatting plugin by KIRI Engine is a free After Effects plugin for artists, motion designers, and VFX creators who want to bring Gaussian Splat captures into a familiar compositing workflow. The plugin lets you view splat scenes through an AE camera and art-direct the result with creative controls for color, crop, opacity, scale, noise, displacement, and density.


## What It Does

- Render Gaussian Splat scenes inside After Effects
- Use After Effects cameras for flythroughs, reveals, and shot design
- Align, scale, rotate, and place splat scenes in a composition
- Recolor splats with gradients, ramps, and shaped color areas
- Create crop reveals, ghost fades, digital dissolves, noise distortion, displacement warps, and density effects
- Combine splat renders with the rest of an After Effects comp
- Add depth of field blur effects


## Download

Download the latest available plugin build from the **Releases** section of this GitHub repository:

https://github.com/Kiri-Innovation/Gaussian-Splatting-plugin-by-KIRI-Engine/releases

The release package may include:

- `Kiri_GaussianSplatting`
- `Kiri_PlyImporter`


## Installation

### Windows

Copy the plugin files into the After Effects Plug-ins folder.

The default location is usually:

```text
C:\Program Files\Adobe\Adobe After Effects 2026\Support Files\Plug-ins
```

After copying the plugin files, restart After Effects.


### macOS

Copy the plugin files into the After Effects Plug-ins folder.

The default location is usually:

```text
/Applications/Adobe After Effects 2026/Plug-ins
```

After copying the plugin files, restart After Effects.
If macOS blocks the plugin from loading, you may need to allow it in macOS security settings.


## Quick Start

1. Create a new After Effects composition.
2. Add a solid layer.
3. Add an After Effects camera.
4. Import your Gaussian Splat file.
5. Apply the effect to the solid layer.
6. Use `Select Footage` to choose the imported splat.
7. Use Align and Transform controls to frame the scene.
8. Animate the AE camera or plugin controls to create your shot.


## Documentation

Full written documentation can be found on our Tools page here:

https://www.kiriengine.app/3d-tools


## Tutorial Videos

Tutorial videos can be found on our Tools Youtube channel:

https://www.youtube.com/@3D-Tools-by-KIRI-Engine


## Feedback

There may be bugs, incomplete features, or workflows that change between releases. We appreciate all bug reports, feedback, examples, and suggestions from users.

Because this is a free plugin, we may not always be able to fix issues or publish updates as quickly as users would like. We will do our best to improve the plugin over time and keep releases moving when possible.


## Build From Source

Clone the repository with submodules:

```bash
git clone --recursive https://github.com/Kiri-Innovation/Gaussian-Splatting-plugin-by-KIRI-Engine.git

```

### macOS

```bash
mkdir build
cd build
cmake .. -GXcode -DCMAKE_POLICY_VERSION_MINIMUM=3.5
```

Open `Kiri_AE_3DGS.xcodeproj` and build `ALL_BUILD`.

### Windows

```cmd
md build
cd build
cmake ..
```

Open `Kiri_AE_3DGS.sln` and build `ALL_BUILD`.

After building, copy the generated plugin files into the After Effects Plug-ins folder.


## Credits And Acknowledgements

Thanks to everybody who contributed from the KIRI Engine team.
