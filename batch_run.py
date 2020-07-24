import run_aberration
import time
from os import listdir
from os.path import isfile, join

# setup
input_folder = 'C:/Users/maxim/eclipse-workspace/Optical_Aberrations/res/inputs/test'
output_folder = 'C:/Users/maxim/eclipse-workspace/Optical_Aberrations/res/outputs/test'
output_type = 'jpg'
samples = 1
exposure = 1
aberration = 'coma'
aberration_size = 0
chromatic_aberration = 0
dark_current_noise = 0
readout_noise = 0
shot_noise = 0
x_min = 0
x_max = 0
y_min = 0
y_max = 0
distort = 0 # set to 1 to simulate camera distortion
fx = 3034.59
fy = 3034.59
cx = 1501.63
cy = 1973.7
k1 = 0.0349199
k2 = -0.08665
p1 = -0.000251559
p2 = -0.000103521
k3 = 0.0861223

files = [f for f in listdir(input_folder) if isfile(join(input_folder, f))]
files_num = len(files)

a = run_aberration.OpticalAberration()

print("")
for i in range(0, files_num):
    print("Image " + str(i+1) + "/" + str(files_num) + ": " + files[i])
    input = join(input_folder, files[i])
    output = join(output_folder, files[i]).split('.')[0] + '_rendered_' + time.strftime("%Y%m%d%H%M%S") + '.' + output_type
    a.generate_aberration(input, output, samples, exposure, aberration, aberration_size, chromatic_aberration, dark_current_noise, readout_noise, shot_noise, x_min, x_max, y_min, y_max, distort, fx, fy, cx, cy, k1, k2, p1, p2, k3)
    #a.generate_distortion(input, output, 3034.59, 3034.59, 1501.63, 1973.7, 0.0349199, -0.08665, -0.000251559, -0.000103521, 0.0861223)
    print("")