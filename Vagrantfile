# -*- mode: ruby -*-
# vi: set ft=ruby :

PRIVATE_IP_ADDRESS = "192.168.33.22"

Vagrant.configure(2) do |config|
  config.vm.box = "centos/7"
  config.vm.network "private_network", ip: PRIVATE_IP_ADDRESS
  config.vm.synced_folder ".", "/vagrant", type: "nfs"
  config.vm.provision "shell", path: "setup.sh"
end
