#ifndef GODOT_RNNOISE_DENOISER_H
#define GODOT_RNNOISE_DENOISER_H

#include <rnnoise.h>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>

namespace godot {

// Scriptable wrapper around a single RNNoise DenoiseState. Frame size and
// sample rate are fixed by RNNoise itself (480 samples / 10ms of mono audio
// at 48kHz) -- callers must chunk their own audio to match get_frame_size().
class RNNoiseDenoiser : public RefCounted {
	GDCLASS(RNNoiseDenoiser, RefCounted)

	DenoiseState *state = nullptr;

protected:
	static void _bind_methods();

public:
	int get_frame_size() const;

	// Denoises one frame of mono samples in Godot's native [-1, 1] range.
	// Returns {"frame": PackedFloat32Array, "vad_probability": float}.
	Dictionary process_frame(const PackedFloat32Array &frame);

	RNNoiseDenoiser();
	~RNNoiseDenoiser();
};

} // namespace godot

#endif // GODOT_RNNOISE_DENOISER_H
