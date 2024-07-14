import subprocess
from pathlib import Path


for file in Path('videos').rglob('*.avi'):
    ''' ffmpeg -i input.avi -c:v libx264 -c:a copy out.mp4 '''
    subprocess.run(["ffmpeg", "-i", file.as_posix(), "-c:v", "libx264", "-c:a", "copy", file.with_suffix('.mp4').as_posix()])
