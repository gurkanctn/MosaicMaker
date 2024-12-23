# MosaicMaker
Photo Mosaic Maker generates hi-resolution image tiles using arbitrary images (supplied by the user in the './Photos' folder).

Example:
This example uses random photos from the internet.

Output (@Left) and Input (@Right):
![image](https://user-images.githubusercontent.com/8074474/165599969-79b18be9-ebbb-408c-9c06-fbf33d6db463.png)


And, here's a closer look (zoom in to analyze the tiles)
![output](https://user-images.githubusercontent.com/8074474/165600047-2c24fb45-ddb0-46f0-8021-5ebc53a1f9f8.png)

Uses olcPixelGameEngine to generate and display a photo mosaic.

# Usage
1. create a "photos" folder and copy several photos to be used in generating the Mosaic
2. run the program (it asks for the Target file name in the Terminal, so don't forget to switch to the terminal when you see the big Canvas)
3. hang on, while the computer analyses your input photos to generate a Mosaic of your target photo.

# Notes
* uses C++/17 standard (to search for filenames)
* comes with no Warranty
* You are supposed to utilize your own photos as this code repo does not contain photos (except for the Readme)
* photos can be any of {*.jpg, *.jpeg, *.png}

# Licenses and Dependencies
* olcPixelGameEngine: https://github.com/OneLoneCoder/olcPixelGameEngine
* stb_image_write: https://github.com/nothings/stb
* It uses the color distance function as described in https://en.wikipedia.org/wiki/Color_difference

# For Developers
* Mosaic Maker can be compiled and run on both Linux and Windows.
* Feel free to add features (User Interface, sharing the output, embedding a watermark, etc)


Enjoy! :)
