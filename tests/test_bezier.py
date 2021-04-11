#! /usr/bin/python2
# -*- coding: utf-8; -*-
#
# (c) 2013 booya (http://booya.at)
#
# This file is part of the OpenGlider project.
#
# OpenGlider is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# OpenGlider is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OpenGlider.  If not, see <http://www.gnu.org/licenses/>.
from __future__ import division

import unittest
import random

from euklid.spline import BezierCurve


class TestBezier(unittest.TestCase):
    Curve = BezierCurve
    def setUp(self):
        controlpoints = [[i, random.random()] for i in range(15)]
        self.numpoints = len(controlpoints)
        self.bezier = self.Curve(controlpoints)

    def test_get_value(self):
        val = random.random() * (self.numpoints-1)
        self.assertAlmostEqual(self.bezier.get(val)[0], self.bezier.get(val)[0])
        self.assertAlmostEqual(self.bezier.get(val)[1], self.bezier.get(val)[1])

    def test_fit(self):
        num = self.numpoints
        to_fit = self.bezier.get_sequence(50)
        bezier2 = self.Curve.fit(to_fit, numpoints=num)
        for p1, p2 in zip(self.bezier.controlpoints, bezier2.controlpoints):
            self.assertAlmostEqual(p1[0], p2[0], 0)
            self.assertAlmostEqual(p1[1], p2[1], 0)

    def test_length(self):
        self.bezier.controlpoints = [[0, 0], [2, 0]]
        sequence = self.bezier.get_sequence(10)
        self.assertAlmostEqual(sequence.get_length(), 2.)

    def test_get_sequence(self):
        sequence = self.bezier.get_sequence(100)
        # print(sequence)



if __name__ == '__main__':
    unittest.main(verbosity=2)