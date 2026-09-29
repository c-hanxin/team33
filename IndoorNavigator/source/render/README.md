# `source/render/`

**Role**: OpenGL ES rendering subsystem (GL ES 3.0 on Android, GL 4.3 core context on Desktop).
**Owner**: Santhosh (Graphic Programmer).
**Layering Rule**:
- Reads simulation state and **never writes it**.
- Implements tactical holographic campus visualization (isometric map view, route ribbons, user location markers).
- Contains M1 "Hello Triangle" / campus mesh rendering logic.
