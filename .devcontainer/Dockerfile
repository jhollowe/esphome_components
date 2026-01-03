ARG VARIANT="3.13"
# ARG VARIANT="stable"
FROM mcr.microsoft.com/vscode/devcontainers/python:${VARIANT}
# FROM ghcr.io/esphome/esphome:${VARIANT}

RUN apt-get update \
  && apt-get install -y --no-install-recommends \
  build-essential \
  git \
  less \
  nano \
  wget \
  && apt purge -y --auto-remove \
  && rm -rf \
      /tmp/* \
      /var/{cache,log}/* \
      /var/lib/apt/lists/* \
      /usr/src/*

COPY requirements.txt /tmp/pip-tmp/
RUN pip3 --disable-pip-version-check --no-cache-dir install -r /tmp/pip-tmp/requirements.txt  \
    && rm -rf /tmp/pip-tmp
