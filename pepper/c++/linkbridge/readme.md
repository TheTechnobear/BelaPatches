# Ableton Link brige

ableton link -> midi clock + transport 
ableton link -> cv


# CV 
cv out 1 = clock beat (1/4 note)
cv out 2 = run status
cv out 3 = bar trig

# compiling using xcBela

CPPFLAGS="-I../../../external/link -I../../../external/link/asio-standalone/asio/include -DLINK_PLATFORM_LINUX=1" xcCompileRun linkbridge

