from distutils.core import setup, Extension

# include_dirs=['usr/include', 'usr/include/sys', 'usr/include/arpa'],library_dirs=['usr/include', 'usr/include/sys', 'usr/include/arpa'] ,libraries=['socket', 'netinet', 'netdb', 'types','stdlib','string','stdio','errno','inet']
module1 = Extension('Spiffy', sources = ['spiffy_interface.c'])
setup (name = 'Spiffy',
       version = '1.0', 
       description = 'This is a demo package', 
       ext_modules = [module1])