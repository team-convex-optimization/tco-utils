#!/bin/bash
# Don't want a glob pattern that matched nothing to remain unchanged
shopt -s nullglob
shopt -s globstar

# Colors
COLOR_GREEN="\033[0;32m"
COLOR_CLEAR="\033[0m"

repo_folder=repos
bins_folder=bin

pull_all() {
    echo -e "\n${COLOR_GREEN}Pulling All Repos${COLOR_CLEAR}\n"
    mkdir $repo_folder
    cd $repo_folder
    for repo in "tco-actuationd" "tco-controld" "tco-sensord" "tco-utils"
    do
        echo $repo
        git clone --recurse-submodules https://github.com/team-convex-optimization/${repo}.git
        if [ $? -ne 0 ]; then
            cd $repo
            git pull
            cd ..
        fi
    done
    cd ..
}

build_and_move_bin_all() {
    echo -e "\n${COLOR_GREEN}Building All Repos${COLOR_CLEAR}\n"
    mkdir $bins_folder
    cd $repo_folder
    for bash_script in ./**/*.sh
    do
        chmod +x $bash_script
    done
    for build_script_path in ./**/build.sh
    do
        build_script_parent_path=${build_script_path::-9}
        ret_path=$(pwd)
        cd $build_script_parent_path
        pwd
        ./build.sh
        cd $ret_path
    done
    echo -e "\n${COLOR_GREEN}Moving all built binaries to '${bins_folder}'${COLOR_CLEAR}\n"
    for built_file in ./**/*.bin
    do
        echo $built_file
        mv -f -t ../$bins_folder $built_file
    done
    cd ..
}

make_release() {
    echo -e "\n${COLOR_GREEN}Making a tar with all built binaries${COLOR_CLEAR}\n"
    timestamp=$(date "+%Y_%m_%d_%H_%M_%S")
    tar_filename="tco_release_$timestamp.tar"
    cd ./bin
    tar --create -f ../$tar_filename *
    cd ..
}

mkdir tco_release
cd tco_release

pull_all
build_and_move_bin_all
make_release
