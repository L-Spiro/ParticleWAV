Don’t you just hate it when you open a WAV file in Audacity, make no changes, save it, and then discover that 50% of the PCM samples were modified?  Just because they couldn’t be bothered to apply `std::round()` when converting to PCM!  Unfortunately this small detail is overlooked quite often, so which tool can you trust to edit WAV files and modify **only what was edited**?

# ParticleWAV
Provides basic WAV-editing functionality with the promise that any samples that weren’t modified intentionally remain unmodified in the output file.
