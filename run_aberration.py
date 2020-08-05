import python_to_c
import distort_image
import sys
import math
import time
import numpy as np
import OpenEXR
import Imath
import array
import cv2
import scipy.ndimage.interpolation
from PIL import Image

class OpticalAberration:

    def __init__(self):
        pass

    @classmethod
    def time_to_string(cls, time):
        hours = math.floor(time/(1000*60*60))
        minutes = math.floor(time/(1000*60)) - hours*60
        seconds = math.floor(time/1000) - minutes*60 - hours*60*60
        h_str = str(hours)
        m_str = str(minutes)
        s_str = str(seconds)
        if hours < 10:
            h_str = "0" + h_str
        if minutes < 10:
            m_str = "0" + m_str
        if seconds < 10:
            s_str = "0" + s_str
        return h_str + ":" + m_str + ":" + s_str		

    @classmethod
    def generate_aberration(cls, in_string, out_string, lens_string, monochrome, samples, exposure, aberration, size, ca_size, dark_noise, read_noise, shot_noise, x_min, x_max, y_min, y_max, dist, fx, fy, cx, cy, k1, k2, p1, p2, k3):
        start = int(round(time.time() * 1000))
        print("Importing image.")
        dimensions = [0, 0]
        render_dimensions = [x_min, x_max, y_min, y_max]
        input = cls.read_image(in_string, dimensions, render_dimensions)
        width = dimensions[0]
        height = dimensions[1]
        x_min = render_dimensions[0]
        x_max = render_dimensions[1]
        y_min = render_dimensions[2]
        y_max = render_dimensions[3]
        lens = []
        lens_scale = 0
        lens_offset = 0
        lens_dim = [0, 0]
        if lens_string != '0' and (size > 0 or shot_noise > 0 or dark_noise > 0 or read_noise > 0):
            print("Importing lens file.")
            lens = cls.read_image(lens_string, lens_dim, [0,0,0,0])
            if width > height:
                lens_scale = lens_dim[0]/width
                lens_offset = (lens_dim[1]-lens_scale*height)/2
            else:
                lens_scale = lens_dim[1]/height
                lens_offset = (lens_dim[0]-lens_scale*width)/2
        if ca_size > 0:
            cls.generate_chromatic_aberration(input, ca_size, width, height)
        if dist > 0:
            cam_mat = cls.get_camera_matrix(fx, fy, cx, cy)
            dist_coeff = cls.get_dist_coeff(k1, k2, p1, p2, k3)
            input = distort_image.distort_matrix(width, height, input, cam_mat, dist_coeff)
            print("Distortion applied.")
        output_img = []
        if size > 0 or shot_noise > 0 or dark_noise > 0 or read_noise > 0:
            output_img = python_to_c.pass_to_c(input, samples, exposure, aberration, size, dark_noise, read_noise, shot_noise, x_min, x_max, y_min, y_max, lens, lens_scale, lens_offset, lens_dim[0], lens_dim[1], width, height)
        else:
            output_img = input
        cls.write_image(output_img, out_string, width, height, monochrome)
        end = int(round(time.time() * 1000)) - start
        total_str = cls.time_to_string(end)
        print("Total time: " + total_str)

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
    def read_image(cls, file, dimensions, render_dimensions):
        sys.stdout.write("Reading file:   0 %")
        input = []
        exr_in = 0
        img_type = file.split('.')[1]
        if img_type == 'jpg' or img_type == 'jpeg' or img_type == 'png' or img_type == 'JPG' or img_type == 'JPEG' or img_type == 'PNG':
            exr_in = 0
            img = Image.open(file)
            input = list(img.getdata())
            width, height = img.size
        elif img_type == 'exr':
            exr_in = 1
            exrFile = OpenEXR.InputFile(file)
            header = exrFile.header()
            dw = header['dataWindow']
            pt = Imath.PixelType(Imath.PixelType.FLOAT)
            width, height = (dw.max.x - dw.min.x + 1, dw.max.y - dw.min.y + 1)
            cc_r = np.frombuffer(exrFile.channel('R', pt), dtype=np.float32)
            cc_g = np.frombuffer(exrFile.channel('G', pt), dtype=np.float32)
            cc_b = np.frombuffer(exrFile.channel('B', pt), dtype=np.float32)
            cc_r.shape = cc_g.shape = cc_b.shape = (width, height)
            cc = np.dstack((cc_r, cc_g, cc_b))
            input = [[0, 0, 0] for i in range(width * height)]
            for j in range(0, width):
                for i in range(0, height):
                    input[j*height + i] = cc[j][i]
            del cc
            del cc_r
            del cc_g
            del cc_b
        input_1d = [0 for i in range(width * height * 3)]
        for i in range(0, width):
            sys.stdout.write("\r")
            sys.stdout.write("Reading file:\t" + str(round(i/width*100)) + " %")
            for j in range(0, height):
                for k in range(0, 3):
                    if exr_in == 0:
                        input_1d[i*height*3 + j*3 + k] = cls.sRGB_to_linear(input[i*height + j][k])
                    else:
                        input_1d[i*height*3 + j*3 + k] = input[i*height + j][k]
        del input[:]
        del input
        if render_dimensions[1] - render_dimensions[0] < 1 or render_dimensions[3] - render_dimensions[2] < 1:
            render_dimensions[0] = 0
            render_dimensions[1] = width
            render_dimensions[2] = 0
            render_dimensions[3] = height
        dimensions[0] = width
        dimensions[1] = height
        print("")
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
    def generate_chromatic_aberration(cls, input, amount, width, height):
        img_r = np.zeros((width, height))
        img_g = np.zeros((width, height))
        img_b = np.zeros((width, height))
        for x in range(0, width):
            sys.stdout.write("\r")
            sys.stdout.write("Applying chromatic aberration:\t" + str(round(x/width*50)) + " %")
            for y in range(0, height):
                img_r[x][y] = input[y*width*3 + x*3 + 0]
                img_g[x][y] = input[y*width*3 + x*3 + 1]
                img_b[x][y] = input[y*width*3 + x*3 + 2]
        strength = amount/3000
        img_r = scipy.ndimage.interpolation.zoom(img_r, 1-strength)
        img_b = scipy.ndimage.interpolation.zoom(img_b, 1+strength)
        w_r = len(img_r)
        h_r = len(img_r[0])
        w_b = len(img_b)
        h_b = len(img_b[0])
        for x in range(0, width):
            sys.stdout.write("\r")
            sys.stdout.write("Applying chromatic aberration:\t" + str(round(x/width*50 + 50)) + " %")
            for y in range(0, height):
                x_r = int(round((w_r-width)/2 + x))
                y_r = int(round((h_r-height)/2 + y))
                x_b = int(round((w_b-width)/2 + x))
                y_b = int(round((h_b-height)/2 + y))
                if 0 <= x_r < w_r and 0 <= y_r < h_r:
                    input[y*width*3 + x*3 + 0] = img_r[x_r][y_r]
                else:
                    input[y*width*3 + x*3 + 0] = 0
                input[y*width*3 + x*3 + 1] = img_g[x][y]
                input[y*width*3 + x*3 + 2] = img_b[x_b][y_b]
        del img_r
        del img_g
        del img_b
        print("")

    @ classmethod
    def generate_distortion(cls, input_file, output_file, fx, fy, cx, cy, k1, k2, p1, p2, k3):
        cam_mat = cls.get_camera_matrix(fx, fy, cx, cy)
        dist_coeff = cls.get_dist_coeff(k1, k2, p1, p2, k3)
        distort_image.distort(input_file, output_file, cam_mat, dist_coeff)

    @ classmethod
    def write_image(cls, img_array, file, width, height, monochrome):
        sys.stdout.write("Writing file:   0 %")
        img_type = file.split('.')[1]
        if img_type == 'jpg' or img_type == 'png' or img_type == 'JPG' or img_type == 'JPEG' or img_type == 'PNG':
            img = Image.new('RGB', (width, height), (0, 0, 0))
            if monochrome == 1:
                for i in range(0, width):
                    sys.stdout.write("\r")
                    sys.stdout.write("Writing file:\t" + str(round(i/width*100)) + " %")
                    for j in range(0, height):
                        ch_r = round(cls.linear_to_sRGB(img_array[j*width*3 + i*3 + 0]))
                        ch_g = round(cls.linear_to_sRGB(img_array[j*width*3 + i*3 + 1]))
                        ch_b = round(cls.linear_to_sRGB(img_array[j*width*3 + i*3 + 2]))
                        avg = int((ch_r + ch_g + ch_b)/3)
                        img.putpixel((i, j), (avg, avg, avg))
            else:
                for i in range(0, width):
                    sys.stdout.write("\r")
                    sys.stdout.write("Writing file:\t" + str(round(i/width*100)) + " %")
                    for j in range(0, height):
                        ch_r = round(cls.linear_to_sRGB(img_array[j*width*3 + i*3 + 0]))
                        ch_g = round(cls.linear_to_sRGB(img_array[j*width*3 + i*3 + 1]))
                        ch_b = round(cls.linear_to_sRGB(img_array[j*width*3 + i*3 + 2]))
                        img.putpixel((i, j), (int(ch_r), int(ch_g), int(ch_b)))
            img.save(file)
        elif img_type == "exr":
            r_out = []
            g_out = []
            b_out = []
            if monochrome == 1:
                for i in range(0, height):
                    sys.stdout.write("\r")
                    sys.stdout.write("Writing file:\t" + str(round(i/height*100)) + " %")
                    for j in range(0, width):
                        avg = (img_array[i*width*3 + j*3 + 0] + img_array[i*width*3 + j*3 + 1] + img_array[i*width*3 + j*3 + 2])/3
                        r_out.append(avg)
                        g_out.append(avg)
                        b_out.append(avg)
            else:
                for i in range(0, height):
                    sys.stdout.write("\r")
                    sys.stdout.write("Writing file:\t" + str(round(i/height*100)) + " %")
                    for j in range(0, width):
                        r_out.append(img_array[i*width*3 + j*3 + 0])
                        g_out.append(img_array[i*width*3 + j*3 + 1])
                        b_out.append(img_array[i*width*3 + j*3 + 2])
            data_r = array.array('f', r_out).tobytes()
            data_g = array.array('f', g_out).tobytes()
            data_b = array.array('f', b_out).tobytes()
            exr_out = OpenEXR.OutputFile(file, OpenEXR.Header(width, height))
            exr_out.writePixels({'R': data_r, 'G': data_g, 'B': data_b})
        else:
            print("no valid file type specified.")
        print("")