# =============================================================================
# Getting Started with Images
# https://docs.opencv.org/4.0.0-alpha/dc/d2e/tutorial_py_image_display.htmla
# =============================================================================

import numpy as np
import cv2 as cv

imgcolor  = cv.imread('ssbbldg.jpg',1)
imggrayscale  = cv.imread('ssbbldg.jpg',0)
imgunchanged  = cv.imread('ssbbldg.jpg',-1)

cv.imshow('Color', imgcolor)
cv.imshow('Grayscale', imggrayscale)
cv.imshow('Unchanged', imgunchanged)

cv.imwrite('ssbgray.png',img)