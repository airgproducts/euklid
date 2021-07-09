#! /usr/bin/python

import logging
import os
import sys
import re
import platform
import subprocess
import multiprocessing

from distutils.version import LooseVersion
from distutils.core import setup
import setuptools
from setuptools.command.build_ext import build_ext
from setuptools.command.install_lib import install_lib
from setuptools.command.install import install

stub_index = """
from . import vector
from . import spline
from . import plane
from . import mesh

"""


DEBUG = False
if "--debug" in sys.argv:
    DEBUG = True
    sys.argv.remove("--debug")

class CMakeExtension(setuptools.Extension):
    def __init__(self, name, sourcedir=''):
        super().__init__(name, [])
        self.sourcedir = os.path.abspath(sourcedir)
        #self.include_dirs = ['euklid-stubs']

class InstallStubs(install_lib):
    def run(self):
        super().run()


class CMakeBuild(build_ext):
    def run(self):
        print("extensions: ", self.extensions)
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError("CMake must be installed to build the following extensions: " +
                               ", ".join(e.name for e in self.extensions))

        if platform.system() == "Windows":
            cmake_version = LooseVersion(re.search(r'version\s*([\d.]+)', out.decode()).group(1))
            if cmake_version < '3.1.0':
                raise RuntimeError("CMake >= 3.1.0 is required on Windows")

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        
        num_cores = multiprocessing.cpu_count()

        cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
                      '-DPYTHON_EXECUTABLE=' + sys.executable]

        cfg = 'Debug' if DEBUG else 'Release'

        cmake_args.append(f"-DCMAKE_BUILD_TYPE={cfg}")
        build_args = ['--config', cfg]

        if platform.system() == "Windows":
            cmake_args += ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), extdir)]
            #if sys.maxsize > 2**32:
            #    cmake_args += ['-A', 'x64']
            build_args += ['--', '/m']
        else:
            cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]
            build_args += ['--', f'-j{num_cores}']

        env = os.environ.copy()
        env['CXXFLAGS'] = '{} -DVERSION_INFO=\\"{}\\"'.format(env.get('CXXFLAGS', ''),
                                                              self.distribution.get_version())
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        logging.info(f"Build dir: {self.build_temp}")
        subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd=self.build_temp, env=env)
        subprocess.check_call(['cmake', '--build', '.'] + build_args, cwd=self.build_temp)

        stubgen_path = self.build_lib
        if not os.path.exists(self.build_lib):
            stubgen_path = self.build_temp
        
        import mypy.stubgen
        opts = mypy.stubgen.parse_options({})
        opts.packages.append("euklid")
        opts.output_dir = stubgen_path

        mypy.stubgen.generate_stubs(opts)
        subprocess.check_call(['mv', 'euklid', 'euklid-stubs'], cwd=stubgen_path)

        with open(os.path.join(stubgen_path, "euklid-stubs", "__init__.pyi"), "w") as outfile:
            outfile.write(stub_index)
        

version = "-"
with open("src/version.hpp") as version_file:
    #print(version_file.read())
    version = re.findall(r"version\s=\s['\"]([0-9\._]+)['\"]", version_file.read())[0]

with open("README.md") as readme_file:
    long_description = readme_file.read()

def find_stub_files(name: str):
    result = []
    for root, dirs, files in os.walk(name):
        for file in files:
            if file.endswith('.pyi'):
                if os.path.sep in root:
                    sub_root = root.split(os.path.sep, 1)[-1]
                    file = os.path.join(sub_root, file)
                result.append(file)
    return result

setup(
    name='euklid',
    version=version,
    description="common vector operations [2D/3D]",
    ext_modules=[CMakeExtension('.')],
    cmdclass={"build_ext": CMakeBuild, "install_lib": InstallStubs},
    license='MIT',
    long_description=long_description,
    install_requires=['mypy'],
    requires=["mypy"],
    #packages=['euklid-stubs'],
    #package_data={'euklid-stubs': find_stub_files('euklid-stubs')},
    author='airgproducts',
    url='http://github.com/airgproducts/euklid',
    test_suite="tests.test_suite",
    include_package_data=True
)