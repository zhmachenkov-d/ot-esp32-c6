ARG DOCKER_TAG=release-v5.4
FROM espressif/idf:${DOCKER_TAG}

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8

RUN apt-get update -y && apt-get install -y udev bash-completion && rm -rf /var/lib/apt/lists/*

ENV PATH="/root/.local/bin:$PATH"

RUN echo "source /opt/esp/idf/export.sh > /dev/null 2>&1" >> ~/.bashrc

ENTRYPOINT ["/opt/esp/entrypoint.sh"]
CMD ["/bin/bash", "-c"]
