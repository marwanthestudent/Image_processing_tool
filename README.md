# Image_processing_tool
A multi-threaded image processing application that does sharpening and edge detection on a PNG using highpass and Sobel filtering. Can be run using 1,2,4,8,16,32 threads for more efficient processing. It creates a copy of the original with the filters applied and **DOES NOT** alter the original image.

## Features

- **Sharpen**: Sharpen outlines in a PNG.
- **Edge detect**: Finds the boundaries of a PNG.
- **Multi-Threaded**: Use more threads for faster results (depending on your CPU).

## Installation

1. **Clone the repository**:
    ```bash
    git clone https://github.com/marwanthestudent/Image_processing_tool.git
    ```

2. **Install dependencies**:
   Ensure you have C installed with the required libraries (libpng, zlib):<br />
   For more info -> http://www.libpng.org/pub/png/libpng.html<br />
   If you have a package manager like homebrew or pacman (whatever your OS and manager is)
    ```bash
    homebrew install libpng
    ```

4. **Compilation path**:
   Ensure you have the directory libpng16 in your include path so the header file can be recognized (`include <png.h>`).
   ```bash
    -I<whatever location the include directory is in>
   ```
   Check Makefile for more comments.

6. **Linker path**:
   Ensure that the linker has the path to require files
   ```bash
    -L<whatever location libpng is in> -lpng16 -lz
   ```
   Check Makefile for more comments.
## Usage

1. **Running the executable**:
    ```bash
    ./image_process <relative path of image> <relative path and name desired of output> <edge, sharpen> <No of threads: 1,2,4,8,16,32 >
    ```
    
## Example Output

An example of how the output would look using the example imgs folder:

Original PNG             |   Output PNG after edge detection
:-------------------------:|:-------------------------:
![](https://github.com/user-attachments/assets/dc907be0-bfb0-4bf8-8133-6e9394091a4d) | ![](https://github.com/user-attachments/assets/0dc1d0ca-f50f-409c-82b2-70a14955d818)


Original PNG             |   Output PNG after sharpening
:-------------------------:|:-------------------------:
![](https://github.com/user-attachments/assets/32c08b27-1bba-4831-854c-20a4bf904cc5) | ![](https://github.com/user-attachments/assets/2ec89596-fee2-4e24-b55c-f878585e41a7)

## Additional Notes
There is a TROFF manual I made in the manual directory so the program can feel more like a command (mainly for learning purposes)

Screenshot from terminal  |
:-------------------------:
![](https://github.com/user-attachments/assets/95bdf316-1074-4581-a028-9b66a384dea1)

To display it, move image_process.1 to your manual's folder (in super user), for me it was 
```bash
sudo mv image_process.1 /usr/Local/share/man/man1
```

Then from the terminal you can display the manual by simply
```bash
man man Image_process
```

## Future Work

- Implement Gaussian blurring and Unsharp. I had trouble debugging so I did not include this feature in the current version.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
