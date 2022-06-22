import unittest
import euklid

class TestCurve(unittest.TestCase):
    def setUp(self) -> None:
        self.curve = euklid.vector.PolyLine2D([
                    [
                        -0.10104283355824818,
                        -0.1923445451803646
                    ],
                    [
                        2.7646271914588056,
                        -0.2998482674465057
                    ],
                    [
                        2.9331737845193144,
                        -0.3375670982030803
                    ],
                    [
                        4.176588348946094,
                        -0.693168305324235
                    ],
                    [
                        4.36715677382829,
                        -0.7732455672374599
                    ]
                ])
        return super().setUp()
        
    def test_cut(self):
        p1 = euklid.vector.Vector2D([4.176588348946094, -0.6066228276490806])
        p2 = euklid.vector.Vector2D([4.176588348946094, -1.3933998974232116])

        cuts = self.curve.cut(p1, p2)
        self.assertEqual(len(cuts), 1)


