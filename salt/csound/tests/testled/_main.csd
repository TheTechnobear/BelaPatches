<CsoundSynthesizer>
<CsOptions>
-m0d
</CsOptions>
<CsInstruments>
ksmps = 8
nchnls = 2
0dbfs = 1

	instr 1

		aSine = poscil(0.5, 250)		

		kFreq = 44100/32
		kDummy = 0

		aPW vco2 1,kFreq, 2, 0.2
		aPW = (aPW * 0.5) + 0.5
		digiOutBela aPW, 7
		
		kSwitch digiInBela 14
		if (kSwitch > 0) then
			kDummy = 1 ;yellow
		    ; kDummy = 0 ;red
			digiIOBela kDummy, 4, 1
		else
			digiIOBela kDummy, 4, 0
		endif

		aOut = aSine * kSwitch
		outs aOut, aOut
	endin

</CsInstruments>
<CsScore>
i1 0 86400
</CsScore>
</CsoundSynthesizer>
