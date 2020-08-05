import run_aberration
import time
from os import listdir
from os.path import isfile, join

# setup
params_dest = 'params.txt'
params = [line.rstrip('\n') for line in open(params_dest)]

input_folder = params[1].split(" ")[2]
output_folder = params[2].split(" ")[2]
lens_file = params[3].split(" ")[2]
output_type = params[4].split(" ")[2]
monochrome = int(params[5].split(" ")[2])
x_min = int(params[8].split(" ")[2])
x_max = int(params[9].split(" ")[2])
y_min = int(params[10].split(" ")[2])
y_max = int(params[11].split(" ")[2])
samples = int(params[14].split(" ")[2])
exposure = float(params[15].split(" ")[2])
aberration = params[18].split(" ")[2]
aberration_size = float(params[19].split(" ")[2])
chromatic_aberration = float(params[20].split(" ")[2])
dark_current_noise = float(params[23].split(" ")[2])
readout_noise = float(params[24].split(" ")[2])
shot_noise = float(params[25].split(" ")[2])
distort = int(params[28].split(" ")[2])
fx = float(params[29].split(" ")[2])
fy = float(params[30].split(" ")[2])
cx = float(params[31].split(" ")[2])
cy = float(params[32].split(" ")[2])
k1 = float(params[33].split(" ")[2])
k2 = float(params[34].split(" ")[2])
p1 = float(params[35].split(" ")[2])
p2 = float(params[36].split(" ")[2])
k3 = float(params[37].split(" ")[2])

files = [f for f in listdir(input_folder) if isfile(join(input_folder, f))]
files_num = len(files)

a = run_aberration.OpticalAberration()

print("")
for i in range(0, files_num):
    print("Image " + str(i+1) + "/" + str(files_num) + ": " + files[i])
    input = join(input_folder, files[i])
    output = join(output_folder, files[i]).split('.')[0] + '_rendered_' + time.strftime("%Y%m%d%H%M%S") + '.' + output_type
    a.generate_aberration(input, output, lens_file, monochrome, samples, exposure, aberration, aberration_size, chromatic_aberration, dark_current_noise, readout_noise, shot_noise, x_min, x_max, y_min, y_max, distort, fx, fy, cx, cy, k1, k2, p1, p2, k3)
    #a.generate_distortion(input, output, 3034.59, 3034.59, 1501.63, 1973.7, 0.0349199, -0.08665, -0.000251559, -0.000103521, 0.0861223)
    print("")