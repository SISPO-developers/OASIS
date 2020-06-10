import python_to_c
import numpy as np
from PIL import Image

class OpticalAberration:
    input = []
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

    def __init__(self):
        pass

    @classmethod
    def generate_aberration(cls):
        output_img = python_to_c.pass_to_c(cls.input, cls.samples, cls.exposure, cls.aberration, cls.strength, cls.x_min, cls.x_max, cls.y_min, cls.y_max, cls.width, cls.height)
        cls.write_image(output_img, cls.output)

    @classmethod
    def set_parameters(cls, in_file, out_file, samples, exp, aberration, strength, x_min, x_max, y_min, y_max):
        cls.read_image(in_file)
        cls.output = out_file
        cls.samples = samples
        cls.exposure = exp
        cls.aberration = aberration
        cls.strength = strength
        if x_max - x_min < 1 or y_max - y_min < 1:
            cls.x_min = 0
            cls.x_max = cls.width
            cls.y_min = 0
            cls.y_max = cls.height
        else:
            cls.x_min = x_min
            cls.x_max = x_max
            cls.y_min = y_min
            cls.y_max = y_max

    @classmethod
    def read_image(cls, file):
        img_type = file.split('.')[1]
        if img_type == 'jpg' or img_type == 'jpeg' or img_type == 'png':
            img = Image.open(file)
            cls.input = list(img.getdata())
            cls.width, cls.height = img.size
        elif img_type == 'exr':
            pass

    @ classmethod
    def write_image(cls, img_array, file):
        img = Image.new('RGB', (cls.width, cls.height), (0, 0, 0))
        for i in range(0, cls.width):
            for j in range(0, cls.height):
                rgb = img_array[i][j]
                for ch in range(0, 3):
                    rgb[ch] = min([255, round(rgb[ch])])
                img.putpixel((i, j), (int(rgb[0]), int(rgb[1]), int(rgb[2])))
        img.save(file)

input_string = 'C:/Users/maxim/eclipse-workspace/Optical_Aberrations/res/inputs/table.jpg'
output_string = 'C:/Users/maxim/eclipse-workspace/Optical_Aberrations/res/outputs/table_out.jpg'
a = OpticalAberration()
a.set_parameters(input_string, output_string, 200, 340, 'coma', 8, 0, 0, 0, 0)
a.generate_aberration()