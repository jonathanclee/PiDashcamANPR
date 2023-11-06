# PiDashcamANPR
Software for Raspberry Pi to record camera feed to file and perform automatic plate number recognition

--- Objectives ---

-Accept camera feed (USB or CSI format) and record the video to an external drive

-Video files shall be split into segments (1-2min) to mitigate file corruption risk

-Detect license plates within frames of the video, and determine the alphanumerical characters it contains
