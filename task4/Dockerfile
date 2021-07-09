FROM local:opencv3.2.0
COPY source.cpp /usr/myprog/
COPY video.MOV /usr/myprog/
WORKDIR /usr/myprog/
RUN g++ -o myprog source.cpp `pkg-config opencv --cflags --libs` && \
    echo /usr/local/lib>/etc/ld.so.conf.d/opencv.conf && \
    ldconfig -v
CMD ./myprog