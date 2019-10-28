# -*- coding: utf-8 -*-
"""
Created on Sat Oct 26 18:46:04 2019

@author: Maxime
"""
from math import sqrt, pow
from random import randrange
print("Printing random integer ", randrange(0, 10, 2))

f = open("StarsRandomCoords.txt", "w")
for i in range(10000):
    while(True):
        number1 = randrange(-5000, 5000, 1)
        number2 = randrange(-5000, 5000, 1)
        number3 = randrange(-5000, 5000, 1)
        size = randrange(1, 7, 1)/2.0
        if abs(sqrt(number1**2 + number2**2 + number3**2)) > 2500:
            break
    f.write(str(number1))
    f.write(" ")
    f.write(str(number2))
    f.write(" ")
    f.write(str(number3))
    f.write(" ")
    f.write(str(size))
    f.write(" ")
    f.write("\n")
f.close()
