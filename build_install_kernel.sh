sudo apt-get install git fakeroot build-essential ncurses-dev xz-utils libssl-dev bc flex libelf-dev bison
cd linux
# cp /boot/config-$(uname -r) .config
make menuconfig
CFLAGS="-fcf-protection=none" make -j64
sudo make modules_install
sudo make install
sudo make headers_install

