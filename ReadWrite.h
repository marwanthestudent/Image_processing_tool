#include <png.h>

extern int width;
extern int height;
extern int color;
extern int depth;
extern size_t rowbytes;
extern int channels;
extern png_bytep *rows;

void write_png_file(char *file_name);
void read_png_file(char *file_name);
