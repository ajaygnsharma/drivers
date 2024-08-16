#!/bin/bash
set -x
#set -e
install_git(){
#Install git
sudo apt-get -y install git
#sudo apt-get -y install git-core
#sudo update-alternatives --config git
}

##install tftpd server
install_tftp(){
sudo apt-get -y install tftpd
sudo mkdir /srv/tftp
sudo chgrp -R user1 /srv
sudo chown -R user1 /srv
sudo cp /home/user1/uclinux-dist/uClinux-dist/images/* /srv/tftp
sudo /etc/init.d/openbsd-inetd start
}



install_nfs(){
sudo apt-get -y install nfs-kernel-server portmap
sudo cp exports /etc/
sudo exportfs -ra
}


apt-get -y install sudo 
install_git;
install_tftp;
install_nfs;
sudo apt-get -y install vim
sudo apt-get -y install openssh-server
sudo apt-get -y install telnet
sudo apt-get -y install ncftp
