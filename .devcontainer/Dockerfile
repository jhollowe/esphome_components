ARG VARIANT="3"
# ARG VARIANT="stable"
FROM mcr.microsoft.com/vscode/devcontainers/python:${VARIANT}
# FROM ghcr.io/esphome/esphome:${VARIANT}

RUN apt-get update \
  && apt-get install -y \
  nano \
  git \
  less \
  wget \
  && apt-get clean -y && rm -rf /var/lib/apt/lists/*

COPY requirements.txt /tmp/pip-tmp/
RUN pip3 --disable-pip-version-check --no-cache-dir install -r /tmp/pip-tmp/requirements.txt  \
    && rm -rf /tmp/pip-tmp
