'''
Oscup: Open Source Custom Uart Protocol
This Software was release under: GPL-3.0 License
Copyright � 2021 Daniel Rossi & Riccardo Salami
Version: ALPHA 1.2.0
'''

from setuptools import setup, find_packages

setup(
    name='pyOscup', 
    version='1.2.0', 
    author="Daniel Rossi & Riccardo Salami",
    description="pyOscup is a computer interface to communicate with Oscup protocol",
    url="https://github.com/ProjectoOfficial/Oscup",
    packages=find_packages(),
    classifiers=[

        "Programming Language :: Python :: 3",

        "License :: OSI Approved :: GPL-3.0 License",

        "Operating System :: OS Independent",

    ],

    python_requires='>=3.8.8',)