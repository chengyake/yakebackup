ffmpeg -i $1.aac -acodec pcm_s16le -ac 1 -ar 8000 -vn $1.wav
