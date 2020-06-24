#include <stdio.h>
#include <math.h>

inline double dotProduct(double* u, double* v){
    return u[0]*v[0] + u[1]*v[1];
}

inline double vectorLength(double* u){
    return sqrt(pow(u[0], 2) + pow(u[1], 2));
}

inline double randomNumber(){
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

inline double aberrationSize(double* position, double* center, double d_max, double strength){
    double vec[2] = {position[0] - center[0], position[1] - center[1]};
    double d = vectorLength(vec);
    return d/d_max * strength;
}

inline double aberrationOrientation(double* position, double* center){
    double vec[2] = {position[0] - center[0], position[1] - center[1]};
    double reference[2] = {0, 1};
    double orientation = acos(dotProduct(reference, vec)/(vectorLength(reference)*vectorLength(vec)));
    if (position[0] < center[0]){
        orientation *= -1;
    }
    return orientation;
}

inline double arrayValue(double* array , int x, int y, int ch, int width){
    return array[x*3 + y*width*3 + ch];
}

void printRGB(double* array, int x, int y, int width){
    double r = arrayValue(array, x, y, 0, width);
    double g = arrayValue(array, x, y, 1, width);
    double b = arrayValue(array, x, y, 2, width);
    printf("R: %f G: %f B: %f \n", r, g, b);
}

void applyLightRay(double* image, double* psf_pos, double* rgb, double gain, int width, int height){
    int pixelX0 = floor(psf_pos[0]);
    int pixelY0 = floor(psf_pos[1]);
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

void printAberration(int aberration, int shotNoise){
    switch (aberration)
    {
    case 0:
        if (shotNoise == 0){
            printf("Applying coma:   0 %%");
        }
        else{
            printf("Applying coma with shot noise:   0 %%");
        }
        break;
    case 1:
        if (shotNoise == 0){
            printf("Applying tangential astigmatism:   0 %%");
        }
        else{
            printf("Applying tangential astigmatism with shot noise:   0 %%");
        }
        break;
    case 2:
        if (shotNoise == 0){
            printf("Applying sagittal astigmatism:   0 %%");
        }
        else{
            printf("Applying sagittal astigmatism with shot noise:   0 %%");
        }
        break;
    default:
        printf("Render progress:   0 %%");
        break;
    };
}

double* generate(double* input, int samples, double exposure, int aberration, double strength, double darkCurrent, double readoutNoise, int shotNoise, int x_min, int x_max, int y_min, int y_max, int width, int height){
    printAberration(aberration, shotNoise);
    double gain = exposure/(samples*10);
    strength = strength * width/2000;
    double* output = generateImageArray(width, height, 3);
    double position[2] = {0 ,0};
    double orientation = 0;
    double size = 0;
    double psf_pos[2] = {0,0};
    double rgb[3] = {0, 0, 0};
    double center[2] = {floor((width + 1) / 2) + 0.5, floor((height + 1) / 2) + 0.5};
    double d_max = vectorLength(center);
    
    if (shotNoise == 0){
        for(int x = x_min; x < x_max; x++){
            position[0] = x;
            printf("\b\b\b\b\b%3d %%", (100*(x-x_min)/(x_max-x_min)+1) );
            for(int y = y_min; y < y_max; y++){
                for(int i = 0; i < 3; i++){
                    rgb[i] = arrayValue(input, x, y, i, width);
                }
                position[1] = y;
                orientation = aberrationOrientation(position, center);
                size = aberrationSize(position, center, d_max, strength);
                for(int i = 0; i < samples; i++){
                    psf(aberration, psf_pos, position, orientation, size);
                    applyLightRay(output, psf_pos, rgb, gain, width, height);
                }
            }
        }
    }
    else{
        int maxSamples = samples*width*height;
        for(int p = 0; p < 100; p++){
            printf("\b\b\b\b\b%3d %%", (100*p/100+1) );
            for(int s = 0; s < maxSamples/100; s++){
                position[0] = floor(randomNumber()*(width-1));
                position[1] = floor(randomNumber()*(height-1));
                for(int i = 0; i < 3; i++){
                    rgb[i] = arrayValue(input, (int)position[0], (int)position[1], i, width);
                }
                orientation = aberrationOrientation(position, center);
                size = aberrationSize(position, center, d_max, strength);
                psf(aberration, psf_pos, position, orientation, size);
                applyLightRay(output, psf_pos, rgb, gain, width, height);
            }
        }
    }
    
    printf("\n");
    float offset = 0.5;
    float multiplier = 2;
    for(int i = 0; i < 3; i++){
        int x = center[0]-offset; int y = center[1]-offset;
        output[x*height*3 + y*3 + i] = arrayValue(input, x, y, i, width) * gain * samples * multiplier;

        x = center[0]-offset; y = center[1]+offset;
        output[x*height*3 + y*3 + i] = arrayValue(input, x, y, i, width) * gain * samples * multiplier;

        x = center[0]+offset; y = center[1]-offset;
        output[x*height*3 + y*3 + i] = arrayValue(input, x, y, i, width) * gain * samples * multiplier;

        x = center[0]+offset; y = center[1]+offset;
        output[x*height*3 + y*3 + i] = arrayValue(input, x, y, i, width) * gain * samples * multiplier;
    }

    double amount = 0;
    if (darkCurrent > 0){
        for (int i = 0; i < width*height*3; i++){
            amount = randomNormalDistribution(-1*darkCurrent / 1000, darkCurrent / 1000);
            if (amount > 0){
                output[i] += amount;
            }
        }
        printf("Dark current noise added.\n");
    }

    if (readoutNoise > 0){
        for (int i = 0; i < width*height*3; i++){
            amount = randomNormalDistribution(-1*readoutNoise / 1000, readoutNoise / 1000);
            output[i] += amount;
            if (output[i] < 0){
                output[i] = 0;
            }
        }
        printf("Readout noise added.\n");
    }

    return output;
}