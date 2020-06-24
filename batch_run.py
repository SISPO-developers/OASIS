import run_aberration
import time
from os import listdir
from os.path import isfile, join

# setup
input_folder = 'C:/Users/maxim/eclipse-workspace/Optical_Aberrations/res/inputs/test'
output_folder = 'C:/Users/maxim/eclipse-workspace/Optical_Aberrations/res/outputs/test'
output_type = 'jpg'
samples = 500
exposure = 3.5
aberration = 'coma'
aberration_size = 3
chromatic_aberration = 0
dark_current_noise = 1
readout_noise = 1
x_min = 0
x_max = 0
y_min = 0
y_max = 0

files = [f for f in listdir(input_folder) if isfile(join(input_folder, f))]
files_num = len(files)

a = run_aberration.OpticalAberration()
for i in range(0, files_num):
    print("Image " + str(i+1) + "/" + str(files_num) + ": " + files[i])
    input = join(input_folder, files[i])
    output = join(output_folder, files[i]).split('.')[0] + '_rendered_' + time.strftime("%Y%m%d%H%M%S") + '.' + output_type
    a.generate_aberration(input, output, samples, exposure, aberration, aberration_size, chromatic_aberration, dark_current_noise, readout_noise, x_max, x_min, y_min, y_max)
    #a.generate_distortion(input, output, 3034.59, 3034.59, 1501.63, 1973.7, 0.0349199, -0.08665, -0.000251559, -0.000103521, 0.0861223)
    print("")