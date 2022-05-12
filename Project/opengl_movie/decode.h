#ifndef DECODE_H
#define DECODE_H

#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <cstdio>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#ifdef __cplusplus
}
#endif

struct Frame {
	AVFrame *frame;
	bool fill;
	bool use;
};

class Decode {
	public:
		Decode(const std::string &path) : url(path) {
			std::cout << "Decode init ..." << std::endl;
			if(init() == 0) {
				std::cout << "Decode init ok" << std::endl;
			}else {
				std::cout << "Decode init error" << std::endl;
			}
		}

		~Decode() {
			stop();
			if(dec_ctx) avcodec_free_context(&dec_ctx);
			if(fmt_ctx) avformat_close_input(&fmt_ctx);
			if(frames[0].frame) av_frame_free(&frames[0].frame);
			if(frames[1].frame) av_frame_free(&frames[1].frame);
		}

		Decode(const Decode &) = delete;
		Decode &operator=(const Decode &) = delete;

		void stop() {
			running = false;
		}


		//获取一帧解码帧，没有数据返回nullptr
		AVFrame *getFrame() {
			if(!running) {
				cv.notify_one();
				return nullptr;
			}

			std::unique_lock<std::mutex> lk(lock_mutex, std::defer_lock);
			if(!lk.try_lock()) return nullptr;

			AVFrame *frame = nullptr;
			//如果frames[1]是填充的并且没有在使用则取出这一帧返回
			//并且将其状态改为非填充且正在使用，
			//并将frames[0]变为非使用，因为同时只有一帧正在使用，
			//notify,让生产者生产数据
			if(frames[1].fill && !frames[1].use) {
				frame = frames[1].frame;
				frames[1].fill = false;
				frames[1].use = true;
				frames[0].use = !frames[1].use;
			}

			lk.unlock();
			cv.notify_one();

			return frame;
		}

		int getWidth() {
			return width;
		}

		int getHeight() {
			return height;
		}

	private:
		std::mutex lock_mutex;
		std::condition_variable cv;

		std::string url;
		int video_index;
		int audio_index;

		//双缓冲
		//frames[0]写入缓冲
		//frames[1]读取缓冲
		Frame frames[2];

		AVFormatContext *fmt_ctx;
		AVCodecContext *dec_ctx;

		AVRational time_base;

		int width = -1;
		int height = -1;

		bool running = true;

		void swapBuffer() {
			Frame tmp = frames[0];
			frames[0] = frames[1];
			frames[1] = tmp;
		}

		bool decodeFrame(AVFrame *frame) {
			//std::cout << "decode a frame..." << std::endl;
			/* read all packets */
			AVPacket packet;
			int ret;
			bool ok = false;
			while (true) {
				if ((ret = av_read_frame(fmt_ctx, &packet)) < 0) break;
				if (packet.stream_index == video_index) {
					ret = avcodec_send_packet(dec_ctx, &packet);
					if (ret < 0) {
						av_log(NULL, AV_LOG_ERROR, "Error while sending a packet to the decoder\n");
						break;
					}
					while (ret >= 0) {
						ret = avcodec_receive_frame(dec_ctx, frame);
						if (ret == AVERROR(EAGAIN)) {
							break;
						}
						else if (ret < 0) {
							av_log(NULL, AV_LOG_ERROR, "Error while receiving a frame from the decoder\n");
							goto end;
						}

						frame->pts = frame->best_effort_timestamp;
						ok = true;
						goto end;
					}
				}

				av_packet_unref(&packet);
			}
end:
			return ok;
		}

		void decodeTask() {
			std::cout << "decode task is running..." << std::endl;
			AVFrame *frame = av_frame_alloc();
			if(!frame) {
				std::cout << "error to alloc frame for decoding!" << std::endl;
				return;
			}
			while(running) {
				std::unique_lock<std::mutex> lk(lock_mutex);

				//如果frames[0]是填充的或者是正在使用的则等待其变为可写入
				//如果running=false则解除等待
				cv.wait(lk, [this]{ return (!running || (!frames[0].fill) && (!frames[0].use)); });

				if(!running) return;

				//到这里说明frames[0]帧是可写入的，解码一帧数据
				if(decodeFrame(frame) && running) {
					av_frame_move_ref(frames[0].frame, frame);
					av_frame_unref(frame);

					//状态变为已填充，并交换buffer，使得frames[1]变为可读
					frames[0].fill = true;
					swapBuffer();
				}

				//lk.unlock();
				//cv.notify_one();
			}
			av_frame_free(&frame);
		}


		int init() {
			//初始化frame双缓冲
			if(!(frames[0].frame = av_frame_alloc())) {
				av_log(NULL, AV_LOG_ERROR, "failed to alloc frame\n");
				return AVERROR(ENOMEM);
			}
			if(!(frames[1].frame = av_frame_alloc())) {
				av_log(NULL, AV_LOG_ERROR, "failed to alloc frame\n");
				return AVERROR(ENOMEM);
			}
			frames[0].fill = false;
			frames[0].use = false;
			frames[1].fill = false;
			frames[1].use = true;

			//打开编码器
			int ret;
			const AVCodec *dec = nullptr;
			if ((ret = avformat_open_input(&fmt_ctx, url.c_str(), NULL, NULL)) < 0) {
				av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
				return ret;
			}
			if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
				av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
				return ret;
			}
			/* select the video stream */
			ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
			if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Cannot find a video stream in the input file\n");
				return ret;
			}
			video_index = ret;
			/* create decoding context */
			dec_ctx = avcodec_alloc_context3(dec);
			if (!dec_ctx)
				return AVERROR(ENOMEM);
			avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[video_index]->codecpar);
			/* init the video decoder */
			if ((ret = avcodec_open2(dec_ctx, dec, NULL)) < 0) {
				av_log(NULL, AV_LOG_ERROR, "Cannot open video decoder\n");
				return ret;
			}

			time_base = fmt_ctx->streams[video_index]->time_base;
			width = dec_ctx->width;
			height = dec_ctx->height;

			//Output Info-----------------------------
			printf("--------------- File Information ----------------\n");
			av_dump_format(fmt_ctx, 0, url.c_str(), 0);
			printf("-------------------------------------------------\n");

			//启动解码线程
			std::thread t(&Decode::decodeTask, this);
			t.detach();

			return 0;
		}
};


#endif



//std::mutex m;
//std::condition_variable cv;
//std::string data;
//bool ready = false;
//bool processed = false;
//
//void worker_thread()
//{
//    // 等待直至 main() 发送数据
//    std::unique_lock<std::mutex> lk(m);
//    cv.wait(lk, []{return ready;});
//
//    // 等待后，我们占有锁。
//    std::cout << "Worker thread is processing data\n";
//    data += " after processing";
//
//    // 发送数据回 main()
//    processed = true;
//    std::cout << "Worker thread signals data processing completed\n";
//
//    // 通知前完成手动解锁，以避免等待线程才被唤醒就阻塞（细节见 notify_one ）
//    lk.unlock();
//    cv.notify_one();
//}
//
//int main()
//{
//    std::thread worker(worker_thread);
//
//    data = "Example data";
//    // 发送数据到 worker 线程
//    {
//        std::lock_guard<std::mutex> lk(m);
//        ready = true;
//        std::cout << "main() signals data ready for processing\n";
//    }
//    cv.notify_one();
//
//    // 等候 worker
//    {
//        std::unique_lock<std::mutex> lk(m);
//        cv.wait(lk, []{return processed;});
//    }
//    std::cout << "Back in main(), data = " << data << '\n';
//
//    worker.join();
//}
