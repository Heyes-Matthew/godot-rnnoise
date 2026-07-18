#ifndef GODOT_AUDIO_EFFECT_RNNOISE_H
#define GODOT_AUDIO_EFFECT_RNNOISE_H

#include <rnnoise.h>

#include <godot_cpp/classes/audio_effect.hpp>
#include <godot_cpp/classes/audio_effect_instance.hpp>
#include <godot_cpp/classes/audio_frame.hpp>

namespace godot {

class AudioEffectRNNoise;

// Owns the actual RNNoise state and the ring buffer needed to bridge Godot's
// arbitrarily-sized audio callback blocks to RNNoise's fixed 480-sample
// frames. A new instance (and RNNoise state) is created per bus-effect
// instantiation, matching how Godot re-instantiates effects per playback.
class AudioEffectRNNoiseInstance : public AudioEffectInstance {
	GDCLASS(AudioEffectRNNoiseInstance, AudioEffectInstance)
	friend class AudioEffectRNNoise;

	Ref<AudioEffectRNNoise> base;

	DenoiseState *state = nullptr;

	// Ring buffer of pending mono samples awaiting a full RNNoise frame.
	PackedFloat32Array pending_in;
	int pending_count = 0;

	// Denoised samples ready to be emitted, in case a processed frame is
	// larger than what _process() was asked to output in one call.
	PackedFloat32Array pending_out;
	int pending_out_start = 0;
	int pending_out_count = 0;

	bool warned_wrong_mix_rate = false;

	void process_available_frames();

protected:
	static void _bind_methods() {}

public:
	virtual void _process(const void *src_buffer, AudioFrame *p_dst_frames, int p_frame_count) override;
	virtual bool _process_silence() const override { return true; }

	AudioEffectRNNoiseInstance();
	~AudioEffectRNNoiseInstance();
};

// The AudioEffect resource added to a bus in the editor. RNNoise requires a
// 48kHz mono-mixed input; this effect expects the bus (and therefore the
// project's audio mix rate) to already be 48kHz and warns at runtime if not,
// rather than silently resampling.
class AudioEffectRNNoise : public AudioEffect {
	GDCLASS(AudioEffectRNNoise, AudioEffect)
	friend class AudioEffectRNNoiseInstance;

	bool enabled = true;

protected:
	static void _bind_methods();

public:
	virtual Ref<AudioEffectInstance> _instantiate() override;

	void set_enabled(bool p_enabled) { enabled = p_enabled; }
	bool is_enabled() const { return enabled; }
};

} // namespace godot

#endif // GODOT_AUDIO_EFFECT_RNNOISE_H
