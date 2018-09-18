# Remove the annotation of MY_DEBUG if debug is needed
g++ -std=c++11 sampler.cpp -o sampler.so -shared -fPIC -O2 -D_GLIBCXX_USE_CXX11_ABI=0 #-D MY_DEBUG
# cp sampler.so ./Experiments/sampler.so
# cd Experiments
# python utils.py
