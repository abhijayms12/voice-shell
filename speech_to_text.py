import sys
import numpy as np
import sounddevice as sd
import whisper
import torch

def main():
    # Check CUDA availability
    if not torch.cuda.is_available():
        print("ERROR: CUDA is not available. This program requires CUDA.", file=sys.stderr)
        sys.exit(1)
    
    # List available audio devices
    print("Available audio devices:")
    print(sd.query_devices())
    print()
    
    # Get default input device
    default_input = sd.default.device[0]
    device_info = sd.query_devices(default_input, 'input')
    print(f"Using default input device: {device_info['name']}")
    print()
    
    # Load Whisper model
    print("Loading Whisper base model...")
    model = whisper.load_model("base", device="cuda")
    print("Model loaded. Listening...\n")
    
    # Audio parameters
    SAMPLE_RATE = 16000
    CHUNK_DURATION = 5  # seconds
    CHUNK_SAMPLES = SAMPLE_RATE * CHUNK_DURATION
    RMS_THRESHOLD = 0.01
    
    try:
        while True:
            # Record audio chunk
            audio = sd.rec(
                CHUNK_SAMPLES,
                samplerate=SAMPLE_RATE,
                channels=1,
                dtype='float32',
                device=default_input
            )
            sd.wait()
            
            # Convert to 1D numpy array
            audio = audio.flatten()
            
            # Normalize audio
            max_val = np.abs(audio).max()
            if max_val > 0:
                audio = audio / max_val
            
            # Compute RMS energy
            rms = np.sqrt(np.mean(audio ** 2))
            
            # Skip if below threshold
            if rms < RMS_THRESHOLD:
                continue
            
            # Transcribe
            result = model.transcribe(
                audio,
                temperature=0.0,
                condition_on_previous_text=False,
                verbose=False
            )
            
            # Print recognized text
            text = result["text"].strip()
            if text:
                print(text)
    
    except KeyboardInterrupt:
        print("\nStopped.")
        sys.exit(0)

if __name__ == "__main__":
    main()
