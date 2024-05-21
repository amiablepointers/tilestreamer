# Sections

* **frameserver** This section defines the output stream configuration.
* **stream_.*** Each stream is defined in a section starting with "stream_".

# Sample configuration

```
[frameserver]
w=1920
h=1080
fps=10
font=/usr/share/fonts/truetype/freefont/FreeSans.ttf

[stream_teststream_1]
uri=/home/test/Videos/video1.mp4
w=480
h=270
x=430
y=0
textoverlay=Stream1
zindex=1

[stream_teststream_2]
uri=rtsp://rtspserver/live
w=480
h=270
x=430
y=0
textoverlay=Stream2
zindex=2

[stream_teststream_3]
uri=http://jpegstreamserver/live
w=480
h=270
x=100
y=170
textoverlay=Stream3
zindex=3
```
# Running the example with http output

```
tilestreamer -p 8080 -f my_config.ini
```
Open a web browser and navigate to http://localhost:8080/stream.
You will see a grid view that displays the two streams specified in the tilestreamer configuration file.

# Running the example with V4L output
 
The following is an example of how to use tilestreamer together with [&mu;Streamer](https://github.com/pikvm/ustreamer) to generate a grid display accessible through a web browser:

Create the V4L device with [v4l2loopback](https://github.com/umlaeute/v4l2loopback):
```
/sbin/modprobe v4l2loopback video_nr=7
```
Start tilestreamer by running the following command:

```
tilestreamer -f my_config.ini -d /dev/video7
```

Start &mu;Streamer by running the following command:

```
ustreamer -d /dev/video7 -f 10 -r 1920x1080 -s localhost > -p 8080
```

Browse http://localhost:8080/stream to see &mu;Streamer output.
