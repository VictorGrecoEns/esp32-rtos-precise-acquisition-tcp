from acquisition.storage import DataStorage
import sys
import numpy as np
from time import time

linesToDisplay = 15
middleLine = "|-------|-------|----------------|------------------|"
emptyLine = "|   --- |   --- |      -------   |     ---------    |"
class AcquisitionController:
    def __init__(self, n_blocks):
        self.n_blocks = n_blocks
        self.received = 0
        self.last_timestamp = None
        self.storage:DataStorage
        self.delay = 0.0

    def init(self, fe, storage:DataStorage):
        self.received       = 0
        self.last_timestamp = None
        self.storage        = storage
        self.fe             = fe
        self.delay          = time()

    def process_block(self, timestamp, samples:dict):
        
        if self.received % linesToDisplay == 0:
            print(middleLine)
            print(3*"\n")
            sys.stdout.write(f"\033[{linesToDisplay+5}F")
        self.received += 1
        print(f"|  {self.received:4d} |{self.n_blocks:6d} |   {np.mean(np.array(samples)):10.2f}   |{self.delay*1e3:13.2f}     |")
        
        if self.received == self.n_blocks:
            for _ in range(linesToDisplay - self.received % linesToDisplay):
                print(emptyLine)
        
        
        sys.stdout.flush()
        
        # if self.last_timestamp is not None:
        #     dt = timestamp - self.last_timestamp
        #     fe_eval = len(samples) * 1e6 / dt
        #     print(f"Bloc {self.received}/{self.n_blocks} fe={fe_eval:.1f} Hz, {timestamp=}")
        # else:
        #     print(f"Bloc {self.received}/{self.n_blocks}, {timestamp=}")
        
        
        self.last_timestamp = timestamp

        self.storage.append_block(samples)

    def is_finished(self):
        return self.received >= self.n_blocks

    def close(self):
        self.storage.close()
