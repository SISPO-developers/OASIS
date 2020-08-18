#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <inttypes.h>
#include <xoshiro256plus.h>
#include <sys/timeb.h>

void printTime(int64_t millis){
    int hours = floor(millis/(1000*60*60));
    int minutes = floor(millis/(1000*60)) - hours*60;
    int seconds = floor(millis/1000) - minutes*60 - hours*60*60;
    int case_definition = 0;
    if(hours < 10){
        case_definition += 1;
    }
    if(minutes < 10){
        case_definition += 2;
    }
    if(seconds < 10){
        case_definition += 4;
    }
    switch (case_definition){
    case 0:
        printf("Render time: %d:%d:%d\n", hours, minutes, seconds);
        break;
    case 1:
        printf("Render time: 0%d:%d:%d\n", hours, minutes, seconds);
        break;
    case 2:
        printf("Render time: %d:0%d:%d\n", hours, minutes, seconds);
        break;
    case 3:
        printf("Render time: 0%d:0%d:%d\n", hours, minutes, seconds);
        break;
    case 4:
        printf("Render time: %d:%d:0%d\n", hours, minutes, seconds);
        break;
    case 5:
        printf("Render time: 0%d:%d:0%d\n", hours, minutes, seconds);
        break;
    case 6:
        printf("Render time: %d:0%d:0%d\n", hours, minutes, seconds);
        break;
    case 7:
        printf("Render time: 0%d:0%d:0%d\n", hours, minutes, seconds);
        break;
    default:
        break;
    }
}

inline double dotProduct(double* u, double* v){
    return u[0]*v[0] + u[1]*v[1];
}

inline double vectorLength(double* u){
    return sqrt(pow(u[0], 2) + pow(u[1], 2));
}

inline double randomNumber(){
    //return (rand()/(double)RAND_MAX);
    return next();
}

