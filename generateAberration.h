#include <stdio.h>
#include <math.h>

int samples = 1;
double exposure = 1;
double gain = 1;
int aberration = 0; // coma = 0, astig_tan = 1, asti_sag = 2
double strength = 1;
int x_min = 0;
int x_max = 1;
int y_min = 0;
int y_max = 1;
int width = 1;
int height = 1;
double center[2] = {0.5,0.5};
double d_max = 1;
double pixelList[5][3]; // p[0][0] = number of valid pixels. p[0<][:] = 1 to 4 pixels.
double reference[2] = {0, 1};
double rgb[3] = {0, 0, 0};

double dotProduct(double* u, double* v){
    return u[0]*v[0] + u[1]*v[1];
}

double vectorLength(double* u){
    return sqrt(pow(u[0], 2) + pow(u[1], 2));
}

double randomNumber(){
    return (rand()/(double)RAND_MAX);
}

double randomNormalDistribution(double min, double max){
    double rand1 = randomNumber();
    double rand2 = randomNumber();
    return sqrt(-2 * log(rand1)) * cos(2 * M_PI * rand2) * (max - min) + min;
}

void psf(double* vec_out, double* position, double orientation, double size){
    double d = 0;
    switch (aberration)
    {
    case 0:
        d = pow(size*randomNumber(), 2);
        double r = d/2;
        double centerPos[2] = {d*sin(orientation) + position[0], d*cos(orientation) + position[1]};
        double angle = randomNumber() * 2*M_PI;
        double localPos[2] = {r*sin(angle), r*cos(angle)};
        vec_out[0] = centerPos[0] + localPos[0];
        vec_out[1] = centerPos[1] + localPos[1];
        break;
    case 1:
        orientation += M_PI_2;
        d = size * randomNormalDistribution(-1, 1);
        vec_out[0] = d*sin(orientation) + position[0];
        vec_out[1] = d*cos(orientation) + position[1];
        break;
    case 2:
        d = size * randomNormalDistribution(-1, 1);
        vec_out[0] = d*sin(orientation) + position[0];
        vec_out[1] = d*cos(orientation) + position[1];
        break;
    default:
        break;
    };
}

double aberrationSize(double* position){
    double vec[2] = {position[0] - center[0], position[1] - center[1]};
    double d = vectorLength(vec);
    return d/d_max * strength;
}

double aberrationOrientation(double* position){
    double vec[2] = {position[0] - center[0], position[1] - center[1]};
    double orientation = acos(dotProduct(reference, vec)/(vectorLength(reference)*vectorLength(vec)));
    if (position[0] < center[0]){
        orientation *= -1;
    }
    return orientation;
}

void nearbyPixels(double* position){
    int pixels = 0;
    double pixel[2] = {0, 0};
    pixel[0] = floor(position[0]);
    pixel[1] = floor(position[1]);
    if (pixel[0] >= 0 && pixel[0] < width && pixel[1] >= 0 && pixel[1] < height){
        pixels++;
        double amount = 1 - fabs(position[0] - pixel[0]) * fabs(position[1] - pixel[1]);
        pixelList[pixels][0] = pixel[0];
        pixelList[pixels][1] = pixel[1];
        pixelList[pixels][2] = amount;
    }
    pixel[0] = floor(position[0]);
    pixel[1] = ceil(position[1]);
    if (pixel[0] >= 0 && pixel[0] < width && pixel[1] >= 0 && pixel[1] < height){
        pixels++;
        double amount = 1 - fabs(position[0] - pixel[0]) * fabs(position[1] - pixel[1]);
        pixelList[pixels][0] = pixel[0];
        pixelList[pixels][1] = pixel[1];
        pixelList[pixels][2] = amount;
    }
    pixel[0] = ceil(position[0]);
    pixel[1] = floor(position[1]);
    if (pixel[0] >= 0 && pixel[0] < width && pixel[1] >= 0 && pixel[1] < height){
        pixels++;
        double amount = 1 - fabs(position[0] - pixel[0]) * fabs(position[1] - pixel[1]);
        pixelList[pixels][0] = pixel[0];
        pixelList[pixels][1] = pixel[1];
        pixelList[pixels][2] = amount;
    }
    pixel[0] = ceil(position[0]);
    pixel[1] = ceil(position[1]);
    if (pixel[0] >= 0 && pixel[0] < width && pixel[1] >= 0 && pixel[1] < height){
        pixels++;
        double amount = 1 - fabs(position[0] - pixel[0]) * fabs(position[1] - pixel[1]);
        pixelList[pixels][0] = pixel[0];
        pixelList[pixels][1] = pixel[1];
        pixelList[pixels][2] = amount;
    }
    pixelList[0][0] = pixels;
}

double arrayValue(double* array , int x, int y, int ch){
    return array[x*3 + y*width*3 + ch];
}

void printRGB(double* array, int x, int y){
    double r = arrayValue(array, x, y, 0);
    double g = arrayValue(array, x, y, 1);
    double b = arrayValue(array, x, y, 2);
    printf("R: %f G: %f B: %f \n", r, g, b);
}

void applyLightRay(double* image, int x, int y, double* rgb, double amount){
    for(int i = 0; i < 3; i++){
        double ray = rgb[i] * amount * gain;
        //printf("rgb: %f gain: %f amount: %f ray: %f \n", rgb[i], gain, amount, ray);
        image[x*height*3 + y*3 + i] += ray;
    }
}

double* generateImageArray(int width, int height, int channels){
    double* img_array = calloc(width*height*channels, sizeof(double));
    return img_array;
}

double* arrayToImage(double* arr){
    int input_parameters = 10;
    samples = arr[0];
    exposure = arr[1];
    gain = exposure/(samples*1000);
    aberration = arr[2];
    strength = arr[3];
    x_min = arr[4];
    x_max = arr[5];
    y_min = arr[6];
    y_max = arr[7];
    width = arr[8];
    height = arr[9];
    center[0] = floor((width + 1) / 2) + 0.5;
    center[1] = floor((height + 1) / 2) + 0.5;
    d_max = vectorLength(center);
    double* input_image = generateImageArray(width, height, 3);
    for(int i = 0; i < width*height*3; i++){
        input_image[i] = arr[i + input_parameters];
    }
    return input_image;
}

double* generate(double* input_data){
    double* input = arrayToImage(input_data);
    double* output = generateImageArray(2304, 1728, 3);
    double position[2] = {0 ,0};
    double orientation = 0;
    double size = 0;
    double psf_pos[2] = {0,0}; 
    printf("start \n");
    for(int y = y_min; y < y_max; y++){
        position[1] = y;
        //printf("progress: %d \n", x);
        for(int x = x_min; x < x_max; x++){
            for(int i = 0; i < 3; i++){
                rgb[i] = arrayValue(input, x, y, i);
            }
            position[0] = x;
            orientation = aberrationOrientation(position);
            size = aberrationSize(position);
            for(int i = 0; i < samples; i++){
                psf_pos[0] = 0.0; psf_pos[0] = 0.0;
                psf(psf_pos, position, orientation, size);
                nearbyPixels(psf_pos);
                for(int j = 0; j < pixelList[0][0]; j++){
                    applyLightRay(output, pixelList[j+1][0], pixelList[j+1][1], rgb, pixelList[j+1][2]);
                }
            }
        }
    }
    printf("end \n");
    return output;
}
