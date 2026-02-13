from acquisition.storage import BLOC_SIZE
import numpy as np
import sys

LINES_DISPLAY = 8

def def_lines(controller):
    middleLine = "|======|=======|"+controller.nbChannels*"==============|"+"==================|"
    emptyLine = "|  --- |   --- |"+controller.nbChannels*"     ------   |"+"     ---------    |"
    return middleLine, emptyLine

def update_disp(controller, samples):
    middleLine, emptyLine = def_lines(controller)
    if controller.received % LINES_DISPLAY == 0:        
        sys.stdout.write(f"\033[{LINES_DISPLAY}F")
    
    print(  f"| {controller.received:4d} |"
            + f"{controller.n_blocks:6d} |"
            + f"{np.mean(np.array(samples[0])):12.2f}  |"
            + f"{np.mean(np.array(samples[1])):12.2f}  |"
            + f"{controller.delay*1e3:13.2f}     |"
            + f"{controller.timeTot*1e3/controller.received:13.2f}     |")
    
    if controller.received == controller.n_blocks:
        for _ in range(LINES_DISPLAY - controller.received % LINES_DISPLAY - 1):
            print(emptyLine)
        print(middleLine)
    sys.stdout.flush()
        
def init_disp(controller, args):
    middleLine, emptyLine = def_lines(controller)
    print(f"\nDuree de l'acquisition      = {args.duration_s}s")
    print(f"Fréquence d'échantillonnage = {controller.fe } Hz")
    print(f"Nombre de bloc attendus     = {controller.n_blocks}")
    print(f"Temps estimé                = {controller.n_blocks*BLOC_SIZE/controller.fe:.2f} s --> {1e3*BLOC_SIZE/controller.fe:.2f} ms/bloc\n")
    print(middleLine)
    moyChannels = ""
    for i in range(controller.nbChannels):
        moyChannels += f" Moyenne ch {i+1} |"
    print(f"| Reçu | Total |"+moyChannels+" Délai récep (ms) |")
    print(middleLine)
    print(emptyLine + (LINES_DISPLAY-1) * f"\n{emptyLine}")
    print(middleLine)
    print(3*"\n")
    sys.stdout.write(f"\033[{LINES_DISPLAY+4}F")
    sys.stdout.flush()