# Description
PTF tests are integration tests based on PTF library.

# Run tests

### Install docker
```
sudo dnf install docker-ce docker-ce-cli containerd.io
```

### Install libguestfs-tools
```
$ sudo dnf install libguestfs-tools-c
```

**Note:**
To run `libguestfs` tools without root privileges, you may need to workaround
the problem of
Linux kernel image not being readable by issuing:
```
$ sudo chmod +r /boot/vmlinuz-*
```

### Install qemu
```
$ git clone https://github.com/oracle/qemu qemu-orcl
$ cd qemu-orcl
$ git checkout 46bb039c31e92ae84cf7fe1f64119c1a78e0d101
$ git submodule update --init --recursive
$ ./configure --enable-multiprocess
$ make
$ make install
```

Create python environment by issuing `bash create_python_environment.sh` within `ipdk/build/storage/tests/it/ptf_tests/scripts directory`.
```
cd ipdk/build/storage/tests/it/ptf_tests/scripts
bash create_python_environment.sh
```

Using dot_env_template create `.env` config file in `ipdk/build/storage/tests/it/ptf_scripts/system_tools` directory.


All platforms for testing provided in config file must be added to ~/.ssh/known_hosts.


Execute command `bash run_ptf_tests.sh` within storage/tests/it/scripts directory.
```
cd ipdk/build/storage/tests/it/ptf_tests/scripts
bash run_ptf_tests.sh
```

**Note:**
For docker error runnning running without  root privileges, you may need to workaround the problem.
```
sudo gpasswd -a ${USER} docker && sudo systemctl restart docker
newgrp docker
```