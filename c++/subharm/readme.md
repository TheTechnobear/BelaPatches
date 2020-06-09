# SubHarm

a project inspired by the moog subharmonicon



# compiling with xcBela
CPPFLAGS="-DPEPPER" xcCompile autocal
CPPFLAGS="-DSALT" xcCompile autocal


# credits 






# issue with clock

                        if(i==0 && c==0) {
                            static unsigned aCount=0;
                            float v=osc.timeVals_[c];
                            // float ctv=cvToTime(v);
                            float h1  =  floorf(8.0f - (v * 8.0f));   
                            // float h2 = constrain(h1, 0.0f,16.0f);
                            unsigned aa = samp > 0 ? s.clkCount_  % samp : 0;  
                            if(trig) {
                                if(s.clkCount_< 2)  {
                                    if(aCount!=h1) {
                                        rt_printf("++++");
                                        rt_printf("%d  (%d)  - %d <= %f  -  %f %f \n",s.clkCount_, trig,aCount, h1, samp,aa);
                                    }
                                    aCount=0;
                                }
                                aCount++;
                                // rt_printf("%d  (%d)  - %d <= %f  -  %f %f \n",s.clkCount_, trig,aCount, h1, samp,aa);
                            } else {
                                if(s.clkCount_< 2 && !trig) {
                                    rt_printf("!!!!");
                                    rt_printf("%d  (%d)  - %d <= %f  -  %f %f \n",s.clkCount_, trig,aCount, h1, samp,aa);
                                }
                            }
                        }
