#include "audio_effect_rnnoise.h"

#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// RNNoise was trained on 16-bit PCM audio, so samples must be scaled from
// Godot's native [-1, 1] float range to roughly [-32768, 32768] and back.
static constexpr float PCM_SCALE = 32768.0f;

void AudioEffectRNNoise::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_enabled", "enabled"), &AudioEffectRNNoise::set_enabled);
	ClassDB::bind_method(D_METHOD("is_enabled"), &AudioEffectRNNoise::is_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "is_enabled");
}

Ref<AudioEffectInstance> AudioEffectRNNoise::_instantiate() {
	Ref<AudioEffectRNNoiseInstance> instance;
	instance.instantiate();
	instance->base = Ref<AudioEffectRNNoise>(this);
	return instance;
}

AudioEffectRNNoiseInstance::AudioEffectRNNoiseInstance() {
	state = rnnoise_create(nullptr);

	const int frame_size = rnnoise_get_frame_size();
	pending_in.resize(frame_size);
	pending_out.resize(frame_size);
}

AudioEffectRNNoiseInstance::~AudioEffectRNNoiseInstance() {
	if (state != nullptr) {
		rnnoise_destroy(state);
		state = nullptr;
	}
}

void AudioEffectRNNoiseInstance::_process(const void *src_buffer, AudioFrame *p_dst_frames, int p_frame_count) {
	const AudioFrame *src = (const AudioFrame *)src_buffer;

	if (base.is_null() || !base->is_enabled()) {
		for (int i = 0; i < p_frame_count; i++) {
			p_dst_frames[i] = src[i];
		}
		return;
	}

	if (!warned_wrong_mix_rate) {
		const double mix_rate = AudioServer::get_singleton()->get_mix_rate();
		if (!Math::is_equal_approx(mix_rate, 48000.0)) {
			UtilityFunctions::push_warning(vformat(
					"AudioEffectRNNoise requires a 48kHz audio mix rate (project is running at %d Hz); denoising quality will be degraded. Set Audio > General > Mix Rate to 48000 in Project Settings.",
					(int)mix_rate));
		}
		warned_wrong_mix_rate = true;
	}

	const int frame_size = rnnoise_get_frame_size();

	for (int i = 0; i < p_frame_count; i++) {
		// Emit a previously-denoised sample if one is ready, otherwise
		// silence -- RNNoise can only start producing output once the first
		// full 480-sample frame has been accumulated, so there's an
		// inherent ~10ms latency before audio starts flowing through.
		if (pending_out_count > 0) {
			const float mono = pending_out[pending_out_start] / PCM_SCALE;
			pending_out_start++;
			pending_out_count--;
			p_dst_frames[i].left = mono;
			p_dst_frames[i].right = mono;
		} else {
			p_dst_frames[i].left = 0.0f;
			p_dst_frames[i].right = 0.0f;
		}

		// Mix the stereo input down to mono and accumulate it for RNNoise.
		pending_in[pending_count] = (src[i].left + src[i].right) * 0.5f * PCM_SCALE;
		pending_count++;

		if (pending_count == frame_size) {
			rnnoise_process_frame(state, pending_out.ptrw(), pending_in.ptr());
			pending_out_start = 0;
			pending_out_count = frame_size;
			pending_count = 0;
		}
	}
}
