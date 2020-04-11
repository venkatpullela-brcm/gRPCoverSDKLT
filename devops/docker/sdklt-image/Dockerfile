FROM base-container:1.0

# define variables
ARG libyaml_dest_folder='/libyaml'
ARG sdklt_dest_folder='/sdklt'
ARG target_platform=native_thsim
ENV DEBIAN_FRONTEND=noninteractive

# Install libyaml-dev, python2.7, pip
RUN apt-get update && \
    apt-get -y upgrade &&\
    apt-get install -y libyaml-dev &&\
    apt-get install -y python-dev python python-pip

# install pyyaml
RUN python -m pip install pyyaml

RUN git clone https://github.com/yaml/libyaml.git ${libyaml_dest_folder}
ENV YAML=${libyaml_dest_folder}
ENV YAML_LIBDIR=${YAML}/src/.libs
ENV SDK=${sdklt_dest_folder}/src

RUN git clone https://github.com/Broadcom-Network-Switching-Software/SDKLT.git ${sdklt_dest_folder} &&\
    cd ${sdklt_dest_folder}/src/appl/demo &&\
    make -s TARGET_PLATFORM=${target_platform}

WORKDIR $SDK/appl/demo/build/${target_platform}

# expose the port for outside communication
EXPOSE 50051

#ADD scripts/entrypoint.sh /usr/local/bin/entrypoint.sh
#RUN chmod +x /usr/local/bin/entrypoint.sh
#ENTRYPOINT [ "entrypoint.sh" ]