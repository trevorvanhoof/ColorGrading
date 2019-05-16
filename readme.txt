Color grading utility with Qt5 and OpenGL
--
After searching for a bit I found it hard to find good color grading implementation references, that would talk about professional color grading tools.
The goal of this project is to get a basic Qt UI that ends up feeling like other color grading software, and to implement the response
to that UI in a post processing effect. In this case I just load an image and grade that.

I've learned the most from these 2 sources:
http://filmicworlds.com/blog/minimal-color-grading-tools/
https://www.bhphotovideo.com/explora/video/tips-and-solutions/introduction-color-grading

All the grading math happens in grading.glsl
Relevant code is in main.cpp
The rest is all (OpenGL) utilities.
--
Controls:
1. Primaries wheels
I attempted to implement primaries wheels from DaVinci Resolve, I know for a fact I didn't succeed after trying out that software,
but it's a step in the right direction and I hope to update this as I learn more on the subject.

You can drag anywhere on the wheel to move the 2D slider, dragging to the top, right adds more red, blue respectively.
Hold ALT to control the Y value, which acts as an offset to RGB.
Hold CTRL to control the 'white' value, which fades RGB to white. It can also go negative and will fade to -1.
Hold SHIFT for faster control, the 2D slider will snap to the mouse, the other controls will drag 10 times faster.

2. Sliders
Click and drag to set the slider at the mouse cursor.

The temperature slider is a bit 'experimental', it shows the gradient up until the current temperature.
However, temperature is used for white balance, so setthing the temperature slider to a yellow value, makes that the new 'white point',
shifting the entire image to colder tones.
--
Details:
In order of shader implementation...

Unsharp mask:
Whnd rawing a pixel, take 4 samples of the neighbouring pixels.
Average them and get the difference between this 'blurry' version and the original version.
Simply offset the original value by the delta:
vec3 blurry = 0.25 * (left + right + top + bottom)
color += (color - blurry) * uUnsharpMask;

Contrast: 
I had a hard time simulating DaVinci Resolve but believe to have a formula that is at least accurate.
Contrast has 2 parts: 
a 'pivot' brightness value that is set to be the new medium grey value.
and the actual 'contrast' value that will change the response in a sort of ease-in and ease-out way.

Below 1 is easy, just fade towards the pivot (where pivot is the point that will remain constant)
Above 1 was harder, I solved it by splitting the curve in 2 (around the pivot) and creating a smooth
falloff from there using pow(color, 1 / contrast).

Saturation is separate from the hue shift code, because I wanted to get a proper luminance value
which a simple rgb2hsv conversion does not cover.
I compute greyscale with:
dot(color, vec3(0.2126, 0.7152, 0.0722))
and mix away from that value by the saturation amount.
So 0 means luminance, 1 means original color, 2 means oversaturate.

Hue shift is done by converting to HSV and offsetting the hue
color = hsv2rgb(rgb2hsv(color) + vec3(fract(uHue / 6.0), 0.0, 0.0));

The last slider to implement is the white balance temperature.
Commented out in the shader is an attempt to actually tint the color with the temperature, but I wasn't sure where to go with that.
So I used it for white balance instead. I use the temperature to create a color from kelvin and simply offset all colors
based no the difference between pure white and that new 'white point'.
color *= vec3(1.0) / colorFromKelvin(uTemperature);
In the future we may just forward our own RGB white point for more control.

The primaries wheels control Y,R,B and white values directly.
The white value is shown on the outer ring, the R and B values are used to position the central dot
and the Y value is invisible for reasons unknown.

The values start at -1, but > 0 they get scaled to respond more conveniently.
Finally they can be pushed beyond the boundaries of the UI, as you can see if you keep dragging and look at
the YRGB labels at the bottom.

By trial and error in DaVinci Resolve I figured the actual RGB value used for the lift, gamma, gain & offset math
is computed by offsetting RGB with Y, and then mixing it all by the white value.

RGB = mix(RGB + Y, 1.0, white)
if white < 0 it will fade in the opposite direction, moving towards -1

The primaries are applied as follows:
color = pow(max(vec3(0.0), color * (1.0 + uGain - uLift) + uLift + uOffset), max(vec3(0.0), 1.0 - uGamma));
where the u-something values are RGB results from the previous formula for each color wheel.

Finally I convert to gamma space, commented out is some filmic tone mapping that I didn't want to pollute
our previewer, but when you do additional tone mapping I recommend you turn it on so it's clear what the
sum of all the changes will look like.
--
Known issues:
1. A lack of qmake!
Included is a Visual Studio 2019 project, targetting Windows 10.0, using Qt 5.
I generated this project using the Qt Visual Studio Tools extension.
It's output is vastly different from qmake, which I also tried but it just created broken projects.
The added bonus of the Qt Visual Studio Tools is that when running from withing visual studio it can always find all Qt binaries.
After building you'll still manually want to copy over the required DLLs next to your executable.
Current dependencies are:
Qt5Core.dll
Qt5Gui.dll
Qt5Widgets.dll
Qt5OpenGL.dll

2. Crash on exit
No idea why, the Qt OpenGL function loading object gets destroyed and somehow generates an exception. I just couldn't be bothered.
