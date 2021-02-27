import unittest
def test_suite():
    test_loader = unittest.TestLoader()
    test_suite = test_loader.discover('tests', pattern='test_*.py')
    return test_suite

if __name__ == "__main__":
    tests = test_suite()
    test_results = unittest.TextTestRunner(verbosity=int(1)).run(tests)