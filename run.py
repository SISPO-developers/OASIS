import python_to_c
import distort_image
import numpy as np
import OpenEXR
import Imath
import array
import cv2
import scipy.ndimage.interpolation
from PIL import Image

class OpticalAberration:
    input = ''
    output = ''
    samples = 1
    exposure = 1
    strength = 1
    aberration = ''
    x_min = 0
    x_max = 1
    y_min = 0
    y_max = 1
    width = 1
    height = 1
    chromatic_aberration = 0
    dark_current = 1
    readout_noise = 1

    def __init__(self):
        pass

    @classmethod
    def generate_aberration(cls):
        input = cls.read_image(cls.input)
        if cls.chromatic_aberration > 0:
            cls.generate_chromatic_aberration(input, cls.chromatic_aberration)
        output_img = python_to_c.pass_to_c(input, cls.samples, cls.exposure, cls.aberration, cls.strength, cls.dark_current, cls.readout_noise, cls.x_min, cls.x_max, cls.y_min, cls.y_max, cls.width, cls.height)
        cls.write_image(output_img, cls.output)

    @classmethod
    def get_camera_matrix(cls, fx, fy, cx, cy):
        cm = np.zeros((3, 3))
        cm[0][0] = fx
        cm[1][1] = fy
        cm[0][2] = cx
        cm[1][2] = cy
        cm[2][2] = 1
        return cm

    @classmethod
    def get_dist_coeff(cls, k1, k2, p1, p2, k3):
        dist_invert = -1
        dc = np.zeros(5)
        dc[0] = k1 * dist_invert
        dc[1] = k2 * dist_invert
        dc[2] = p1 * dist_invert
        dc[3] = p2 * dist_invert
        dc[4] = k3 * dist_invert
        return dc

    @classmethod
    def set_parameters(cls, in_file, out_file, samples, exp, aberration, strength, chrom_ab, dark_current, readout_noise, x_min, x_max, y_min, y_max):
        #cls.read_image(in_file)
        cls.input = in_file
        cls.output = out_file
        cls.samples = samples
        cls.exposure = exp
        cls.aberration = aberration
        cls.strength = strength
        cls.chromatic_aberration = chrom_ab
        cls.x_min = x_min
        cls.x_max = x_max
        cls.y_min = y_min
        cls.y_max = y_max
        cls.dark_current = dark_current
        cls.readout_noise = readout_noise

    @classmethod
    def read_image(cls, file):
        input = []
        exr_in = 0
        img_type = file.split('.')[1]
        if img_type == 'jpg' or img_type == 'jpeg' or img_type == 'png':
            exr_in = 0
            img = Image.open(file)
            input = list(img.getdata())
            cls.width, cls.height = img.size
        elif img_type == 'exr':
            exr_in = 1
            exrFile = OpenEXR.InputFile(file)
            header = exrFile.header()
            dw = header['dataWindow']
            pt = Imath.PixelType(Imath.PixelType.FLOAT)
            cls.width, cls.height = (dw.max.x - dw.min.x + 1, dw.max.y - dw.min.y + 1)
            cc_r = np.fromstring(exrFile.channel('R', pt), dtype=np.float32)
            cc_g = np.fromstring(exrFile.channel('G', pt), dtype=np.float32)
            cc_b = np.fromstring(exrFile.channel('B', pt), dtype=np.float32)
            cc_r.shape = cc_g.shape = cc_b.shape = (cls.width, cls.height)
            cc = np.dstack((cc_r, cc_g, cc_b))
            input = [[0, 0, 0] for i in range(cls.width * cls.height)]
            for j in range(0, cls.width):
                for i in range(0, cls.height):
                    input[j*cls.width + i] = cc[j][i]
            del cc
            del cc_r
            del cc_g
            del cc_b
        input_1d = [0 for i in range(cls.width * cls.height * 3)]
        for i in range(0, cls.width):
            for j in range(0, cls.height):
                for k in range(0, 3):
                    if exr_in == 0:
                        input_1d[i*cls.height*3 + j*3 + k] = cls.sRGB_to_linear(input[i*cls.height + j][k])
                    else:
                        input_1d[i*cls.height*3 + j*3 + k] = input[i*cls.height + j][k]
        del input[:]
        del input
        if cls.x_max - cls.x_min < 1 or cls.y_max - cls.y_min < 1:
            cls.x_min = 0
            cls.x_max = cls.width
            cls.y_min = 0
            cls.y_max = cls.height
        return input_1d

    @classmethod
    def sRGB_to_linear(cls, sRGB):
        sRGB = sRGB / 255
        if sRGB <= 0.04045:
            return sRGB / 12.92
        else:
            return ((sRGB + 0.055) / 1.055) ** 2.4

    @classmethod
    def linear_to_sRGB(cls, lin):
        sRGB = 0
        if lin <= 0.0031308:
            sRGB = lin * 12.92
        else:
            sRGB = 1.055 * (lin ** (1 / 2.4)) - 0.055
        return min([255, sRGB * 255])

    @classmethod
    def generate_chromatic_aberration(cls, input, amount):
        img_r = np.zeros((cls.width, cls.height))
        img_g = np.zeros((cls.width, cls.height))
        img_b = np.zeros((cls.width, cls.height))
        for x in range(0, cls.width):
            for y in range(0, cls.height):
                img_r[x][y] = input[y*cls.width*3 + x*3 + 0]
                img_g[x][y] = input[y*cls.width*3 + x*3 + 1]
                img_b[x][y] = input[y*cls.width*3 + x*3 + 2]
        strength = amount/3000
        img_r = scipy.ndimage.interpolation.zoom(img_r, 1-strength)
        img_b = scipy.ndimage.interpolation.zoom(img_b, 1+strength)
        w_r = len(img_r)
        h_r = len(img_r[0])
        w_b = len(img_b)
        h_b = len(img_b[0])
        for x in range(0, cls.width):
            for y in range(0, cls.height):
                x_r = int(round((w_r-cls.width)/2 + x))
                y_r = int(round((h_r-cls.height)/2 + y))
                x_b = int(round((w_b-cls.width)/2 + x))
                y_b = int(round((h_b-cls.height)/2 + y))
                if 0 <= x_r < w_r and 0 <= y_r < h_r:
                    input[y*cls.width*3 + x*3 + 0] = img_r[x_r][y_r]
                else:
                    input[y*cls.width*3 + x*3 + 0] = 0
                input[y*cls.width*3 + x*3 + 1] = img_g[x][y]
                input[y*cls.width*3 + x*3 + 2] = img_b[x_b][y_b]
        del img_r
        del img_g
        del img_b

    @ classmethod
    def generate_distortion(cls, input_file, output_file, fx, fy, cx, cy, k1, k2, p1, p2, k3):
        cam_mat = cls.get_camera_matrix(fx, fy, cx, cy)
        dist_coeff = cls.get_dist_coeff(k1, k2, p1, p2, k3)
        distort_image.distort(input_file, output_file, cam_mat, dist_coeff)

    @ classmethod
    def write_image(cls, img_array, file):
        img_type = file.split('.')[1]
        if img_type == "jpg":
            img = Image.new('RGB', (cls.width, cls.height), (0, 0, 0))
            for i in range(0, cls.width):
                for j in range(0, cls.height):
                    ch_r = round(cls.linear_to_sRGB(img_array[i*cls.height*3 + j*3 + 0]))
                    ch_g = round(cls.linear_to_sRGB(img_array[i*cls.height*3 + j*3 + 1]))
                    ch_b = round(cls.linear_to_sRGB(img_array[i*cls.height*3 + j*3 + 2]))
                    img.putpixel((i, j), (int(ch_r), int(ch_g), int(ch_b)))
            img.save(file)
        elif img_type == "exr":
            r_out = []
            g_out = []
            b_out = []
            for i in range(0, cls.height):
                for j in range(0, cls.width):
                    r_out.append(img_array[j*cls.height*3 + i*3 + 0])
                    g_out.append(img_array[j*cls.height*3 + i*3 + 1])
                    b_out.append(img_array[j*cls.height*3 + i*3 + 2])
            data_r = array.array('f', r_out).tostring()
            data_g = array.array('f', g_out).tostring()
            data_b = array.array('f', b_out).tostring()
            exr_out = OpenEXR.OutputFile(file, OpenEXR.Header(cls.width, cls.height))
            exr_out.writePixels({'R': data_r, 'G': data_g, 'B': data_b})
        else:
            print("no valid file type specified.")


input_string = 'C:/Users/maxim/eclipse-workspace/Optical_Aberrations/res/inputs/table.jpg'
output_string = 'C:/Users/maxim/eclipse-workspace/Optical_Aberrations/res/outputs/table_test.jpg'
a = OpticalAberration()
a.set_parameters(input_string, output_string, 200, 3.5, 'coma', 3, 1, 1, 1, 0, 0, 0, 0)
a.generate_aberration()
#a.generate_distortion(input_string, output_string, 3034.59, 3034.59, 1501.63, 1973.7, 0.0349199, -0.08665, -0.000251559, -0.000103521, 0.0861223)