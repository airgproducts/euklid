import sys
import os
import shutil
import multiprocessing

import mypy.stubgen


stub_index = """
from . import vector
from . import spline
from . import plane
from . import mesh

"""

if __name__ == '__main__':
    multiprocessing.freeze_support()

    stubgen_path = "."
    if len(sys.argv) > 1:
        stubgen_path = sys.argv[1]

    sys.path.append(stubgen_path)

    opts = mypy.stubgen.parse_options([])
    opts.packages.append("euklid")
    opts.output_dir = stubgen_path


    mypy.stubgen.generate_stubs(opts)


    with open(os.path.join(stubgen_path, "euklid", "__init__.pyi"), "w") as outfile:
        outfile.write(stub_index)

    shutil.move(os.path.join(stubgen_path, "euklid"), os.path.join(stubgen_path, "euklid-stubs"))