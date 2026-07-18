extends SceneTree

## Headless demo of the RNNoiseDenoiser scripted API: run with
##   godot --headless --script res://scripted_demo.gd
## Synthesizes a "speech" tone buried in noise, denoises it frame by frame,
## and reports the RMS noise level before/after so the effect is visible
## without needing a real audio file or a running editor.


func _initialize() -> void:
	var denoiser := RNNoiseDenoiser.new()
	var frame_size := denoiser.get_frame_size()
	print("RNNoise frame size: %d samples (%.1fms @ 48kHz)" % [frame_size, frame_size / 48.0])

	var rng := RandomNumberGenerator.new()
	rng.seed = 0

	var num_frames := 50
	var sample_rate := 48000.0
	var tone_freq := 220.0

	var noisy_rms := 0.0
	var denoised_rms := 0.0
	var sample_count := 0
	var last_vad := 0.0

	for f in range(num_frames):
		var frame := PackedFloat32Array()
		frame.resize(frame_size)
		for i in range(frame_size):
			var t := (f * frame_size + i) / sample_rate
			var tone := sin(TAU * tone_freq * t) * 0.3
			var noise := rng.randf_range(-1.0, 1.0) * 0.2
			frame[i] = clampf(tone + noise, -1.0, 1.0)
			noisy_rms += frame[i] * frame[i]
			sample_count += 1

		var result := denoiser.process_frame(frame)
		var denoised: PackedFloat32Array = result["frame"]
		last_vad = result["vad_probability"]
		for s in denoised:
			denoised_rms += s * s

	noisy_rms = sqrt(noisy_rms / sample_count)
	denoised_rms = sqrt(denoised_rms / sample_count)

	print("Input RMS:    %.4f" % noisy_rms)
	print("Denoised RMS: %.4f" % denoised_rms)
	print("Last frame speech probability: %.3f" % last_vad)
	print("Done.")

	quit()
