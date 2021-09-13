#FROM archlinux:base
FROM ubuntu

#RUN pacman -Syuu --noconfirm libmicrohttpd cmake gcc clang make sudo
RUN DEBIAN_FRONTEND=noninteractive apt-get update -y  && DEBIAN_FRONTEND=noninteractive apt-get upgrade -y 
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y cmake gcc g++ make sudo sqlite3 git libssl-dev

COPY . /backend
RUN mkdir /backend/build;
WORKDIR /backend/build
RUN cmake ..; make;
CMD ./backend