inline double randomGaussianDistribution(double mean, double std_dev){
    double rand1 = randomNumber();
    double rand2 = randomNumber();
    return sqrt(-2 * log(rand1)) * cos(2 * M_PI * rand2) * std_dev + mean;
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
        d = size * randomGaussianDistribution(0, 1);
        vec_out[0] = d*sin(orientation) + position[0];
        vec_out[1] = d*cos(orientation) + position[1];
        break;
    case 2:
        d = size * randomGaussianDistribution(0, 1);
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

inline double arrayValue(double* array , int x, int y, int ch, int width){
    return array[x*3 + y*width*3 + ch];
}

inline double aberrationSizeFromLens(double* position, double* lens, double lens_scale, double lens_offset, double strength, int width, int height, int lens_width, int lens_height){
    double x = 0;
    double y = 0;
    if (width > height){
        x = position[0] * lens_scale;
        y = position[1] * lens_scale + lens_offset;
    }
    else{
        x = position[0] * lens_scale + lens_offset;
        y = position[1] * lens_scale;
    }
    double amount = 0;
    for (int i = 0; i < 3; i++){
        amount += arrayValue(lens, (int)x, (int)y, i, lens_width);
    }
    amount = amount / 3;

    return amount * strength;
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

void printAberration(int aberration, int shotNoise, double strength){
    if(strength == 0){
        printf("Applying shot noise:   0 %%");
    }
    else{
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
}

double* generate(double* input, int samples, double exposure, int aberration, double strength, double darkCurrent, double readoutNoise, int shotNoise, int x_min, int x_max, int y_min, int y_max, double* lens, double lens_scale, double lens_offset, int lens_width, int lens_height, int width, int height, int mono){
    create(rand(), rand(), rand(), rand());
    double* output = generateImageArray(width, height, 3);
    if(strength != 0 || shotNoise == 1){
        struct timeb t_start, t_end;
        uint64_t t_elapsed;
        ftime(&t_start);
        printAberration(aberration, shotNoise, strength);
        double gain = exposure * (double) 1/(samples*3);
        int internal_coma = 0;
        if(strength < 0){
            internal_coma = 1;
        }
        strength = sqrt(fabs(strength)) * width/2048;
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
                    orientation = internal_coma*M_PI + aberrationOrientation(position, center);
                    if (lens_scale > 0){
                        size = aberrationSizeFromLens(position, lens, lens_scale, lens_offset, strength, width, height, lens_width, lens_height);
                    }
                    else{
                        size = aberrationSize(position, center, d_max, strength);
                    }
                    for(int i = 0; i < samples; i++){
                        psf(aberration, psf_pos, position, orientation, size);
                        applyLightRay(output, psf_pos, rgb, gain, width, height);
                    }
                }
            }
        }
        else{
            int maxSamples = 3*samples*(x_max-x_min)*(y_max-y_min);
            for(int p = 0; p < 100; p++){
                printf("\b\b\b\b\b%3d %%", (100*p/100+1) );
                for(int s = 0; s < maxSamples/100; s++){
                    position[0] = x_min + floor(randomNumber()*(x_max-x_min-1));
                    position[1] = y_min +  floor(randomNumber()*(y_max-y_min-1));
                    int rgb_ch = floor(randomNumber()*3);
                    rgb[0] = 0; rgb[1] = 0; rgb[2] = 0;
                    rgb[rgb_ch] = arrayValue(input, (int)position[0], (int)position[1], rgb_ch, width);
                    orientation = internal_coma*M_PI + aberrationOrientation(position, center);
                    if (lens_scale > 0){
                        size = aberrationSizeFromLens(position, lens, lens_scale, lens_offset, strength, width, height, lens_width, lens_height);
                    }
                    else{
                        size = aberrationSize(position, center, d_max, strength);
                    }
                    psf(aberration, psf_pos, position, orientation, size);
                    applyLightRay(output, psf_pos, rgb, gain, width, height);
                }
            }
        }
        printf("\n");
        ftime(&t_end);
        t_elapsed = (uint64_t) (1000.0 * (t_end.time - t_start.time) + (t_end.millitm - t_start.millitm));
        printTime(t_elapsed);
    }
    else{
        for(int x = x_min; x < x_max; x++){
            for(int y = y_min; y < y_max; y++){
                for(int i = 0; i < 3; i++){
                    output[x*height*3 + y*3 + i] = arrayValue(input, x, y, i, width);
                }
            }
        }
    }

    if(mono == 0){
        double amount = 0;
        if (darkCurrent > 0){
            for (int i = 0; i < width*height*3; i++){
                amount = fabs(randomGaussianDistribution(0, darkCurrent / 1000));
                output[i] += amount;
                if (output[i] > 1){
                    output[i] = 1;
                }
            }
            printf("Dark current noise applied.\n");
        }

        if (readoutNoise > 0){
            for (int i = 0; i < width*height*3; i++){
                amount = randomGaussianDistribution(0, readoutNoise / 1000);
                output[i] += amount;
                if (output[i] < 0){
                    output[i] = 0;
                }
                else if (output[i] > 1){
                    output[i] = 1;
                }
                
            }
            printf("Readout noise applied.\n");
        }
    }
    else{
        double amount = 0;
        if (darkCurrent > 0){
            for(int x = x_min; x < x_max; x++){
                for(int y = y_min; y < y_max; y++){
                    amount = fabs(randomGaussianDistribution(0, darkCurrent / 1000));
                    for(int i = 0; i < 3; i++){
                        output[x*height*3 + y*3 + i] += amount;
                        if (output[i] > 1){
                            output[i] = 1;
                        }
                    }
                }
            }
            printf("Dark current noise added.\n");
        }

        if (readoutNoise > 0){
            for(int x = x_min; x < x_max; x++){
                for(int y = y_min; y < y_max; y++){
                    amount = randomGaussianDistribution(0, readoutNoise / 1000);
                    for(int i = 0; i < 3; i++){
                        output[x*height*3 + y*3 + i] += amount;
                        if (output[i] < 0){
                            output[i] = 0;
                        }
                        else if (output[i] > 1){
                            output[i] = 1;
                        }
                    }
                }
            }
            printf("Readout noise added.\n");
        }
    }
    return output;
}