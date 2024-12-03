#include <stdlib.h>
#include <stdio.h>
#include <png.h>
#include <math.h>
#include <string.h>
#include "ReadWrite.h"

// Global variables to hold image data and metadata
int width, height, color, depth;
size_t rowbytes;
int channels;
png_bytep *rows = NULL;

void write_png_file(char *file_name) {
    FILE *fp = fopen(file_name, "wb");
    if (!fp) {
        fprintf(stderr, "Error writing file %s\n", file_name);
        exit(EXIT_FAILURE);
    }

    //separate PNG structures for writing
    png_structp write_struct = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!write_struct) {
        fprintf(stderr, "Failed to create PNG write struct\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    png_infop write_info = png_create_info_struct(write_struct);
    if (!write_info) {
        fprintf(stderr, "Failed to create PNG info struct\n");
        png_destroy_write_struct(&write_struct, NULL);
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    //yeah i don't even know just follow the standard?
    if (setjmp(png_jmpbuf(write_struct))) {
        fprintf(stderr, "Error during PNG write\n");
        png_destroy_write_struct(&write_struct, &write_info);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    png_init_io(write_struct, fp);

    // use updated depth and color variables NOT old ones
    png_set_IHDR(
        write_struct, write_info, width, height,
        depth, color, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT
    );

    png_write_info(write_struct, write_info);
    png_write_image(write_struct, rows);
    png_write_end(write_struct, NULL);


    png_destroy_write_struct(&write_struct, &write_info);

    fclose(fp);
}

void read_png_file(char *file_name) {
    char header[8];  //checking PNG signature

    FILE *fp = fopen(file_name, "rb");
    if (!fp) {
        fprintf(stderr, "Error opening file %s\n", file_name);
        exit(EXIT_FAILURE);
    }

    fread(header, 1, 8, fp);
    if (png_sig_cmp((png_const_bytep)header, 0, 8)) {
        fprintf(stderr, "%s is not a PNG file\n", file_name);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    png_structp img_struct = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!img_struct) {
        fprintf(stderr, "Failed to create PNG read struct\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    png_infop img_info = png_create_info_struct(img_struct);
    if (!img_info) {
        fprintf(stderr, "Failed to create PNG info struct\n");
        png_destroy_read_struct(&img_struct, NULL, NULL);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    if (setjmp(png_jmpbuf(img_struct))) {
        fprintf(stderr, "Error during PNG read initialization\n");
        png_destroy_read_struct(&img_struct, &img_info, NULL);
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    png_init_io(img_struct, fp);
    png_set_sig_bytes(img_struct, 8);

    png_read_info(img_struct, img_info);

    width = png_get_image_width(img_struct, img_info);
    height = png_get_image_height(img_struct, img_info);
    color = png_get_color_type(img_struct, img_info);
    depth = png_get_bit_depth(img_struct, img_info);

    //image metadata
    if (depth == 16) png_set_strip_16(img_struct);
    if (color == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(img_struct);
    if (color == PNG_COLOR_TYPE_GRAY && depth < 8) png_set_expand_gray_1_2_4_to_8(img_struct);
    if (png_get_valid(img_struct, img_info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(img_struct);
    if (color == PNG_COLOR_TYPE_GRAY || color == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(img_struct);

    //technically I don't have to have my if statements because
    //images will ALWAYS have 4 channels but it's nice for code
    //resuability
    if (!(color & PNG_COLOR_MASK_ALPHA))
        png_set_add_alpha(img_struct, 0xFF, PNG_FILLER_AFTER);

    png_read_update_info(img_struct, img_info);

    // update depth and color after transformations
    // TO SEE ACTUAL RESULTS MY GOD
    depth = png_get_bit_depth(img_struct, img_info);
    color = png_get_color_type(img_struct, img_info);


    rowbytes = png_get_rowbytes(img_struct, img_info);
    channels = png_get_channels(img_struct, img_info);

    rows = (png_bytep *)malloc(sizeof(png_bytep) * height);
    for (int i = 0; i < height; i++) {
        rows[i] = (png_byte *)malloc(rowbytes);
    }

    png_read_image(img_struct, rows);

    png_destroy_read_struct(&img_struct, &img_info, NULL);

    fclose(fp);
}
