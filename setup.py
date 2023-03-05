import pathlib
from glob import glob
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension

MODULE_NAME = 'optrie'
__version__ = '0.1.0'

ext_modules = [
    Pybind11Extension(
        MODULE_NAME,
        glob('src/*.cpp'),
        include_dirs=['include'],
        cxx_std=11,
    ),
]

setup(
    name=MODULE_NAME,
    version=__version__,
    author="Junfu Qian",
    author_email="qianjunfu42@163.com",
    url="https://github.com/qjf42/optrie",
    description="A simple template matcher",
    long_description="",
    ext_modules=ext_modules,
    extras_require={},
    zip_safe=False,
    python_requires=">=3.5",
)