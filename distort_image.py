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