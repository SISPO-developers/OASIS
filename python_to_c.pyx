import numpy as np
cimport numpy as cnp  #cython requires special numpy array

cdef extern from "generateAberration.h":
    double* generate(double* x, int samples, double exposure, int aberration, double strength, double darkCurrent, double readoutNoise, int shotNoise, int x_min, int x_max, int y_min, int y_max, int width, int height)

cpdef pass_to_c(image_arr, samples, exposure, aberration, strength, dark_current, readout_noise, shot_noise, x_min, x_max, y_min, y_max, width, height):
    aberration_int = 0
    if aberration == 'coma':
        aberration_int = 0
    elif aberration == 'astigmatism_tangential':
        aberration_int = 1
    elif aberration == 'astigmatism_sagittal':
        aberration_int = 2
    cdef cnp.ndarray[cnp.float64_t, ndim=1] input = np.array(image_arr, dtype=np.float64)
    output_data = generate(<double*> input.data, samples, exposure, aberration_int, strength, dark_current, readout_noise, shot_noise, x_min, x_max, y_min, y_max, width, height)
    del input
    for x in range(0, width):
        for y in range(0, height):
            for ch in range(0, 3):
                image_arr[x*height*3 + y*3 + ch] = output_data[x*height*3 + y*3 + ch]
    return image_arr


