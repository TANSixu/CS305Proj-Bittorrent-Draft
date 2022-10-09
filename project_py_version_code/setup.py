from distutils.core import setup, Extension

module1 = Extension('Spiffy', sources = ['spiffy_interface.c'])
setup (name = 'Spiffy',
       version = '1.0', 
       description = 'This is a demo package', 
       ext_modules = [module1])