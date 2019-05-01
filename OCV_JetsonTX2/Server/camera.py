import time
from base_camera import BaseCamera
import os
import sys
import struct
import errno

FIFO = '/tmp/myfifo'

class Camera(BaseCamera):
    """An emulated camera implementation that streams a repeated sequence of
    files 1.jpg, 2.jpg and 3.jpg at a rate of one frame per second."""
    
    #imgs = [open(f + '.jpg', 'rb').read() for f in ['0','1', '2', '3','4','5','6','7','8','9']]

    @staticmethod
    def frames():
        while True:
            #time.sleep(1)
            with open(FIFO, "rb") as fifo:
            	yield fifo.read()
            	fifo.close()
    			
            
