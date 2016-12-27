from graphics import *
import random
import time
x = 1920
y = 1080
boxSize = 400
win = GraphWin("target",x,y)
nextTarget = 0
test = 1


background = Rectangle(Point(0,0), Point(x,y))
background.draw(win)
background.setOutline('black')
background.setFill('black')

win.getMouse()
for i in range (1,1000):
	#draw target
	randx= float(x-boxSize)*random.random()
	randy= float(y-100-boxSize)*random.random()

	rx = int(randx)
	ry = int(randy)
	pt1 = Point(rx, ry)
	pt2 = Point((rx+boxSize), (ry+boxSize))

	target = Rectangle(pt1, pt2)
	target.draw(win)
	target.setOutline('white')
	target.setFill('white')
	test = test + 1
	nextTarget = 0
	#win.getMouse()
	time.sleep(10)
	target.setOutline('black')
	target.setFill('black')