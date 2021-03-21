#pragma once

namespace cxx
{
    // simple pcm audio data reader from wave file
    class wave_reader final: public noncopyable
    {
    public:
        // readonly
        int mChannelsCount = 0;
        int mSampleRate = 0;
        int mSampleBits = 0;
        int mSamplesCount = 0;
        unsigned int mAudioDataLength = 0; // in bytes

    public:
        wave_reader(std::istream& inputStream);

        bool parse_audio();
        bool audio_present() const;

        // read pcm audio data
        // @param samplesCount: samples count to read
        // @parm buffer: Destination buffer
        // @returns Samples count copied to buffer
        int read_pcm_samples(int samplesCount, void* buffer);

        // @param samples: Samples count
        bool seek_pcm_beg(int samples);
        bool seek_pcm_end(int samples);
        bool seek_pcm_cur(int samples);

        bool end_of_stream() const;

    private:
        void clear();

    private:
        std::istream& mInputStream;
        std::streampos mAudioDataStart;
        std::streampos mAudioDataEnd;
    };

} // namespace cxx