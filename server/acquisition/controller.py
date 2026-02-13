from time import time
import numpy as np

from acquisition.storage import DataStorage
from acquisition.display import update_disp, init_disp

NB_CHANNELS = 2

class AcquisitionController:
    def __init__(self, n_blocks: int):
        self.n_blocks = n_blocks
        self.received = 0
        self.last_timestamp = None
        self.storages:list[DataStorage]
        self.nbChannels = NB_CHANNELS
        

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
    
    def init_display(self, args):
        init_disp(self, args)
        
    def update_display(self,samples):
        update_disp(self,samples)
    
    def is_finished(self) -> bool:
        return self.received >= self.n_blocks

    def close(self):
        if self.storages:
            for storage in self.storages:
                storage.close()
