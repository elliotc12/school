from __future__ import division
import numpy as np
import visual as vis

dt = 0.01
Tfinal = 0.8
Eo = 1
w = 3
q = 1
m = 0.1
s = vis.vector(0.1, -0.4, 0)
v = vis.vector(0, 0, 0)
a = vis.vector(0, 0, 0)
t = 0

ball = vis.sphere(pos = s, radius = 0.1, make_trail = True)
plate1 = vis.cylinder(pos = vis.vector(0 , 0.5, 0), radius = 0.5, axis=(0, 0.05, 0))
plate2 = vis.cylinder(pos = vis.vector(0, -0.5, 0), radius = 0.5, axis=(0,-0.05, 0))

while t < Tfinal:
    vis.rate(100)
    t += dt
    a = vis.vector(0,0,0)
    if (s.y < 0.5 and s.y > -0.5 and np.sqrt(s.x**2 + s.z**2) < 0.5):
        E = vis.vector(0, Eo * np.cos(w * t), 0)
        phi = np.arctan(s.z/s.x)
        Bo = - vis.mag(s) * w * Eo * np.sin(w * t) / 2
        B = vis.vector(Bo * np.cos(phi), Bo * np.sin(phi), 0)
        F = q * (E + vis.cross(v, B))
        a = F/m
    v = a*dt + v
    s = v*dt + s
    ball.pos = s
