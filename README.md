# Open-GL-Stargate-Project
Small experimentation with OpenGL on my free-time for my course INFOH502 at Uni.
Initially based on Glitter release and some code from learnopengl.com.

Application launches in windowed full-screen and should be at your monitor resolution.
Startup can take some time (more than a few tens of seconds on low-end specs) due to some high-poly models. Don't panic if screen stays black at startup, just take your time and enjoy the music :)

Gameplay:
- 6DOF camera with smooth controls, and "sprint"
- Control of the ship (pitch, yaw, roll and 6 translations).
- Outlining of the ship using stencil buffer to allow to see it through other objects.
- Launchable missile with 10 sec cooldown.
- Background music.

Environment:
- cubemap/skybox in space.
- Automatic stars procedural generation with python script, reading of the file in the c++ code and drawing of the stars with varying size far away in the sky.
	 The rendering is done using instancing to increase performance.
- Asteroid rendering using instancing as well.
- Blinn-Phong's light algorithm (ambient, diffuse, specular) and emission lighting.
- Use of emission maps for other light sources on the object itself (as well as diffuse and specular maps).
- Environment mapping: Reflection and refraction, using both reflection map on the ship and can be toggled as a whole on the planet.
- Light attenuation with distance (makes sense as we are in space: very long distance but not highly realistic).
- 3 different types of lightsources (point light, directional, spotlight/flashlight) with their own class
- Sunlight soft shadows using depth cubemaps, working on all objects

Objects:
- Jumper (spaceship) which can be moved, outlined and exploded (explosion effect using the normals and geometry shader).
- Stargate with its own shader for water ripple effect (sum of 4 circular wave and a spiral, moving with time)).
- Planet.
- Edison type lightbulb (Use of blending for transparency of a light bulb glass).
- Sun with its own shader for corona effect.
- "Weird" Cube with normal mapping.
- Missile (not by me) with my own particle effects.
- Asteroids (not by me).

Others:
- Use of a framebuffer to act as a secondary POV following the ship
- Use of kernels in the framebuffer for toggable post-processing effects.
- Use of face culling when relevant for a performance increase.
- Very basic implementation of MSAA (anti-aliasing).


keybindings (AZERTY keyboard):
Camera Movements:
	ZQSD - move Camera
	X, SPACE - move Camera vertically
	SHIFT - increase Camera Movement speed
	Mouse - orient Camera

Jumper (ship):
	T, G - move forward/backward
	H, F - move laterally
	Y, B - move vertically
	Arrows - orient Jumper (pitch & roll)
	+, - (numpad) - orient Jumper (Yaw)

	K - explode Jumper
	L - launch missile (10 sec cooldown)

	O - outline Jumper (can see it through other objects)

Technical Demo:
	I - Toggle planet as a crystal (refraction only)
	U - Toggle planet as chrome (reflection added to the output color)
	N - toggle MSAA
	V - toggle Normal Mapping on "weird cube"
	J - toggle framebuffer second POV

Post-processing:
	Numpad 4 - toggle Sharpening
	Numpad 5 - toggle Blur
	Numpad 6 - toggle edge detection
	Numpad 7 - toggle grayscale
	Numpad 8 - toggle shadows
	
Music: 
	P - pause Music

Miscellaneous:
	1,2,3 - Wireframe modes
	W - deactivate VSync and show FPS


