
// CREATE A 3D TEXTURE


// CREATE A FRAMEBUFFER OBJECT
// bind the texture to the framebuffer object

// glFramebufferTexture3D


// FOR EACH LAYER
// RENDER A 2D SLICE OF THE 3D TEXTURE
// BY PASSING A FULLSCREEN QUAD (2 TRIANGLES)

//for(int i = 0; int i < layers)
// {
// glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, depthTexture, 0, i)
// glDrawElements(GL_TRIANGLES, shadowCaster.triangleCount(), GL_UNSIGNED_INT, (void*)0)
// }


// INSIDE THE VERTEX SHADER
// ------------------------

// FILL THE LAYER WITH ACTUAL VALUES

// SET THE VALUES INSIDE THE FRAGMENT SHADER
// 