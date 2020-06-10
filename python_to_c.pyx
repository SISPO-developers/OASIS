import numpy as np
cimport numpy as cnp  #cython requires special numpy array


def parameters_to_array(image_arr, samples, exposure, aberration, strength, x_min, x_max, y_min, y_max, width, height):
    input_arr = []
    aberration_int = 0
    if aberration == 'coma':
        aberration_int = 0
    elif aberration == 'astigmatism_tangential':
        aberration_int = 1
    elif aberration == 'astigmatism_sagittal':
        aberration_int = 2
    input_arr.append(samples)
    input_arr.append(exposure)
    input_arr.append(aberration_int)
    input_arr.append(strength)
    input_arr.append(x_min)
    input_arr.append(x_max)
    input_arr.append(y_min)
    input_arr.append(y_max)
    input_arr.append(width)
    input_arr.append(height)
    for i in range(0, width*height):
        for j in range(0, 3):
            input_arr.append(image_arr[i][j])
    return input_arr

cdef extern from "generateAberration.h":
    double* generate(double* x)

cpdef pass_to_c(image_arr, samples, exposure, aberration, strength, x_min, x_max, y_min, y_max, width, height):
    input_data = parameters_to_array(image_arr, samples, exposure, aberration, strength, x_min, x_max, y_min, y_max, width, height)
    cdef cnp.ndarray[cnp.float64_t, ndim=1] input = np.array(input_data, dtype=np.float64)
    output_data = generate(<double*> input.data)
    output_img = []
    for x in range(0, width):
        col = []
        for y in range(0, height):
            pixel = []
            for ch in range(0, 3):
                pixel.append(output_data[x*height*3 + y*3 + ch])
            col.append(pixel)
        output_img.append(col)
    return output_img


