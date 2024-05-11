#!/bin/sh

install_packages() {
    case $1 in
        "ubuntu" | "debian")
            apt-get install $OPT tmux pv reptyr sl
            exit
            ;;
        "fedora")
            dnf install $OPT tmux pv reptyr sl
            exit
            ;;
    esac
}

if [ "x$(id -u)" != x0 ]; then
    echo "Please run it again with 'sudo'."
    exit
fi

OPT="${@}"

if [ ! -f /etc/os-release ]; then
    echo "not supported, install manually."
    exit
fi

distro=$(grep "^ID=" /etc/os-release | cut -d\= -f2 | sed -e 's/"//g')
id_like=$(grep "^ID_LIKE=" /etc/os-release | cut -d\= -f2 | sed -e 's/"//g')

install_packages "$distro"

for distro_like in $id_like; do
    install_packages "$distro_like"
    echo 0 > /proc/sys/kernel/yama/ptrace_scope
done

echo "\"$distro\" not supported, install manually."
echo

# sh -c 'while :; do printf "1 "; sleep 1; done'
