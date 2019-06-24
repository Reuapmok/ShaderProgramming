#version 430

layout(points) in;
layout(points) out;
layout(max_vertices = 30) out;

// ATOMIC COUNTER
layout(binding = 4, offset = 0) uniform atomic_uint ac;

// READ IN  
in float Type0[];
in vec3 Position0[];
in vec3 Velocity0[];
in float Age0[];

// WRITE OUT
out float Type1;
out vec3 Position1;
out vec3 Velocity1;
out float Age1;

uniform float elapsedTime;
uniform float wholeTime;

// PARTICLE TYPE
#define PARTICLE_TYPE_SPAWNER	1.0f
#define PARTICLE_TYPE_NORMAL	0.0f
#define PARTICLE_TYPE_SNOW		2.0f
#define PARTICLE_TYPE_STEAM		3.0f


// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) 
{
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }


// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }




void main()
{
	// Update Age
    float Age = Age0[0] - elapsedTime;

	uint counter = atomicCounter(ac);

	// PARTICLE LAUNCHER 
	//
    if (Type0[0] == PARTICLE_TYPE_SPAWNER) 
	{	
		// ALIVE?
        if (Age <= 0.0f) 
		{
			// SPAWN NEW NORMAL PARTICLE
            Type1 = PARTICLE_TYPE_NORMAL;
            Position1 = Position0[0];
			Velocity1.y =  (random(elapsedTime) * 3.0f) + 2.0f;
			Velocity1.x = (random(wholeTime * elapsedTime + Position0[0].y)*2) -1;
			Velocity1.z = (random(wholeTime / elapsedTime - Position0[0].y)*2) -1;
            Age1 = (random(wholeTime) * 0.5) + 0.6f;//((noise1(elapsedTime)* 0.5f) + 0.5f); //5.0f;  Value between 1 and 5
			
			gl_Position = vec4(Position0[0], 1.0f);
            EmitVertex();
            EndPrimitive();
			
			Age = (random(vec2(wholeTime, elapsedTime)) * 0.01);//3.0f;
		}

		// UPDATE SPAWNER PARTICLE
		//
		atomicCounterIncrement(ac);
		Type1 = PARTICLE_TYPE_SPAWNER;
		Position1 = Position0[0];
		Velocity1 = Velocity0[0];
		Age1 = Age;

		gl_Position = vec4(Position0[0], 1.0f);
		EmitVertex();
		EndPrimitive();
	}
	else
	{
		// NORMAL PARTICLES
		//

		vec3 DeltaP = elapsedTime * Velocity0[0];
		// v = a * t		t			*				g
		vec3 DeltaV = vec3(elapsedTime) * vec3(9.0f, 9.81f, 9.0f); 

		// IF NORMAL PARTICLE
		if(Type0[0] == PARTICLE_TYPE_NORMAL)
		{
			// ALIVE?
			if(Age > 0)
			{	
				// UPDATE PARTICLE
				atomicCounterIncrement(ac);
				Type1 = Type0[0];
				Position1 = Position0[0] + DeltaP;
				Velocity1 = Velocity0[0] + DeltaV;
				Age1 = Age;

				gl_Position = vec4(Position0[0], 1.0f);
				EmitVertex();
				EndPrimitive();
			}
			else
			{
				// CHANGE TO SNOW
				atomicCounterIncrement(ac);
				Type1 = PARTICLE_TYPE_SNOW;
				Position1 = Position0[0] + DeltaP;
				Velocity1 = vec3(0.0f,-1.3f,0.0f);
				Age1 = 2.0f;

				gl_Position = vec4(Position0[0], 1.0f);
				EmitVertex();
				EndPrimitive();
			}
		}
		else if(Type0[0] == PARTICLE_TYPE_SNOW)
		{
		// IF SNOW PARTICLE
			// ALIVE?
			vec3 DeltaV = vec3(elapsedTime) * vec3(4.0f, -4.81f, 4.0f); 
			if(Age > 0)
			{
				atomicCounterIncrement(ac);
				Type1 = Type0[0];
				Position1 = Position0[0] + DeltaP;
				Velocity1 = Velocity0[0] + DeltaV;
				Age1 = Age;

				gl_Position = vec4(Position0[0], 1.0f);
				EmitVertex();
				EndPrimitive();
			}
		}
	}

} 