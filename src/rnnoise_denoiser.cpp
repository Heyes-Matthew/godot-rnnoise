#include "rnnoise_denoiser.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// RNNoise was trained on 16-bit PCM audio, so samples must be scaled from
// Godot's native [-1, 1] float range to roughly [-32768, 32768] and back.
static constexpr float PCM_SCALE = 32768.0f;

void RNNoiseDenoiser::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_frame_size"), &RNNoiseDenoiser::get_frame_size);
	ClassDB::bind_method(D_METHOD("process_frame", "frame"), &RNNoiseDenoiser::process_frame);
}

RNNoiseDenoiser::RNNoiseDenoiser() {
	state = rnnoise_create(nullptr);
}

RNNoiseDenoiser::~RNNoiseDenoiser() {
	if (state != nullptr) {
		rnnoise_destroy(state);
		state = nullptr;
	}
}

int RNNoiseDenoiser::get_frame_size() const {
	return rnnoise_get_frame_size();
}

Dictionary RNNoiseDenoiser::process_frame(const PackedFloat32Array &frame) {
	const int frame_size = rnnoise_get_frame_size();

	if (frame.size() != frame_size) {
		UtilityFunctions::push_error(
				vformat("RNNoiseDenoiser.process_frame: expected a frame of exactly %d samples (got %d). Use get_frame_size() to size your buffers.",
						frame_size, frame.size()));
		return Dictionary();
	}

	PackedFloat32Array pcm_in;
	pcm_in.resize(frame_size);
	for (int i = 0; i < frame_size; i++) {
		pcm_in[i] = frame[i] * PCM_SCALE;
	}

	PackedFloat32Array pcm_out;
	pcm_out.resize(frame_size);

	const float vad_probability = rnnoise_process_frame(state, pcm_out.ptrw(), pcm_in.ptr());

	PackedFloat32Array out_frame;
	out_frame.resize(frame_size);
	for (int i = 0; i < frame_size; i++) {
		out_frame[i] = pcm_out[i] / PCM_SCALE;
	}

	Dictionary result;
	result["frame"] = out_frame;
	result["vad_probability"] = vad_probability;
	return result;
}
