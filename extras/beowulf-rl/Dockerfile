FROM mlpack/mlpack:latest

ARG USER_NAME=mlpack
ARG USER_UID
ARG USER_GID

RUN echo "USER_NAME=${USER_NAME}, USER_UID=${USER_UID}, USER_GID=${USER_GID}" \
    && if [ -z "$USER_UID" ] || [ -z "$USER_GID" ] || [ -z "$USER_NAME" ]; then \
        echo "One or more required build args are missing!" >&2; exit 1; \
    fi

USER root

# Install dependencies
RUN apt-get update && apt-get install -y \
  g++ gdb cmake git \
  libboost1.74-all-dev libminiupnpc-dev \
  libsdl2-dev libsdl2-mixer-dev libcurl4-openssl-dev \
  libbz2-dev liblua5.2-dev gettext bash-completion nano locales linux-perf linux-base \
  sudo \
  && apt-get clean && rm -rf /var/lib/apt/lists/*

RUN locale-gen en_US.UTF-8 \
  && update-locale LANG=en_US.UTF-8
ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8
  
RUN set -eux; \
    echo "Creating user: ${USER_NAME} with UID=${USER_UID} and GID=${USER_GID}"; \
    if [ -z "${USER_NAME}" ] || [ -z "${USER_UID}" ] || [ -z "${USER_GID}" ]; then \
      echo "Error: Missing one or more user build arguments!" >&2; exit 1; \
    fi; \
    EXISTING_GROUP=$(getent group "${USER_GID}" | cut -d: -f1 || true); \
    if [ -z "$EXISTING_GROUP" ]; then \
      groupadd --gid "${USER_GID}" "${USER_NAME}"; \
      GROUP_NAME="${USER_NAME}"; \
    else \
      echo "Group with GID ${USER_GID} already exists: $EXISTING_GROUP"; \
      GROUP_NAME="$EXISTING_GROUP"; \
    fi; \
    EXISTING_USER=$(getent passwd "${USER_UID}" | cut -d: -f1 || true); \
    if [ -z "$EXISTING_USER" ]; then \
      useradd --uid "${USER_UID}" --gid "${USER_GID}" -m "${USER_NAME}"; \
    else \
      echo "User with UID ${USER_UID} already exists: $EXISTING_USER"; \
    fi; \
    echo "${USER_NAME} ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers; \
    id "${USER_NAME}"

USER ${USER_NAME}
WORKDIR /home/${USER_NAME}

CMD ["/bin/bash"]
