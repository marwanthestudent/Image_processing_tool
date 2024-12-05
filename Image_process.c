//simplified code for the algorithms in python
//https://www.codingame.com/playgrounds/2524/basic-image-manipulation/filtering

#include <stdlib.h>
#include <stdio.h>
#include <png.h> //edit include path to where libpng16 lives
#include <math.h>
#include <string.h>
#include <pthread.h>
#include "ReadWrite.h"

// global variables are found in the header file



//struct to pass data to threads
typedef struct {
    int y_start;                // starting row index for the thread
    int y_end;                  // ending row index (exclusive) for the thread
    png_bytep *rows_target;     // Pointer to the target image rows
    float **intensity;          // pointer to the intensity array (edge detection)
} ThreadData;

//thread function for sharpening
void *sharpen_thread(void *arg) {
    ThreadData *data = (ThreadData *)arg;           // get the thread data
    int y_start = data->y_start;            
    int y_end = data->y_end;                       
    png_bytep *rows_target = data->rows_target;     

    //sharpening kernel
    float sharpen_kernel[3][3] = {
        {  0.0f, -0.5f,   0.0f },
        { -0.5f,  3.0f, -0.5f },
        {  0.0f, -0.5f,   0.0f }
    };
    int kernel_size = 3;
    int offset = kernel_size / 2;

    // convolution, here I discover the need to read a signal processing book
    // since my brain is barely handling it.
    for (int y = y_start; y < y_end; y++) {         // Process assigned rows
        for (int x = 0; x < width; x++) {
            float sum_r = 0.0f, sum_g = 0.0f, sum_b = 0.0f, sum_a = 0.0f;

            for (int ky = 0; ky < kernel_size; ky++) {
                for (int kx = 0; kx < kernel_size; kx++) {
                    int ny = y + ky - offset;
                    int nx = x + kx - offset;

                    // handle edges with mirroring, if the pixel is above or to the left of the image
                    // access the abs equivalent, if we're accessing (-1,-1) index when we apply
                    // the kernel at pixel 0 then just access (1,1)
                    ny = ny < 0 ? -ny : (ny >= height ? 2 * height - ny - 2 : ny);
                    nx = nx < 0 ? -nx : (nx >= width ? 2 * width - nx - 2 : nx);
                    // if it's to the right or below the img we force it to be inbounds.
                    // simple equation: 2*height - out_of_bound_height (at min is the height) - 2

                    png_bytep px = &(rows[ny][nx * channels]);
                    float kernel_value = sharpen_kernel[ky][kx];
                    sum_r += px[0] * kernel_value;
                    sum_g += px[1] * kernel_value;
                    sum_b += px[2] * kernel_value;
                    if (channels == 4)
                        sum_a += px[3] * kernel_value;
                }
            }

            // clamp RGB color code between 0 and 255.
            png_bytep target_px = &(rows_target[y][x * channels]);
            target_px[0] = (png_byte)(fminf(fmaxf(sum_r, 0.0f), 255.0f));
            target_px[1] = (png_byte)(fminf(fmaxf(sum_g, 0.0f), 255.0f));
            target_px[2] = (png_byte)(fminf(fmaxf(sum_b, 0.0f), 255.0f));
            if (channels == 4)
                target_px[3] = (png_byte)(fminf(fmaxf(sum_a, 0.0f), 255.0f));
        }
    }

    pthread_exit(NULL);  // Terminate the thread
}

