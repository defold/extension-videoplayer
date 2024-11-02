# Disclaimer

This is an example extension, with no sound support. Although we aim to provide good example functionality in this example, we cannot guarantee the quality/stability at all times.
Please regard it as just that, an example, and don't rely on this as a dependency for your production code.

Alternative videoplayers:

* Native videoplayer (Android/iOS) - [extension-videoplayer-native](https://github.com/defold/extension-videoplayer-native)
* MPEG1 videoplayer - [extension-videoplayer-mpeg](https://github.com/defold/extension-videoplayer-mpeg)


# extension-videoplayer

Example of creating a videoplayer (.webm) through native extensions.


# How to build the libraries

[Instructions here](videoplayer/utils/README.md)


# FAQ

## How do I use this extension?

Add the package link (https://github.com/defold/extension-videoplayer/archive/master.zip)
to the project setting `project.dependencies`, and you should be good to go.

See the [manual](http://www.defold.com/manuals/libraries/) for further info.


# Lua API

## videoplayer.open(videoresource)

Opens a video resource, and returns a handle to the instance.

    local videoresource = resource.load("/videos/big_buck_bunny.webm")
    self.video = videoplayer.open(videoresource)

## videoplayer.get_info(video)

Gets the info about the video:

* width - in pixels
* height - in pixels
* bytes_per_pixel
* frame - current frame
* time - current time in seconds
* eos - true if end of stream was reached

## videoplayer.get_frame(video)

Gets the video buffer. This is allocated when opening the video.
The buffer has one stream, with signature `{hash("rgb"), buffer.VALUE_TYPE_UINT8, 3}`.

    self.videoframe = videoplayer.get_frame(self.video)

## videoplayer.update(video, dt)

Updates the video with a delta time


# Example

*[main.script](main/main.script):*

    function init(self)
        local logosize = 128
        local screen_width = sys.get_config("display.width", 600)
        local screen_height = sys.get_config("display.height", 800)
        local scale_width = screen_width / logosize
        local scale_height = screen_height / logosize

        go.set("#sprite", "scale", vmath.vector3(scale_width, scale_height, 1) )

        if videoplayer ~= nil then
            local videoresource = resource.load("/videos/big_buck_bunny.webm")
            self.video = videoplayer.open(videoresource)
            self.videoinfo = videoplayer.get_info(self.video)
            self.videoheader = { width=self.videoinfo.width, height=self.videoinfo.height, type=resource.TEXTURE_TYPE_2D, format=resource.TEXTURE_FORMAT_RGB, num_mip_maps=1 }
            self.videoframe = videoplayer.get_frame(self.video)
        else
            print("Could not initialize videoplayer")
        end
    end

    function update(self, dt)
        if videoplayer ~= nil then
            videoplayer.update(self.video, dt)
            local path = go.get("#sprite", "texture0")
            resource.set_texture(path, self.videoheader, self.videoframe)
        end
    end

