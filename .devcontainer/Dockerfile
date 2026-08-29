ARG DOCKER_TAG=release-v5.4
FROM espressif/idf:${DOCKER_TAG}

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8

RUN apt-get update -y && apt-get install -y udev bash-completion sudo \
    && rm -rf /var/lib/apt/lists/* \
    && if id -u ubuntu >/dev/null 2>&1; then \
         usermod -l vscode ubuntu \
         && groupmod -n vscode ubuntu \
         && usermod -d /home/vscode -m vscode; \
       elif ! id -u vscode >/dev/null 2>&1; then \
         groupadd --gid 1000 vscode \
         && useradd -s /bin/bash --uid 1000 --gid 1000 -m vscode; \
       fi \
    && usermod -aG dialout,plugdev,sudo vscode \
    && echo "vscode ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/vscode \
    && chmod 0440 /etc/sudoers.d/vscode

USER vscode
ENV HOME=/home/vscode
ENV PATH="/home/vscode/.local/bin:$PATH"

RUN echo "source /opt/esp/idf/export.sh > /dev/null 2>&1" >> ~/.bashrc

ENTRYPOINT ["/opt/esp/entrypoint.sh"]
CMD ["/bin/bash", "-c"]
