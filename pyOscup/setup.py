'''
Oscup: Open Source Custom Uart Protocol
This Software was release under: GPL-3.0 License
Copyright � 2022 Daniel Rossi 
Version: 1.2.4
'''

from setuptools import setup, find_packages

setup(
    name='pyOscup', 
    version='1.2.4', 
    author="Daniel Rossi",
    description="pyOscup is a computer interface to communicate with Oscup protocol",
    url="https://github.com/ProjectoOfficial/Oscup",
    packages=find_packages(),
    classifiers=[

        "Programming Language :: Python :: 3",

        "License :: OSI Approved :: GPL-3.0 License",

        "Operating System :: OS Independent",

    ],

    python_requires='>=3.8.8',)