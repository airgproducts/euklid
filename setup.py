#! /usr/bin/python

import logging
import os
import sys
import re
import platform
import subprocess
import multiprocessing

import setuptools
from setuptools.command.build_ext import build_ext
from setuptools.command.install_lib import install_lib
from setuptools.command.install import install


version = "0.2.7"

DEBUG = False
if "--debug" in sys.argv:
    DEBUG = True
    sys.argv.remove("--debug")

CONDA_BUILD = False
if "--conda" in sys.argv:
    CONDA_BUILD = True
    sys.argv.remove("--conda")



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
            cmake_version = re.search(r'version\s*([\d.]+)', out.decode()).group(1).split(".")
            if cmake_version[0] <= '3':
                if cmake_version[0] < '3' or cmake_version[1] < '1':
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

        if CONDA_BUILD:
            cmake_args += [
                "-DWALL=off",
                "-DUSE_SUBMODULE_LIBS=off",
                "-DBUILD_SHARED_LIBS=ON",
                '-G "Ninja"'
                ]

        if platform.system() == "Windows":
            cmake_args += ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), extdir)]
            #if sys.maxsize > 2**32:
            #    cmake_args += ['-A', 'x64']
            if not CONDA_BUILD:
                build_args += ['--', f'-m:{num_cores}']
        
        if CONDA_BUILD or platform.system() != "Windows":
            cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]
            build_args += ['--', f'-j{num_cores}']

        env = os.environ.copy()
        env['CXXFLAGS'] = '{} -DVERSION_INFO=\\"{}\\"'.format(env.get('CXXFLAGS', ''),
                                                              self.distribution.get_version())
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        logging.info(f"Build dir: {self.build_temp}")
        logging.info("Cmake args: " + " ".join(cmake_args))
        logging.info("Build args: " + " ".join(build_args))
        subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd=self.build_temp, env=env)
        subprocess.check_call(['cmake', '--build', '.'] + build_args, cwd=self.build_temp)

        stubgen_path = self.build_lib
        if not os.path.exists(self.build_lib):
            stubgen_path = self.build_temp

        subprocess.check_call([sys.executable, 'stubs.py', stubgen_path])


with open("README.md") as readme_file:
    long_description = readme_file.read()

setuptools.setup(
    name='euklid',
    version=version,
    description="common vector operations [2D/3D]",
    package_data={"version": ["src/version.hpp"]},
    ext_modules=[CMakeExtension('.')],
    cmdclass={"build_ext": CMakeBuild, "install_lib": InstallStubs},
    license='MIT',
    long_description=long_description,
    author='airgproducts',
    url='http://github.com/airgproducts/euklid',
    test_suite="tests.test_suite",
    include_package_data=True
)