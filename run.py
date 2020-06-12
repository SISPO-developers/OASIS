import python_to_c
import numpy as np
import OpenEXR
import Imath
import array
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
    use_exr = False

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
            cls.use_exr = False
            img = Image.open(file)
            cls.input = list(img.getdata())
            cls.width, cls.height = img.size
        elif img_type == 'exr':
            cls.use_exr = True
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
            for j in range(0, cls.width):
                for i in range(0, cls.height):
                    cls.input.append(cc[j][i])

    @ classmethod
    def write_image(cls, img_array, file):
        if cls.use_exr == False:
            img = Image.new('RGB', (cls.width, cls.height), (0, 0, 0))
            for i in range(0, cls.width):
                for j in range(0, cls.height):
                    rgb = img_array[i][j]
                    for ch in range(0, 3):
                        rgb[ch] = min([255, round(rgb[ch])])
                    img.putpixel((i, j), (int(rgb[0]), int(rgb[1]), int(rgb[2])))
            img.save(file)
        else:
            file_name = file.split('.')[0] + ".exr"
            r_out = []
            g_out = []
            b_out = []
            for i in range(0, cls.height):
                for j in range(0, cls.width):
                    r_out.append(img_array[j][i][0])
                    g_out.append(img_array[j][i][1])
                    b_out.append(img_array[j][i][2])
            data_r = array.array('f', r_out).tostring()
            data_g = array.array('f', g_out).tostring()
            data_b = array.array('f', b_out).tostring()
            exr_out = OpenEXR.OutputFile(file_name, OpenEXR.Header(cls.width, cls.height))
            exr_out.writePixels({'R': data_r, 'G': data_g, 'B': data_b})


input_string = 'C:/Users/maxim/eclipse-workspace/Optical_Aberrations/res/inputs/c_1.exr'
output_string = 'C:/Users/maxim/eclipse-workspace/Optical_Aberrations/res/outputs/c_1_test.jpg'
a = OpticalAberration()
a.set_parameters(input_string, output_string, 200, 340, 'coma', 8, 0, 0, 0, 0)
a.generate_aberration()