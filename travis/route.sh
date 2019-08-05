#!/bin/sh
__CURRENT__=`pwd`
__DIR__=$(cd "$(dirname "$0")";pwd)

export DOCKER_COMPOSE_VERSION="1.21.0"
[ -z "${TRAVIS_BRANCH}" ] && export TRAVIS_BRANCH="master"
[ -z "${TRAVIS_BUILD_DIR}" ] && export TRAVIS_BUILD_DIR=$(cd "$(dirname "$0")";cd ../;pwd)
[ -z "${PHP_VERSION_ID}" ] && export PHP_VERSION_ID=`php -r "echo PHP_VERSION_ID;"`
if [ ${PHP_VERSION_ID} -lt 70400 ]; then
    export PHP_VERSION="`php -r "echo PHP_MAJOR_VERSION;"`.`php -r "echo PHP_MINOR_VERSION;"`"
else
    export PHP_VERSION="rc"
fi
if [ "${TRAVIS_BRANCH}" = "alpine" ]; then
    export PHP_VERSION="${PHP_VERSION}-alpine"
fi

echo "\n🗻 With PHP version ${PHP_VERSION} on ${TRAVIS_BRANCH} branch"