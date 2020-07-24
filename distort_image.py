import numpy as np
import OpenEXR
import Imath
import array
import cv2
from PIL import Image

def distort(input, output, camera_matrix, distortion_coefficients):
    img = cv2.imread(input)
    dst = cv2.undistort(img, camera_matrix, distortion_coefficients)
    cv2.imwrite(output, dst)

def distort_matrix(width, height, img_matrix, camera_matrix, distortion_coefficients):
    img = np.zeros((height, width, 3))
    for x in range (0, width):
        for y in range (0, height):
            for ch in range (0, 3):
                img[y,x,ch] = img_matrix[y*width*3 + x*3 + ch]
    dst = cv2.undistort(img, camera_matrix, distortion_coefficients)
    for x in range (0, width):
        for y in range (0, height):
            for ch in range (0, 3):
                img_matrix[y*width*3 + x*3 + ch] = dst[y,x,ch]
    return img_matrix