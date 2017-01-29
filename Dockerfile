FROM ubuntu:16.04

WORKDIR /workdir
RUN apt -y update
RUN apt -y install gcc premake4 libsdl2-dev

COPY . .
CMD ./build.sh
