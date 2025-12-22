import whisper
import sounddevice as sd
import numpy as np
import warnings

warnings.filterwarnings("ignore")

SAMPLE_RATE = 16000
DURATION = 5  # seconds

# Load model (CPU-safe)
model = whisper.load_model("base", device="cpu")

# Record audio
audio = sd.rec(
    int(DURATION * SAMPLE_RATE),
    samplerate=SAMPLE_RATE,
    channels=1,
    dtype="float32"
)
sd.wait()

audio = audio.flatten()

# Normalize
max_val = np.max(np.abs(audio))
if max_val > 0:
    audio = audio / max_val

# Silence check
rms = np.sqrt(np.mean(audio ** 2))
if rms < 0.01:
    print("")
    exit(0)

# Transcribe
result = model.transcribe(
    audio,
    language="en",
    temperature=0.0,
    condition_on_previous_text=False,
    suppress_blank=True,
    suppress_tokens="-1",
    initial_prompt="This is a command for a computer terminal."
)

spoken = result["text"].strip().lower()

# Detect hallucination (repetitive phrases)
words = spoken.split()
if len(words) > 10:
    # Check if more than 40% of words are the same
    unique_ratio = len(set(words)) / len(words)
    if unique_ratio < 0.6:
        print("")
        exit(0)

# Remove trailing punctuation
spoken = spoken.rstrip(".,!?")

# Replace "underscore" with actual underscore for file/folder names
spoken = spoken.replace(" underscore ", "_")

# Rule-based mapping
command = ""

# Directory commands
if "create" in spoken and ("folder" in spoken or "directory" in spoken):
    # Extract folder name - everything after "folder" or "directory"
    words = spoken.split()
    if "folder" in words:
        idx = words.index("folder")
        folder_name = "_".join(words[idx+1:]) if idx+1 < len(words) else words[-1]
    elif "directory" in words:
        idx = words.index("directory")
        folder_name = "_".join(words[idx+1:]) if idx+1 < len(words) else words[-1]
    else:
        folder_name = words[-1]
    command = f"mkdir {folder_name}"

elif ("remove" in spoken or "delete" in spoken) and ("folder" in spoken or "directory" in spoken):
    words = spoken.split()
    if "folder" in words:
        idx = words.index("folder")
        folder_name = "_".join(words[idx+1:]) if idx+1 < len(words) else words[-1]
    elif "directory" in words:
        idx = words.index("directory")
        folder_name = "_".join(words[idx+1:]) if idx+1 < len(words) else words[-1]
    else:
        folder_name = words[-1]
    command = f"rmdir {folder_name}"

elif ("change directory" in spoken or "go to" in spoken or "navigate to" in spoken):
    # Extract directory name
    words = spoken.split()
    if "to" in words:
        idx = words.index("to")
        if idx + 1 < len(words):
            dir_name = words[idx + 1]
            command = f"cd {dir_name}"
    elif len(words) > 0:
        command = f"cd {words[-1]}"

elif "go back" in spoken or "parent directory" in spoken:
    command = "cd .."

elif "go home" in spoken or "home directory" in spoken:
    command = "cd"

# File listing commands
elif "list files" in spoken or "show files" in spoken or "list directory" in spoken:
    command = "ls"

elif "list" in spoken and len(words) > 1:
    # ls with path, e.g., "list demo"
    command = f"ls {words[-1]}"

# Print working directory
elif "current directory" in spoken or "where am i" in spoken or "show directory" in spoken or "print working directory" in spoken:
    command = "pwd"

# File operations
elif "create file" in spoken or "new file" in spoken or "touch" in spoken:
    words = spoken.split()
    if "file" in words:
        idx = words.index("file")
        file_name = "_".join(words[idx+1:]) if idx+1 < len(words) else words[-1]
    else:
        file_name = words[-1]
    command = f"touch {file_name}"

elif "delete file" in spoken or "remove file" in spoken:
    words = spoken.split()
    if "file" in words:
        idx = words.index("file")
        file_name = "_".join(words[idx+1:]) if idx+1 < len(words) else words[-1]
    else:
        file_name = words[-1]
    command = f"rm {file_name}"

elif "read file" in spoken or "show file" in spoken or "cat" in spoken or "display contents" in spoken or "display file" in spoken:
    words = spoken.split()
    if "file" in words:
        idx = words.index("file")
        file_name = "_".join(words[idx+1:]) if idx+1 < len(words) else words[-1]
    elif "contents" in words:
        idx = words.index("contents")
        file_name = "_".join(words[idx+1:]) if idx+1 < len(words) else words[-1]
    else:
        file_name = words[-1]
    command = f"cat {file_name}"

elif "copy" in spoken:
    # Extract source and destination
    words = spoken.split()
    if "to" in words:
        to_idx = words.index("to")
        if to_idx > 1 and to_idx + 1 < len(words):
            source = words[to_idx - 1]
            dest = words[to_idx + 1]
            command = f"cp {source} {dest}"

elif "move" in spoken or "rename" in spoken:
    # Extract source and destination
    words = spoken.split()
    if "to" in words:
        to_idx = words.index("to")
        if to_idx > 1 and to_idx + 1 < len(words):
            source = words[to_idx - 1]
            dest = words[to_idx + 1]
            command = f"mv {source} {dest}"

# Utility commands
elif "clear screen" in spoken or "clear terminal" in spoken or "clear" in spoken:
    command = "clear"

elif "help" in spoken or "show commands" in spoken or "available commands" in spoken:
    command = "help"

elif "exit" in spoken or "quit" in spoken or "close" in spoken or "bye" in spoken:
    command = "exit"

# Fallback: use transcribed text as-is
else:
    command = spoken

print(command)
