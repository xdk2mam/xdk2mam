#!/bin/bash
# Clone Cmake Repo, run Cmake + Make + Move bin + Remove Entangled files
git clone -b master https://github.com/iot2tangle/cmake-mam.git
cd cmake-mam
cmake .  
make
mv mam/send-msg ../send-msg
mv mam/recv ../recv
cd ..
rm -rf cmake-mam
exec bash
