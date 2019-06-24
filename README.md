# ShaderProgramming

## Controls 
- 1 : Marching Cubes Geometrie mit Displacement Mapping
- 2 : Particle System
- 3 : Soft Shadows
- 4 : Tesselation


- W und S Kamera vor und zurück bewegen
- A und D Kamera nach links und rechts bewegen
- Q und E generieren neue Geometrieschritte im MarchingCubes
- R und T regeln wie weich der Soft Shadow sein soll
- M Togglet wireframe für das Tesselierungs Beispiel
- I Erhöht den inneren Layer der Tesselierung
- O erhöht den outer Layer der Tesselierung


## Marching Cubes
Der NoiseShader.vert und NoiseShader.frag befüllen eine 3d Textur mit Dichtewerten. Diese 3D Textur wird dann von in den MarchingCubes.vert und weiter an den MarchingCubes.geom gereicht. Darin werden für jeden Punkt mit einer Lookup Table dreiecke erzeugt die anschließend mit dem MarchingCubes.frag auf ein Quadrat gerendert wird dass auf unseren Viewport gerendert wird.

## Particles 
Particles könne auf der GPU mit einem TransformFeedback Buffer realisiert werden. Dabei werden 2 Shaderprogramme erzeugt. Eines davon ist für Zustandsänderungen(Position, Geschwindigkeit, Typ) zuständig und gibt diese mittels TransformFeedback Buffer and ein weiteres Shaderprogramm, dass die Partikel rendert.

## Displacement Mapping
Je nach Winkel, mit dem auf eine Geometrie gesehen wird, werden die Texturkoordinaten versetzt so, dass teile der Textur verdeckt werden und der Eindruck von Tiefe entsteht.

## Soft Shadows 
Variance Shadow Mapping bestimmen mit einer gewissen Wahrscheinlichkeit ob etwas beleuchtet ist. Die Szene wird aus der Perspektive des Lichts in eine Tiefentextur gerendered. Für die Schatten werden die Tiefewerte der Textur mit der Tiefe der Objekte zur Lichtquelle verglichen und in Beleuchtungswahrscheinlichkeiten umgewandelt. Die Tiefentextur kann noch geblurred werden um weichere Übergänge zu erzielen. 

## Tesselation
Um Geometrie zu Tellelieren muss diese als Patch beim Renderaufruf deklariert werden. Eine TesselationControlShader definiert die Einstellungen und setzt Inner und Outer Tesselierungsfaktoren. Ein TesselationEvaluationShader setzt einzelne Vertecies wieder zu neuen zusammen und kann mithilfe von einer HeightMap die Vertecies in bevorzugten Richtungen erzeugen. 
