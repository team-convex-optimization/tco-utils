#!/bin/bash
# TODO: Use 'pushd' and 'popd' instead of 'cd'

# Don't want a glob pattern that matched nothing to remain unchanged
shopt -s nullglob
shopt -s globstar

# Github person access token (note that the default value is not a real access token)
token=1164e2e2fa4f631e26d01de9d165c0629b8f9288
repos=(tco-actuationd tco-controld tco-utils)

# Colors
COLOR_GREEN="\033[0;32m"
COLOR_RED="\033[0;31m"
COLOR_CYAN="\033[0;36m"
COLOR_CLEAR="\033[0m"

repo_folder=repos
bins_folder=bin

pull_all() 
{
    echo -e "\n${COLOR_GREEN}Pulling All Repos${COLOR_CLEAR}\n"
    mkdir $repo_folder
    cd $repo_folder
    for repo in "${repos[@]}"
    do
        git clone --recurse-submodules https://github.com/team-convex-optimization/${repo}.git
        if [ $? -ne 0 ]; then
            cd $repo
            # Force pull (ignore all local changes)
            git reset --hard
            git pull
            cd ..
        fi
    done
    cd ..
}

# Handle all repos that are not authored by TCO
download_repo_other()
{
    repo=$1
    if [ "$repo" = "ws-server" ]
    then
        curl -L https://api.github.com/repos/Theldus/wsServer/tarball > ${repo}.tar.gz
        tar -zxvf ${repo}.tar.gz
        rm -rf ${repo}.tar.gz
        repo_underscores=$(echo $repo | tr '-' '_')
        mv Theldus-wsServer-* ${repo_underscores}
        return
    fi
    echo -e "\n${COLOR_RED}Failed to download '$repo'${COLOR_CLEAR}\n"
}

download_repo_tco()
{
    local repo=$1
    echo -e "\n${COLOR_GREEN}Downloading '$repo'${COLOR_CLEAR}\n"
    curl -H "Authorization: token ${token}" -L https://api.github.com/repos/team-convex-optimization/${repo}/tarball > ${repo}.tar.gz
    tar -zxvf ${repo}.tar.gz
    rm -rf ${repo}.tar.gz
    mv team-convex-optimization-${repo}-* ${repo}
    local repo_underscores=$(echo $repo | tr '-' '_')
    mv $repo $repo_underscores
    cd ${repo_underscores}
    echo -e "\n${COLOR_GREEN}Downloading submodules for '$repo'${COLOR_CLEAR}\n"
    for lib in **/lib/*
    do
        local ret_path=$(pwd)
        cd $lib
        cd ..
        local submodule=$(basename $lib)
        rm -rf $submodule
        local submodule=$(echo $submodule | tr '_' '-')
        echo -e "\n${COLOR_CYAN}Downloading submodule '$submodule' for '$repo'${COLOR_CLEAR}\n"
        if [ "$(echo $submodule | cut -c1-3)" != "tco" ]
        then
            download_repo_other $submodule
        else
            download_repo_tco $submodule
        fi
        cd $ret_path
    done
    cd ..
}

download_all() {
    echo -e "\n${COLOR_GREEN}Downloading All Repos${COLOR_CLEAR}\n"
    mkdir $repo_folder
    # Delete all repos before redownloading
    if [ $? -ne 0 ]; then
        rm -rf $repo_folder
        mkdir $repo_folder
    fi
    cd $repo_folder
    for repo in "${repos[@]}"
    do
        download_repo_tco $repo
    done
    cd ..
}

build_and_move_bin_all() {
    echo -e "\n${COLOR_GREEN}Building All Repos${COLOR_CLEAR}\n"
    rm -rf $bins_folder
    mkdir $bins_folder
    cd $repo_folder
    for bash_script in ./**/*.sh
    do
        chmod +x $bash_script
    done
    for build_script_path in ./**/build.sh
    do
        build_script_parent_path=${build_script_path::-9}
        local ret_path=$(pwd)
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
    local timestamp=$(date "+%Y_%m_%d_%H_%M_%S")
    tar_filename="tco_release_$timestamp.tar"
    cd ./bin
    tar --create -f ../$tar_filename *
    cd ..
}

mkdir tco_release
cd tco_release

download_all
build_and_move_bin_all
make_release
