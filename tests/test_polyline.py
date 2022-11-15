import unittest

import euklid


class TestPolyline(unittest.TestCase):
    def setUp(self):
        self.p1 = euklid.vector.PolyLine2D([
            [1.7273, 9.5983],
            [1.3771, -0.3956],
            [0.9972, -0.3956],
            [0.9909, -0.3922],
            [1.2346, 9.6048],
            [1.7273, 9.5983]
            ])
        
        self.p2 = euklid.vector.PolyLine2D([
            [0, 0],
            [0.8974, -0.0130],
            [1.0935, -0.0193],
            [1.2883, -0.0268],
            [1.4817, -0.0357],
            [2, 0],
            [2, -1],
            [0, -1],
            [0, 0]
        ])
    
    def test_union(self):
        union = self.p1.bool_union(self.p2)

if __name__ == "__main__":
    unittest.main()