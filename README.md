VR Fork of [dynamic container inventory framework](https://github.com/SeaSparrowOG/DynamicContainerInventoryFramework)

## Building
### Requirements:
- CMake
- VCPKG
- Visual Studio (with desktop C++ development)
- [CommonLibVR](https://github.com/alandtse/CommonLibVR/tree/vr)
---
### Instructions:
setup commonlibVR build and path, see [vr address tools readme](https://github.com/alandtse/vr_address_tools) for reference

```
git clone https://github.com/andperks6/DynamicContainerInventoryFramework
cd DynamicContainerInventoryFramework

cmake --preset vs2022-windows-vcpkg 
cmake --build build --config Release
```
---

