from time import time
import numpy as np

import sys
from acquisition.storage import DataStorage


NB_CHANNELS = 2
linesToDisplay = 8
middleLine = "|======|=======|"+NB_CHANNELS*"================|"+"==================|"
emptyLine = "|  --- |   --- |"+NB_CHANNELS*"      -------   |"+"     ---------    |"

class AcquisitionController:
    def __init__(self, n_blocks: int):
        self.n_blocks = n_blocks
        self.received = 0
        self.last_timestamp = None
        self.storages:list[DataStorage]
        

    def init(self, fe: int, storages: list[DataStorage]):
        self.received = 0
        self.last_timestamp = None
        self.storages        = storages
        self.fe             = fe
        self.delay          = time()
        self.timeTot = 0.0

    def process_block(self, timestamp, samples:list):
        
        self.received += 1
        self.timeTot += self.delay
        
        self.last_timestamp = timestamp
        for i in range(len(self.storages)):
            self.storages[i].append_block(samples[i])
        

    def update_display(self, samples):
        if self.received % linesToDisplay == 0:
            print(middleLine)
            print(3*"\n")
            sys.stdout.write(f"\033[{linesToDisplay+5}F")
        
        print(  f"| {self.received:4d} |"
              + f"{self.n_blocks:6d} |"
              + f"{np.mean(np.array(samples[0])):13.2f}   |"
              + f"{np.mean(np.array(samples[1])):13.2f}   |"
              + f"{self.delay*1e3:13.2f}     |"
              + f"{self.timeTot*1e3/self.received:13.2f}     |")
        
        if self.received == self.n_blocks:
            for _ in range(linesToDisplay - self.received % linesToDisplay):
                print(emptyLine)
        
        sys.stdout.flush()
            
    def is_finished(self) -> bool:
        return self.received >= self.n_blocks

    def close(self):
        if self.storages:
            for storage in self.storages:
                storage.close()
