#!/usr/bin/env python
"""
Test numerical stability improvements for cut_2d and offset functions.
"""
import unittest
import math
from euklid.vector import PolyLine2D, Vector2D


class TestNumericalStability(unittest.TestCase):
    """Test cases for numerical stability improvements."""
    
    def test_cut_2d_nearly_parallel_lines(self):
        """Test intersection of nearly parallel lines."""
        # Two lines that are nearly parallel (1 degree angle)
        angle = math.radians(1)
        
        p1 = PolyLine2D([[0, 0], [10, 0]])
        p2 = PolyLine2D([[0, 0.1], [10, 0.1 + 10 * math.tan(angle)]])
        
        # Should find intersection or handle gracefully
        cuts = p1.cut(p2)
        # If lines are detected as parallel, cuts should be empty
        # If intersection is found, it should be valid
        if len(cuts) > 0:
            self.assertIsInstance(cuts[0][0], float)
            self.assertIsInstance(cuts[0][1], float)
    
    def test_cut_2d_very_short_segments(self):
        """Test intersection with very short segments."""
        # One segment is very short (1e-9 length)
        p1 = PolyLine2D([[0, 0], [10, 0]])
        p2 = PolyLine2D([[5, -1], [5, -1 + 1e-9]])
        
        # Should handle gracefully without crashing
        cuts = p1.cut(p2)
        # Very short segment should be detected as degenerate
        self.assertEqual(len(cuts), 0)
    
    def test_cut_2d_exact_parallel_lines(self):
        """Test intersection of exactly parallel lines."""
        p1 = PolyLine2D([[0, 0], [10, 0]])
        p2 = PolyLine2D([[0, 1], [10, 1]])
        
        # Parallel lines should not intersect
        cuts = p1.cut(p2)
        self.assertEqual(len(cuts), 0)
    
    def test_cut_2d_perpendicular_lines(self):
        """Test intersection of perpendicular lines."""
        p1 = PolyLine2D([[0, 0], [10, 0]])
        p2 = PolyLine2D([[5, -5], [5, 5]])
        
        # Should find exactly one intersection at (5, 0)
        cuts = p1.cut(p2)
        self.assertEqual(len(cuts), 1)
        
        # Check intersection is at correct location
        ik1, ik2 = cuts[0]
        # ik1 should be 0.5 (halfway along first segment)
        self.assertAlmostEqual(ik1, 0.5, places=6)
        # ik2 should be 0.5 (halfway along second segment)
        self.assertAlmostEqual(ik2, 0.5, places=6)
    
    def test_cut_2d_with_nearest_ik(self):
        """Test cut with nearest_ik parameter."""
        p1 = PolyLine2D([[0, 0], [10, 0], [20, 0]])
        p2_start = Vector2D([5, -1])
        p2_end = Vector2D([5, 1])
        
        # Should find intersection near ik=0.5
        result = p1.cut(p2_start, p2_end, 0.5)
        self.assertIsInstance(result, tuple)
        self.assertEqual(len(result), 2)
        
        ik1, ik2 = result
        # Should be close to 0.5 on first segment
        self.assertAlmostEqual(ik1, 0.5, places=6)
    
    def test_offset_simple_square(self):
        """Test offset on a simple square."""
        # Create a square
        square = PolyLine2D([[0, 0], [10, 0], [10, 10], [0, 10], [0, 0]])
        
        # Offset inward by 1 unit
        offset_square = square.offset(1.0, simple=True)
        
        # Should have same number of points
        self.assertEqual(len(offset_square), len(square))
        
        # All points should be offset inward
        for i in range(len(square) - 1):
            orig_pt = square.get(i)
            offset_pt = offset_square.get(i)
            # Distance should be approximately 1.0 (or sqrt(2) at corners)
            dist = (offset_pt - orig_pt).length()
            self.assertGreater(dist, 0.5)
            self.assertLess(dist, 2.0)
    
    def test_offset_with_sharp_angle(self):
        """Test offset with sharp angle (potential miter spike)."""
        # Create a polyline with a sharp angle
        sharp = PolyLine2D([[0, 0], [10, 0], [10.1, 5]])
        
        # Offset should handle sharp angle gracefully
        offset_sharp = sharp.offset(1.0, simple=True)
        
        # Should not crash and should have same number of points
        self.assertEqual(len(offset_sharp), len(sharp))
    
    def test_offset_with_very_small_segments(self):
        """Test offset with very small segments."""
        # Create polyline with a very small segment
        small_seg = PolyLine2D([[0, 0], [10, 0], [10 + 1e-9, 0], [10, 10]])
        
        # Should handle gracefully
        offset_small = small_seg.offset(1.0, simple=True)
        
        # Should not crash
        self.assertIsInstance(offset_small, PolyLine2D)
    
    def test_cut_with_mixed_scales(self):
        """Test cut with very different coordinate scales."""
        # Large scale line
        p1 = PolyLine2D([[0, 0], [1e6, 0]])
        # Small scale line
        p2 = PolyLine2D([[5e5, -1e-3], [5e5, 1e-3]])
        
        # Should find intersection
        cuts = p1.cut(p2)
        
        if len(cuts) > 0:
            ik1, ik2 = cuts[0]
            # Should be approximately at halfway point
            self.assertGreater(ik1, 0.4)
            self.assertLess(ik1, 0.6)


