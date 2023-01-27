Machine learning project to identify rock samples by laser ablation spectrum.
The SVM performs best at 97% accuracy.


### Dependencies.
Compulsory:  
`libdlib-dev` - but compile it from source with cuda support:  
1. nvidia-cuda-dev, nvidia-smi
2. (need to register) https://developer.nvidia.com/rdp/cudnn-download
3. https://github.com/davisking/dlib
4. CC=gcc CXX=g++ cmake .. -DUSE_AVX_INSTRUCTIONS=1

Optional:

    libsqlite3-dev
    libqwt-qt5-dev
    https://github.com/ashokpant/accuracy-evaluation-cpp
    https://www.shark-ml.org/
    texlive-bibtexextra  # for the docs


### Introduction to the domain

- [graph of an example datapoint](https://physics.nist.gov/cgi-bin/ASD/lines1.pl?compositionCu&spectra=Cu&low_w=180&limits_type=0&upp_w=961&show_av=2&unit=1&resolution=1&temp=1&eden=1e17&libs=1)
- [some physics background](https://www.nist.gov/pml/atomic-spectroscopy-compendium-basic-ideas-notation-data-and-formulas/atomic-spectroscopy)
- [a dataset with lots of documentation](https://www.nist.gov/pml/atomic-spectra-database)
