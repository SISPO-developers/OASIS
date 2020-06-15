#include <stdio.h>
#include <math.h>


static unsigned int g_seed;

inline void fast_srand(int seed) {
    g_seed = seed;
}

inline int fast_rand(void) {
    g_seed = (214013*g_seed+2531011);
    return (g_seed>>16)&0x7FFF;
}


int samples = 1000;
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

inline double dotProduct(double* u, double* v){
    return u[0]*v[0] + u[1]*v[1];
}

inline double vectorLength(double* u){
    return sqrt(pow(u[0], 2) + pow(u[1], 2));
}

inline double randomNumber(){
    //return (fast_rand()/(double)32767);
    return (rand()/(double)RAND_MAX);
}

inline double randomNormalDistribution(double min, double max){
    double rand1 = randomNumber();
    double rand2 = randomNumber();
    return sqrt(-2 * log(rand1)) * cos(2 * M_PI * rand2) * (max - min) + min;
}

inline void psf(int aberration, double* vec_out, double* position, double orientation, double size){
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

inline double aberrationSize(double* position, double* center){
    double vec[2] = {position[0] - center[0], position[1] - center[1]};
    double d = vectorLength(vec);
    return d/d_max * strength;
}

inline double aberrationOrientation(double* position, double* center){
    double vec[2] = {position[0] - center[0], position[1] - center[1]};
    double orientation = acos(dotProduct(reference, vec)/(vectorLength(reference)*vectorLength(vec)));
    if (position[0] < center[0]){
        orientation *= -1;
    }
    return orientation;
}

inline double arrayValue(double* array , int x, int y, int ch){
    return array[x*3 + y*width*3 + ch];
}

void printRGB(double* array, int x, int y){
    double r = arrayValue(array, x, y, 0);
    double g = arrayValue(array, x, y, 1);
    double b = arrayValue(array, x, y, 2);
    printf("R: %f G: %f B: %f \n", r, g, b);
}

void applyLightRay(double* image, double* psf_pos, 
                            double* rgb, 
                            double gain, int width, int height){
    int pixelX0 = floor(psf_pos[0]);
    int pixelY0 = floor(psf_pos[1]);
    //if(pixelX0+1>=width || pixelX0<0) return;
    //if(pixelY0+1>=height || pixelY0<0) return;

    for(int i = 0; i < 2; i++){
        int pixelX = pixelX0+i;
        if(pixelX > (width-1) || pixelX < 0) continue;
        for(int j = 0; j < 2; j++){
            int pixelY = pixelY0+j;
            if(pixelY > (height-1) || pixelY < 0) continue;
            double amount = 1 - fabs(psf_pos[0] - pixelX) * fabs(psf_pos[1] - pixelY);
            for(int k = 0; k < 3; k++){
                double ray = rgb[k] * amount * gain;
                image[pixelX*height*3 + pixelY*3 + k] += ray;
            }
        }
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
    x_min = arr[4];
    x_max = arr[5];
    y_min = arr[6];
    y_max = arr[7];
    width = arr[8];
    height = arr[9];
    strength = arr[3] * width/2000;
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
    //fast_srand(1);
    
    double* input = arrayToImage(input_data);
    double* output = generateImageArray(width, height, 3);
    double position[2] = {0 ,0};
    double orientation = 0;
    double size = 0;
    double psf_pos[2] = {0,0}; 
    printf("start \n");
    
    for(int x = x_min; x < x_max; x++){
        
        position[0] = x;
        //printf("progress: %d \n", x);
        for(int y = y_min; y < y_max; y++){
            for(int i = 0; i < 3; i++){
                rgb[i] = arrayValue(input, x, y, i);
            }
            position[1] = y;
            orientation = aberrationOrientation(position, center);
            size = aberrationSize(position, center);
            for(int i = 0; i < samples; i++){
                psf(aberration, psf_pos, position, orientation, size);
                applyLightRay(output, psf_pos, rgb, gain, width, height);
            }
        }
    }
    float offset = 0.5;
    float multiplier = 2;
    for(int i = 0; i < 3; i++){
        int x = center[0]-offset; int y = center[1]-offset;
        output[x*height*3 + y*3 + i] = arrayValue(input, x, y, i) * gain * samples * multiplier;

        x = center[0]-offset; y = center[1]+offset;
        output[x*height*3 + y*3 + i] = arrayValue(input, x, y, i) * gain * samples * multiplier;

        x = center[0]+offset; y = center[1]-offset;
        output[x*height*3 + y*3 + i] = arrayValue(input, x, y, i) * gain * samples * multiplier;

        x = center[0]+offset; y = center[1]+offset;
        output[x*height*3 + y*3 + i] = arrayValue(input, x, y, i) * gain * samples * multiplier;
    }
    printf("end \n");
    return output;
}