//Thread function for edge detection
void *edge_detect_thread(void *arg) {
    ThreadData *data = (ThreadData *)arg;           // get the thread data
    int y_start = data->y_start;
    int y_end = data->y_end;          
    png_bytep *rows_target = data->rows_target;
    float **intensity = data->intensity;    

    // sobel operator kernels
    int gx_kernel[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    int gy_kernel[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };
    int kernel_size = 3;
    int offset = kernel_size / 2;

    for (int y = y_start; y < y_end; y++) {         // assigned rows
        for (int x = 0; x < width; x++) {
            float gx = 0.0f;
            float gy = 0.0f;

            for (int ky = 0; ky < kernel_size; ky++) {
                for (int kx = 0; kx < kernel_size; kx++) {
                    int ny = y + ky - offset;
                    int nx = x + kx - offset;

                    ny = ny < 0 ? -ny : (ny >= height ? 2 * height - ny - 2 : ny);
                    nx = nx < 0 ? -nx : (nx >= width ? 2 * width - nx - 2 : nx);
                    // we accumulate gradients instead of color channels here
                    float pixel_intensity = intensity[ny][nx];
                    gx += pixel_intensity * gx_kernel[ky][kx];
                    gy += pixel_intensity * gy_kernel[ky][kx];
                }
            }

            //gradient magnitude
            float gradient = sqrtf(gx * gx + gy * gy);

            //clamp to [0, 255]
            png_byte edge_value = (png_byte)(fminf(fmaxf(gradient, 0.0f), 255.0f));

            // Set the output pixel to the edge value (in grayscale)
            png_bytep target_px = &(rows_target[y][x * channels]);
            target_px[0] = edge_value;
            target_px[1] = edge_value;
            target_px[2] = edge_value;
            if (channels == 4) {
                target_px[3] = 255;  //alpha channel to fully opaque
            }
        }
    }

    pthread_exit(NULL);
}

void sharpen(int num_threads) {
    // sharpening kernel
    float sharpen_kernel[3][3] = {
        {  0.0f, -0.5f,   0.0f },
        { -0.5f,  3.0f, -0.5f },
        {  0.0f, -0.5f,   0.0f }
    };
    int kernel_size = 3;
    int offset = kernel_size / 2;

    //buffer
    png_bytep *rows_target = (png_bytep *)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
        rows_target[y] = (png_byte *)malloc(rowbytes);
        memset(rows_target[y], 0, rowbytes);
    }

    //threads
    pthread_t threads[num_threads]; //thread count
    ThreadData thread_data[num_threads];       // array to hold thread data

    int rows_per_thread = height / num_threads; //base number of rows per thread
    int remaining_rows = height % num_threads;  //remaining rows after division
    int y = 0;                                        // starting row

    for (int t = 0; t < num_threads; t++) {
        thread_data[t].y_start = y;                   // set starting row for this thread
        //let's say height 10, threads 3 (for demonstration)
        //3 rows each with 1 remaining, equaiton below if threadidx < remaining then add a thread
        // so thread 0 will get 4 threads while thread 2 and 3 will get 3
        int rows_for_this_thread = rows_per_thread + (t < remaining_rows ? 1 : 0);
        thread_data[t].y_end = y + rows_for_this_thread;  // Set ending row for this thread
        thread_data[t].rows_target = rows_target; 
        thread_data[t].intensity = NULL;              //not used in sharpening

        pthread_create(&threads[t], NULL, sharpen_thread, &thread_data[t]);

        y += rows_for_this_thread;                    // Update the start for the next thread
    }

    // joining threadpool
    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);   
    }

    //replace original rows
    for (int y = 0; y < height; y++) {
        free(rows[y]);
        rows[y] = rows_target[y];
    }
    free(rows_target);
}

void edge_detect(int num_threads) {

    png_bytep *rows_target = (png_bytep *)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
        rows_target[y] = (png_byte *)malloc(rowbytes);
        memcpy(rows_target[y], rows[y], rowbytes);
    }

    float **intensity = (float **)malloc(sizeof(float *) * height);
    for (int y = 0; y < height; y++) {
        intensity[y] = (float *)malloc(sizeof(float) * width);
        for (int x = 0; x < width; x++) {
            png_bytep px = &(rows[y][x * channels]);
            float r = px[0];
            float g = px[1];
            float b = px[2];
            // convert to grayscale using luminosity method
            intensity[y][x] = 0.299f * r + 0.587f * g + 0.114f * b;
        }
    }


    pthread_t threads[num_threads];// threads
    ThreadData thread_data[num_threads];              //thread data

    int rows_per_thread = height / num_threads;       //number of rows per thread
    int remaining_rows = height % num_threads;  //remaining rows see, explanation in sharpen
    int y = 0;                                        // starting row 

    for (int t = 0; t < num_threads; t++) {
        thread_data[t].y_start = y;                  
        int rows_for_this_thread = rows_per_thread + (t < remaining_rows ? 1 : 0);
        thread_data[t].y_end = y + rows_for_this_thread; 
        thread_data[t].rows_target = rows_target;     
        thread_data[t].intensity = intensity;        

        pthread_create(&threads[t], NULL, edge_detect_thread, &thread_data[t]);  

        y += rows_for_this_thread;                    
    }


    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);               // join threadpool
    }

    //replace original rows 
    for (int y = 0; y < height; y++) {
        free(rows[y]);
        free(intensity[y]);
        rows[y] = rows_target[y];
    }
    free(rows_target);
    free(intensity);
}

int main(int argc, char *argv[]) {

    if (argc != 5) {
        fprintf(stderr, "Usage: %s <input_png> <output_png> <effect> <num_threads>\n", argv[0]);
        fprintf(stderr, "Effect options: edge, sharpen\n");
        exit(EXIT_FAILURE);
    }

    int num_threads = atoi(argv[4]);
    if (!(num_threads == 1 || num_threads == 2 || num_threads == 4 || num_threads == 8 ||
        num_threads == 16 || num_threads == 32)) {
        fprintf(stderr, "Number of threads must be 1, 2, 8, 16, or 32.\n");
        exit(EXIT_FAILURE);
    }

    read_png_file(argv[1]);

    if (!strcmp(argv[3], "edge")) {
        edge_detect(num_threads); 
    } else if (!strcmp(argv[3], "sharpen")) {
        sharpen(num_threads);      
    } else {
        fprintf(stderr, "Invalid effect. Choose 'edge' or 'sharpen'.\n");
        exit(EXIT_FAILURE);
    }

    write_png_file(argv[2]);

    for (int y = 0; y < height; y++) {
        free(rows[y]);
    }
    free(rows);

    return 0;
}