# rocks
Machine learning project to identify rock samples by laser ablation spectrum.


# Dependencies.
libsqlite3-dev  
libdlib-dev - no, compile it from source with cuda support: 1. nvidia-cuda-dev, nvidia-smi 2.(need to register) https://developer.nvidia.com/rdp/cudnn-download
      3. https://github.com/davisking/dlib     CC=gcc-7 CXX=g++-7 cmake .. -DUSE_AVX_INSTRUCTIONS=1  
https://github.com/ashokpant/accuracy-evaluation-cpp  
libqwt-qt5-dev  
https://github.com/bjoern-andres/random-forest.git


# Introduction to the domain
https://physics.nist.gov/cgi-bin/ASD/lines1.pl?compositionCu&spectra=Cu&low_w=180&limits_type=0&upp_w=961&show_av=2&unit=1&resolution=1&temp=1&eden=1e17&libs=1  
https://www.nist.gov/pml/atomic-spectroscopy-compendium-basic-ideas-notation-data-and-formulas/atomic-spectroscopy  
https://www.nist.gov/pml/atomic-spectra-database  
