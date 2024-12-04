#!/usr/bin/env bash

set -e

podman run \
       --interactive \
       --detach \
       --volume "${PWD}":"/root/scarab" \
       --volume "${XDG_CONFIG_HOME}/git":/root/.config/git:ro \
       --volume $(gpgconf --list-dirs agent-extra-socket):/root/.gnupg/S.gpg-agent \
       --volume $(gpgconf --list-dirs agent-ssh-socket):/root/.gnupg/S.gpg-agent.ssh \
       --volume /gnu/store:/gnu/store \
       --env SSH_AUTH_SOCK=/root/.gnupg/S.gpg-agent.ssh \
       --name scarab \
       localhost/scarab_new
