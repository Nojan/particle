FROM ubuntu:17.10

WORKDIR /workdir
RUN apt -y update
RUN apt -y install gcc cmake libsdl2-dev

CMD ./build.sh
