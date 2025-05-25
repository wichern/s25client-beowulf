FROM mlpack/mlpack:latest
USER root

RUN apt-get update && apt-get install -y g++ cmake git libboost1.74-all-dev libminiupnpc-dev libsdl2-dev libsdl2-mixer-dev libcurl4-openssl-dev libbz2-dev liblua5.2-dev gettext \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

CMD ["/bin/bash"]
