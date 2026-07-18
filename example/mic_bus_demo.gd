extends Control

## Live mic-input demo for AudioEffectRNNoise. Speak into the microphone and
## toggle the checkbox to A/B compare the raw vs. denoised signal, both
## played back through the "Mic" bus in real time.

@onready var toggle: CheckButton = %EnabledToggle
@onready var mic_player: AudioStreamPlayer = %MicPlayer

var _bus_index: int
var _effect: AudioEffectRNNoise


func _ready() -> void:
	_bus_index = AudioServer.get_bus_index("Mic")
	_effect = AudioServer.get_bus_effect(_bus_index, 0)

	toggle.button_pressed = _effect.enabled
	toggle.toggled.connect(_on_toggled)

	mic_player.stream = AudioStreamMicrophone.new()
	mic_player.bus = "Mic"
	mic_player.play()


func _on_toggled(enabled: bool) -> void:
	_effect.enabled = enabled
