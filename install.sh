echo "Removing old cadical, old MaxPre, libleximax.a and cbc..." && \
rm -rf cadical && rm -rf maxpre && rm -rf lib && rm -rf bin && rm -rf cbc && rm -rf mccs-1.1/objs \
echo "Setting up CaDiCaL..." && \
git clone https://github.com/arminbiere/cadical.git && \
cd cadical && ./configure --competition && make && \
cd .. && \
echo "Setting up MaxPre..." && \
git clone https://bitbucket.org/coreo-group/maxpre2.git maxpre && cd maxpre &&\
make lib && \
echo "Compiling leximaxIST library and command line tool..." && \
cd .. && \
mkdir -v lib && mkdir -v bin && \
cd src && make clean release && \
echo "Compiling packup..." && \
cd ../old_packup/ && make clean release packup2mcnf && \
echo "Compiling mccs with ILP solver Cbc..." && \
echo "Installing Cbc..." && \
cd .. && mkdir -v cbc && cd cbc && \
wget https://raw.githubusercontent.com/coin-or/coinbrew/master/coinbrew && \
chmod u+x coinbrew && \
./coinbrew fetch Cbc@2.10.7 && \
./coinbrew build Cbc && \
echo "Compiling mccs..." && \
mkdir -v ../mccs-1.1/objs && \
cd ../mccs-1.1 && make clean mccs-static && \
echo "All Done"
