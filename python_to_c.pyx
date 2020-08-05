import numpy as np
cimport numpy as cnp  #cython requires special numpy array

cdef extern from "generateAberration.h":
    double* generate(double* x, int samples, double exposure, int aberration, double strength, double darkCurrent, double readoutNoise, int shotNoise, int x_min, int x_max, int y_min, int y_max, double* lens_file, double lens_scale, double lens_offset, int lens_width, int lens_height, int width, int height, int mono)

cpdef pass_to_c(image_arr, samples, exposure, aberration, strength, dark_current, readout_noise, shot_noise, x_min, x_max, y_min, y_max, lens_file, lens_scale, lens_offset, lens_width, lens_height, width, height, mono):
    aberration_int = 0
    if aberration == 'coma':
        aberration_int = 0
    elif aberration == 'astigmatism_tangential':
        aberration_int = 1
    elif aberration == 'astigmatism_sagittal':
        aberration_int = 2
    cdef cnp.ndarray[cnp.float64_t, ndim=1] input = np.array(image_arr, dtype=np.float64)
    cdef cnp.ndarray[cnp.float64_t, ndim=1] lens = np.array(lens_file, dtype=np.float64)
    output_data = generate(<double*> input.data, samples, exposure, aberration_int, strength, dark_current, readout_noise, shot_noise, x_min, x_max, y_min, y_max, <double*> lens.data, lens_scale, lens_offset, lens_width, lens_height, width, height, mono)
    del input
    del lens
    for x in range(0, width):
        for y in range(0, height):
            for ch in range(0, 3):
                image_arr[y*width*3 + x*3 + ch] = output_data[x*height*3 + y*3 + ch]
    return image_arr


