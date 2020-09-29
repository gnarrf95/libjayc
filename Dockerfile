# Based on ubuntu
FROM ubuntu:latest

# Copy repository into root home
WORKDIR /root/libjayc
COPY . .

# Dependencies
RUN apt-get update && apt-get -y upgrade && apt-get -y install libssl-dev build-essential make rsyslog

# Compile and install
RUN make all && make install PREFIX=/usr/

# Using port 1234
EXPOSE 1234

# Execute jsys_test daemon with any IP, port 1234, syslog logging and SHA256 hash
CMD ["jsys_test", "--ip", "0.0.0.0", "--port", "1234", "--syslog", "daemon", "--hash", "2"]