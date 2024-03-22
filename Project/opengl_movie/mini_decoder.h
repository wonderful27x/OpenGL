#ifndef MINI_DECODER_H
#define MINI_DECODER_H

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#ifdef __cplusplus
}
#endif

class IMiniDecoder
{
public:
    virtual AVFrame *DecodeFrame() = 0;
    virtual ~IMiniDecoder() = default;
};

class MiniDecoder : public IMiniDecoder
{
public:
    enum Type
    {
        AUDIO,
        VIDEO
    };

    MiniDecoder(const std::string &url) : _url(url) {}

    ~MiniDecoder()
    {
        if(_dec_ctx) avcodec_free_context(&_dec_ctx);
        if(_fmt_ctx) avformat_close_input(&_fmt_ctx);
        if(_frame) av_frame_free(&_frame);
    }

    int Init()
    {
        _media_type = (DecoderType() == VIDEO) ? AVMEDIA_TYPE_VIDEO : AVMEDIA_TYPE_AUDIO;

        if((_frame = av_frame_alloc()) == nullptr) return CODE(1);

        /* open url */
        _fmt_ctx = avformat_alloc_context();
        if(avformat_open_input(&_fmt_ctx, _url.c_str(), NULL, NULL) < 0) return CODE(2);
        if(avformat_find_stream_info(_fmt_ctx, NULL) < 0) return CODE(3);

        /* select video stream */
        const AVCodec *dec = nullptr;
        _stream_index = av_find_best_stream(_fmt_ctx, _media_type, -1, -1, &dec, 0);
        if(_stream_index < 0) return CODE(4);

        /* create and open codec */
        if((_dec_ctx = avcodec_alloc_context3(dec)) == nullptr) return CODE(5);
        avcodec_parameters_to_context(_dec_ctx, _fmt_ctx->streams[_stream_index]->codecpar);
        if (avcodec_open2(_dec_ctx, dec, NULL) < 0) return CODE(6);

        _init_ok = true;

        return CODE(0);
    }

    virtual AVFrame *DecodeFrame() override
    {
        if(_frame == nullptr)
        {
            if((_frame = av_frame_alloc()) == nullptr)
            {
                av_log(NULL, AV_LOG_ERROR, "Failed to alloc AVFrame!\n");
                return nullptr;
            }
        }

        AVPacket packet;
        int ret;
        bool ok = false;
        while (true) {
            if(avcodec_receive_frame(_dec_ctx, _frame) == 0)
            {
                _frame->pts = _frame->best_effort_timestamp;
                ok = true;
                break;
            }

            if ((ret = av_read_frame(_fmt_ctx, &packet)) < 0)
            {
                av_log(NULL, AV_LOG_ERROR, "Failed to read AVPacket, code: %d\n", ret);
                break;
            }

            if (packet.stream_index == _stream_index)
            {
                ret = avcodec_send_packet(_dec_ctx, &packet);
                if(ret == AVERROR(EAGAIN))
                {
                    continue;
                }
                else if(ret < 0)
                {
                    av_log(NULL, AV_LOG_ERROR, "Failed to send AVPacket, code: %d\n", ret);
                    break;
                }
                while (ret >= 0)
                {
                    ret = avcodec_receive_frame(_dec_ctx, _frame);
                    if (ret == AVERROR(EAGAIN)) {
                        break;
                    }
                    else if (ret < 0)
                    {
                        av_log(NULL, AV_LOG_ERROR, "Failed to receive AVFrame, code: %d\n", ret);
                        goto end;
                    }

                    _frame->pts = _frame->best_effort_timestamp;
                    ok = true;
                    goto end;
                }
            }

            av_packet_unref(&packet);
        }
end:
        av_packet_unref(&packet);

        /* av_log(NULL, AV_LOG_ERROR, "MiniDecoder::DecodeFrame, ok = %d\n", ok); */
        return ok ? _frame : nullptr;
    }

protected:
    const std::string _url;

    int _stream_index = -1;
    AVFormatContext *_fmt_ctx = nullptr;
    AVCodecContext *_dec_ctx = nullptr;
    AVFrame *_frame = nullptr;
    AVMediaType _media_type;

    bool Good()
    {
        return _init_ok;
    }

    int CODE(int code, void *extra = nullptr)
    {
        return code;
    }

private:
    bool _init_ok = false;
    virtual Type DecoderType() = 0;
};

class VMiniDecoder : public MiniDecoder
{
public:
    using MiniDecoder::MiniDecoder;

    int Width()
    {
        return Good() ? _dec_ctx->width : -1;
    }

    int Height()
    {
        return Good() ? _dec_ctx->height : -1;
    }

    double Framerate()
    {
        return Good() ? _fmt_ctx->streams[_stream_index]->avg_frame_rate.num/_fmt_ctx->streams[_stream_index]->avg_frame_rate.den : -1.0;
    }

private:
    virtual Type DecoderType() override
    {
        return MiniDecoder::VIDEO;
    }
};

class AMiniDecoder : public MiniDecoder
{
public:
    using MiniDecoder::MiniDecoder;

    int Channels()
    {
        return Good() ? _dec_ctx->channels : -1;
    }

    int Samplerate()
    {
        return Good() ? _dec_ctx->sample_rate : -1;
    }

    int BitsPerSample()
    {
        return Good() ? _dec_ctx->bits_per_coded_sample : -1;
    }

private:
    virtual Type DecoderType() override
    {
        return MiniDecoder::AUDIO;
    }
};

#endif