class TestProjectionBasedIntersection(unittest.TestCase):
    """Specific tests for the projection-based intersection algorithm."""
    
    def test_intersection_symmetry(self):
        """Test that intersection is symmetric (swapping lines gives consistent results)."""
        p1 = PolyLine2D([[0, 0], [10, 0]])
        p2 = PolyLine2D([[5, -5], [5, 5]])
        
        cuts1 = p1.cut(p2)
        cuts2 = p2.cut(p1)
        
        # Both should find an intersection
        self.assertEqual(len(cuts1), 1)
        self.assertEqual(len(cuts2), 1)
        
        # The intersection point should be the same
        ik1_1, ik2_1 = cuts1[0]
        ik1_2, ik2_2 = cuts2[0]
        
        # When swapped, parameters should swap too
        self.assertAlmostEqual(ik1_1, ik2_2, places=6)
        self.assertAlmostEqual(ik2_1, ik1_2, places=6)
    
    def test_intersection_at_endpoint(self):
        """Test intersection at segment endpoint."""
        p1 = PolyLine2D([[0, 0], [10, 0]])
        p2 = PolyLine2D([[10, -5], [10, 5]])
        
        cuts = p1.cut(p2)
        
        # Should find intersection at endpoint
        self.assertEqual(len(cuts), 1)
        ik1, ik2 = cuts[0]
        
        # Should be at end of first segment
        self.assertAlmostEqual(ik1, 1.0, places=6)
    
    def test_no_intersection_parallel(self):
        """Test that parallel lines don't intersect."""
        p1 = PolyLine2D([[0, 0], [10, 0]])
        p2 = PolyLine2D([[0, 5], [10, 5]])
        
        cuts = p1.cut(p2)
        
        # Should find no intersection
        self.assertEqual(len(cuts), 0)
    
    def test_intersection_diagonal_lines(self):
        """Test intersection of diagonal lines."""
        p1 = PolyLine2D([[0, 0], [10, 10]])
        p2 = PolyLine2D([[0, 10], [10, 0]])
        
        cuts = p1.cut(p2)
        
        # Should intersect at center (5, 5)
        self.assertEqual(len(cuts), 1)
        ik1, ik2 = cuts[0]
        
        # Both should be at 0.5 (center of segments)
        self.assertAlmostEqual(ik1, 0.5, places=6)
        self.assertAlmostEqual(ik2, 0.5, places=6)


if __name__ == '__main__':
    unittest.main(verbosity=2)
