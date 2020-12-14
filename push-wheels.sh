#!/bin/sh

# Do not leak secrets:
set +x +e

# ARTIFACT_URL_BASE=https://some.cloud.com/public.php/webdav
# ARTIFACT_USER=share_token

if [ -n "${ARTIFACT_URL_BASE}" ]; then
  export ARTIFACT_FOLDER=$(printf '%04d' ${DRONE_BUILD_NUMBER})__${DRONE_TAG:-${DRONE_BRANCH}}__${DRONE_COMMIT:0:7}
  export ARTIFACT_URL="${ARTIFACT_URL_BASE}/${ARTIFACT_FOLDER}"
  export ARTIFACT_CURL_OPTS="-u '${ARTIFACT_USER}:${ARTIFACT_PASS}' -H 'X-Requested-With: XMLHttpRequest'"

  if [ "404" = "$(eval curl -o /dev/null -s -w '%{http_code}' ${ARTIFACT_CURL_OPTS} -X PROPFIND ${ARTIFACT_URL})" ]; then
    echo "Creating folder ${ARTIFACT_FOLDER} ..."
    eval curl ${ARTIFACT_CURL_OPTS} -X MKCOL ${ARTIFACT_URL}
  fi

  cd wheelhouse
  for whl in *.whl; do
    echo "Pushing artifact ${whl} ..."
    eval curl -T ${whl} ${ARTIFACT_CURL_OPTS} ${ARTIFACT_URL}/${whl}
  done
  cd ..

  cd dist
  for tar in *.tar.gz; do
    # should only ever be one file though
    echo "Pushing source tarball ${tar} ..."
    eval curl -T ${tar} ${ARTIFACT_CURL_OPTS} ${ARTIFACT_URL}/${tar}
  done
  cd ..
fi
