//
//  audio-thread.cpp
//  ndnrtc
//
//  Copyright 2013 Regents of the University of California
//  For licensing details see the LICENSE file.
//
//  Author:  Peter Gusev
//

#include "audio-thread.h"
#include "estimators.h"

using namespace ndnrtc;
using namespace webrtc;

//******************************************************************************
#pragma mark - public
AudioThread::AudioThread(const AudioThreadParams& params,
    const AudioCaptureParams& captureParams,
    IAudioThreadCallback* callback,
    size_t bundleWireLength):
bundleNo_(0),
rateId_(estimators::setupFrequencyMeter(4)),
threadName_(params.threadName_),
codec_(params.codec_),
callback_(callback),
bundle_(boost::make_shared<AudioBundlePacket>(bundleWireLength)),
capturer_(captureParams.deviceId_, this, 
    (params.codec_ == "opus" ? WebrtcAudioChannel::Codec::Opus : WebrtcAudioChannel::Codec::G722)),
isRunning_(false)
{
    description_ = "athread";
}

AudioThread::~AudioThread()
{
    if (isRunning_) stop();
}

void AudioThread::start()
{
    if (isRunning_) throw std::runtime_error("Audio thread already started");
    isRunning_ = true;
    bundleNo_ = 0;
    capturer_.startCapture();

    LogTraceC << "started" << std::endl;
}

void AudioThread::stop()
{
    if (isRunning_) 
    {
        isRunning_ = false;
        capturer_.stopCapture();
        LogTraceC << "stopped" << std::endl;
    }
}

double AudioThread::getRate() const
{
    return estimators::currentFrequencyMeterValue(rateId_);
}

//******************************************************************************
void AudioThread::onDeliverRtpFrame(unsigned int len, uint8_t* data)
{   
    if (isRunning_)
    {
        AudioBundlePacket::AudioSampleBlob blob({false}, len, data);
        deliver(blob);
    }
}

void AudioThread::onDeliverRtcpFrame(unsigned int len, uint8_t* data)
{
    if (isRunning_)
    {
        AudioBundlePacket::AudioSampleBlob blob({true}, len, data);
        deliver(blob);
    }
}

void AudioThread::deliver(const AudioBundlePacket::AudioSampleBlob& blob)
{
    if (!bundle_->hasSpace(blob))
    {
        estimators::frequencyMeterTick(rateId_);
        callback_->onSampleBundle(threadName_, bundleNo_++, bundle_);
    }

    *bundle_ << blob;
}
