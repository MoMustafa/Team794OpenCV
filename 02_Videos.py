# =============================================================================
# Getting started with Videos
# https://docs.opencv.org/4.0.0-alpha/dd/d43/tutorial_py_video_display.html
# =============================================================================

import numpy as np
import cv2 as cv

cap = cv.VideoCapture(0)

while(True):
    ret, frame = cap.read()
    gray = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)
    cv.imshow('frame', gray)
    
    if cv.waitKey(1)&0xFF == ord('q'):
        break
cap.release()
    
    
    